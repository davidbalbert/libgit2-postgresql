typedef struct {
	git_odb_backend parent;

        PGconn *conn;
        char *table;
} postgresql_odb_backend;

typedef struct {
        git_refdb_backend parent;

        PGconn *conn;
        char *table;
} posgresql_refdb_backend;

int git_postgresql_make_connection(PGconn **conn, char *database_url);

int git_odb_backend_postgresql(git_odb_backend **backend, git_repository *repo, char *database_url, char *table);

int git_odb_backend_postgresql_with_conn(git_odb_backend **backend, git_repository *repo, PGconn *conn, char *table);


int git_refdb_backend_postgresql(git_refdb_backend **backend, git_repository *repo, char *database_url, char *table);

int git_refdb_backend_postgresql_with_conn(git_refdb_backend **backend, git_repository *repo, PGconn *conn, char *table);
