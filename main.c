#include <stdio.h>

#include <git2.h>
#include <git2/odb_backend.h>
#include <git2/refdb.h>

#include <libpq-fe.h>

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
