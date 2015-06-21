#include <ruby.h>

#include <git2.h>
#include <git2/sys/odb_backend.h>
#include <git2/sys/refdb_backend.h>

#include <libpq-fe.h>

#include <git2-postgresql.h>

VALUE rb_mRugged;
VALUE rb_mPostgresql;
VALUE rb_cBackend;

static VALUE
hello_world(VALUE self) {
        return rb_str_new2("Hello, world!");
}

void
Init_rugged_postgresql(void) {
        rb_mRugged = rb_const_get(rb_cObject, rb_intern("Rugged"));
        rb_mPostgresql = rb_define_module_under(rb_mRugged, "Postgresql");
        rb_cBackend = rb_define_class_under(rb_mPostgresql, "Backend", rb_cObject);

        rb_define_singleton_method(rb_mPostgresql, "hello", hello_world, 0);
}
