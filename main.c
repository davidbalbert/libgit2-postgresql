#include <stdio.h>

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
usage() {
        fprintf(stderr, "usage: %s database-url\n", prog_name);
        exit(1);
}

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

        if (argc < 2) {
                usage();
        }

        conn = PQconnectdb(argv[1]);

        if (PQstatus(conn) != CONNECTION_OK) {
                dieConn(conn);
        }

        res = PQexec(conn, "SELECT * FROM objects");

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
                dieResult(conn, res);
        }

        int nfields = PQnfields(res);

        for (i = 0; i < nfields; i++) {
                printf("%-15s", PQfname(res, i));
        }
        printf("\n");

        for (i = 0; i < PQntuples(res); i++) {
                for (j = 0; j < nfields; j++)
                        printf("%-15s", PQgetvalue(res, i, j));
                printf("\n");
        }

        PQfinish(conn);
        return 0;
}
