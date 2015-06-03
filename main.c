#include <stdio.h>
#include <string.h>

#include <git2.h>
#include <git2/sys/odb_backend.h>
#include <git2/sys/refdb_backend.h>

#include <libpq-fe.h>

#include "git2-postgresql.h"

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

int
table_exists(PGconn *conn, char *table) {
        char *q = "SELECT COUNT(*) FROM pg_tables WHERE tablename = 'objects'";

        return 0;
}

int
create_odb_table(PGconn *conn, char *table) {
        return 0;
}

int
git_odb_backend_postgresql(git_odb_backend **backend, git_repository *repo, char *database_url, char *table) {
        PGconn *conn;
        int error;

        error = git_postgresql_make_connection(&conn, database_url);

        if (error) {
                return error;
        }

        return git_odb_backend_postgresql_with_conn(backend, repo, conn, table);
}

int
git_odb_backend_postgresql_with_conn(git_odb_backend **backend, git_repository *repo, PGconn *conn, char *table) {
        int error;

        if (!table_exists(conn, table)) {
                error = create_odb_table(conn, table);
                if (error) {
                        return error;
                }
        }

        return GIT_OK;
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

char *prog_name;

static void
dieConn(PGconn *conn) {
        fprintf(stderr, "%s: %s", prog_name, PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
}

static void
dieResult(PGconn *conn, PGresult *res) {
        fprintf(stderr, "%s: %s", prog_name, PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(1);
}

int
main(int argc, char *argv[]) {
        PGconn *conn;
        PGresult *res;
        int i, j;

        prog_name = argv[0];

        conn = PQconnectdb("postgres://localhost/libgit2-test");

        if (PQstatus(conn) != CONNECTION_OK) {
                dieConn(conn);
        }

        char *s = "libgit2_odb_table; SELECT * FROM whatever WHERE thing = 'something'";

        printf("%s\n", PQescapeLiteral(conn, s, strlen(s)));

        PQfinish(conn);
        return 0;
}
