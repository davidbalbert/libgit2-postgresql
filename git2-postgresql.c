#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <git2.h>
#include <git2/sys/odb_backend.h>
#include <git2/sys/refdb_backend.h>

#include <libpq-fe.h>

#include "git2-postgresql.h"

static int
create_table(PGconn *conn, char *table, char *schema) {
        char *tmpl = "CREATE TABLE ";
        char *escaped;
        PGresult *res;

        escaped = PQescapeIdentifier(conn, table, strlen(table));

        if (!escaped) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                return GIT_ERROR;
        }

        size_t len = strlen(tmpl) + strlen(escaped) + strlen(schema) + 1;
        char *q = calloc(len, sizeof(char));

        if (!q) {
                PQfreemem(escaped);
                return GITERR_NOMEMORY;
        }

        strlcpy(q, tmpl, len);
        strlcat(q, escaped, len);
        strlcat(q, schema, len);

        res = PQexec(conn, q);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                PQclear(res);
                PQfreemem(escaped);
                free(q);
                return GIT_ERROR;
        }

        PQclear(res);
        PQfreemem(escaped);
        free(q);

        return GIT_OK;
}

static int
table_exists(PGconn *conn, char *table, int *exists) {
        char *escaped;
        char *tmpl = "SELECT COUNT(*) FROM pg_tables WHERE tablename = ";
        PGresult *res;

        escaped = PQescapeLiteral(conn, table, strlen(table));

        if (escaped == NULL) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                return GIT_ERROR;
        }

        size_t len = strlen(tmpl) + strlen(escaped) + 1;
        char *q = calloc(len, sizeof(char));

        if (q == NULL) {
                PQfreemem(escaped);
                return GITERR_NOMEMORY;
        }

        strlcpy(q, tmpl, len);
        strlcat(q, escaped, len);

        res = PQexec(conn, q);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                PQclear(res);
                return GIT_ERROR;
        }

        assert(PQnfields(res) == 1);
        assert(PQntuples(res) == 1);

        if (strcmp(PQgetvalue(res, 0, 0), "1") == 0) {
                *exists = 1;
        } else {
                *exists = 0;
        }

        PQclear(res);
        free(q);

        return GIT_OK;
}

static int
create_odb_table(PGconn *conn, char *table) {
        char *tmpl =
                "("
                "oid char(%d) PRIMARY KEY, "
                "type smallint NOT NULL, "
                "data bytea NOT NULL"
                ")";

        char *schema;
        int error;

        asprintf(&schema, tmpl, GIT_OID_HEXSZ);

        if (schema == NULL) {
                return GITERR_NOMEMORY;
        }

        error = create_table(conn, table, schema);

        free(schema);

        return error;
}

static char *
data2hex(const void *data, size_t len) {
        char *hexdata = calloc(2 + len*2 + 1, sizeof(char)); // "\\x" "FF" * len '\0'

        if (hexdata == NULL) {
                return NULL;
        }

        const unsigned char *p = data;
        char *s = hexdata;

        *s++ = '\\';
        *s++ = 'x';

        for (int i = 0; i < len; i++) {
                sprintf(s, "%02x", *p);
                s += 2;
                p++;
        }

        *s = '\0';

        return hexdata;
}

/*
static void *
hex2data(const char *hexdata) {
        size_t len = strlen(hexdata);

        assert(len % 2 == 0);

        unsigned char *data = calloc(len - 2 + 1, sizeof(char));
        unsigned char *p = data;

        if (data == NULL) {
                return NULL;
        }

        hexdata += 2; // Skip "\\x"

        for (; *hexdata; hexdata += 2, p++) {
                sscanf(hexdata, "%2hhx", p);
        }

        return data;
}
*/

