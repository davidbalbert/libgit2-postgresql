OSX_VERSION = $(shell sw_vers -productVersion)
LIBEXT=dylib

libgit2-postgresql.dylib: $(OBJS)
	$(LIBTOOL) -dynamic -o $@ $(LDFLAGS) -lSystem -macosx_version_min $(OSX_VERSION) -current_version $(VERSION) -compatibility_version $(VERSION) $^
 
