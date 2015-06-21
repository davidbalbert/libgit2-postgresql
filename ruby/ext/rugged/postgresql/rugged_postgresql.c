#include <ruby.h>

VALUE rb_mRugged;
VALUE rb_mRuggedPostgresql;

static VALUE
hello_world(VALUE self) {
        return rb_str_new2("Hello, world!");
}

void
Init_rugged_postgresql(void) {
       rb_mRugged = rb_const_get(rb_cObject, rb_intern("Rugged")); 
       rb_mRuggedPostgresql = rb_define_module_under(rb_mRugged, "Postgresql");

       rb_define_singleton_method(rb_mRuggedPostgresql, "hello", hello_world, 0);
}