static int
postgresql_odb_backend__write(git_odb_backend *backend, const git_oid *oid, const void *data, size_t len, git_otype type) {
        postgresql_odb_backend *b = (postgresql_odb_backend *)backend;
        char *tmpl = "INSERT INTO %s (oid, type, data) VALUES ('%s', %d, %s)";
        char *str_id, *hexdata, *esc_hexdata, *q;
        PGresult *res;

        str_id = calloc(GIT_OID_HEXSZ + 1, sizeof(char));

        if (str_id == NULL) {
                return GITERR_NOMEMORY;
        }

        git_oid_tostr(str_id, GIT_OID_HEXSZ + 1, oid);

        hexdata = data2hex(data, len);

        if (hexdata == NULL) {
                free(str_id);
                return GITERR_NOMEMORY;
        }

        esc_hexdata = PQescapeLiteral(b->conn, hexdata, strlen(hexdata));

        if (esc_hexdata == NULL) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(b->conn));
                free(hexdata);
                free(str_id);
                return GIT_ERROR;
        }

        asprintf(&q, tmpl, b->esc_table, str_id, type, esc_hexdata);

        if (q == NULL) {
                PQfreemem(esc_hexdata);
                free(hexdata);
                free(str_id);
                return GITERR_NOMEMORY;
        }

        res = PQexec(b->conn, q);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(b->conn));
                PQclear(res);
                return GIT_ERROR;
        }

        PQclear(res);
        PQfreemem(esc_hexdata);
        free(hexdata);
        free(str_id);

        return GIT_OK;
}

static void
postgresql_odb_backend__free(git_odb_backend *_backend) {
        postgresql_odb_backend *backend = (postgresql_odb_backend *)_backend;

        if (!backend->shared_conn) {
                PQfinish(backend->conn);
        }

        PQfreemem(backend->esc_table);
        free(backend->table);
        free(backend);
}


static int
odb_backend_with_conn(git_odb_backend **_backend, git_repository *repo, PGconn *conn, char *table, int shared) {
        int error;
        int exists = 0;
        postgresql_odb_backend *backend = NULL;

        backend = calloc(1, sizeof(postgresql_odb_backend));
        if (backend == NULL) {
                return GITERR_NOMEMORY;
        }

        error = table_exists(conn, table, &exists);

        if (error) {
                return error;
        }

        if (!exists) {
                error = create_odb_table(conn, table);
                if (error) {
                        return error;
                }
        }

        backend->conn = conn;
        backend->shared_conn = shared;

        backend->table = strdup(table);
        if (backend->table == NULL) {
                return GITERR_NOMEMORY;
        }

        backend->esc_table = PQescapeIdentifier(conn, table, strlen(table));
        if (backend->esc_table == NULL) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                return GIT_ERROR;
        }

        backend->parent.version = 1;

        // read, write, read_prefix, read_header, exists, free, writestream, foreach
        // backend->parent.read = &postgresql_odb_backend__read;
        backend->parent.write = &postgresql_odb_backend__write;
        // backend->parent.read_prefix = &postgresql_odb_backend__read_prefix;
        // backend->parent.read_header = &postgresql_odb_backend__read_header;
        // backend->parent.exists = &postgresql_odb_backend__exists;
        backend->parent.free = &postgresql_odb_backend__free;

        *_backend = (git_odb_backend *)backend;

        return GIT_OK;
}

int
git_odb_backend_postgresql(git_odb_backend **backend, git_repository *repo, char *database_url, char *table) {
        PGconn *conn;
        int error;

        error = git_postgresql_make_connection(&conn, database_url);

        if (error) {
                return error;
        }

        return odb_backend_with_conn(backend, repo, conn, table, 0);
}

int
git_odb_backend_postgresql_with_conn(git_odb_backend **backend, git_repository *repo, PGconn *conn, char *table) {
        return odb_backend_with_conn(backend, repo, conn, table, 1);
}

int
git_refdb_backend_postgresql(git_refdb_backend **backend, git_repository *repo, char *database_url, char *table) {
        PGconn *conn;
        int error;

        error = git_postgresql_make_connection(&conn, database_url);

        if (error) {
                return error;
        }

        return git_refdb_backend_postgresql_with_conn(backend, repo, conn, table);
}

int
git_refdb_backend_postgresql_with_conn(git_refdb_backend **backend, git_repository *repo, PGconn *conn, char *table) {
        return GIT_OK;
}

int
git_postgresql_make_connection(PGconn **_conn, char *database_url) {
        PGconn *conn = PQconnectdb(database_url);

        if (PQstatus(conn) != CONNECTION_OK) {
                giterr_set_str(GITERR_REFERENCE, PQerrorMessage(conn));
                PQfinish(conn);
                return GIT_ERROR;
        }

        *_conn = conn;

        return GIT_OK;
}
