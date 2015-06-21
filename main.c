#include <stdio.h>
#include <string.h>

#include <git2.h>
#include <git2/sys/odb_backend.h>
#include <git2/sys/refdb_backend.h>

#include <libpq-fe.h>

#include "git2-postgresql.h"

static void
abort_git() {
        fprintf(stderr, "%s\n", giterr_last()->message);
        exit(1);
}

int
main(int argc, char *argv[]) {
        int error = 0;
        char *url = "postgres://localhost/libgit2-test";

        git_repository *repo;
        git_odb *odb;
        git_odb_backend *odb_backend;

        git_libgit2_init();

        error = git_odb_new(&odb);

        if (error) {
                abort_git();
        }

        error = git_repository_wrap_odb(&repo, odb);

        if (error) {
                abort_git();
        }

        error = git_odb_backend_postgresql(&odb_backend, repo, url, "objects");

        if (error) {
                abort_git();
        }

        error = git_odb_add_backend(odb, odb_backend, 1);

        if (error) {
                abort_git();
        }

        odb_backend->free(odb_backend);

        git_libgit2_shutdown();

        return 0;
}
