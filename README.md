# Libgit2-postgresql

A PostgreSQL odb and refdb backend for libgit2. Don't expect it to work yet. It's a work in progress.

## Building

Running `make` will build the static and shared libraries. Currently, the only supported operating system is OS X, but it uses no OS-specific libraries, so in theory it should work on any operating system that libgit2 and Postgres support. Building on Linux and \*BSD should be as simple as adding the appropriate `.mk` file and tweaking the code that builds the shared library. The build system will probably need to be modified a bit more for Windows, but the code should work just fine.

TODO: `make install`

## Using

To compile against this library, you'll need to include headers for libgit2 and libpq in addition to the headers for libgit2-postgresql:

```c
#include <git2.h>
#include <git2/sys/odb_backend.h>
#include <git2/sys/refdb_backend.h>

#include <libpq-fe.h>

#include <git2-postgresql.h>
```

## License

Libgit2-postgresql is copyright 2015 David Albert and is licensed under the terms of the GNU GPLv3 or any later version. See COPYING for more details.
