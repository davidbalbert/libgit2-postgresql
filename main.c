#include <stdio.h>

#include <git2.h>
#include <git2/odb_backend.h>
#include <git2/refdb.h>

#include <libpq-fe.h>

int
main(int argc, char *argv[]) {
        printf("Hello, libgit2-postgres!\n");
        return 0;
}
