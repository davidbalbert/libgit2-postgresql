require 'mkmf'

# PostgreSQL

pg_config = with_config('pg_config') || find_executable('pg_config')

pg_incdir = `#{pg_config} --includedir`.chomp
pg_libdir = `#{pg_config} --libdir`.chomp

dir_config("postgresql", pg_incdir, pg_libdir)

unless have_library("pq", "PQexec")
  abort "ERROR: libpq is required"
end



# Rugged/libgit2

require 'rugged'

RUGGED_ROOT = File.join(Rugged::Repository.instance_method(:lookup).source_location.first.split(File::SEPARATOR)[0..-4])

LIBGIT2_DIR = File.join(RUGGED_ROOT, 'vendor', 'libgit2')

$CFLAGS << " -I#{LIBGIT2_DIR}/include"
$LIBPATH.unshift "#{LIBGIT2_DIR}/build"

unless have_library('git2') && have_header('git2.h')
  abort "Error: libgit2 is required"
end

$CPPFLAGS += "-I #{File.expand_path("../../../../../", __FILE__)}"

create_makefile('rugged/postgresql/rugged_postgresql')
