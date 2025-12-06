using jank_object_ref = void *;

extern "C" void jank_module_set_loaded(char const *module);
extern "C" jank_object_ref jank_eval(jank_object_ref s);
extern "C" jank_object_ref jank_string_create(char const *s);

extern "C" void jank_load_jank_test_a()
{
  jank_eval(jank_string_create("(ns jank-test.a)"));
  jank_module_set_loaded("jank-test.a");
  jank_eval(jank_string_create("(defn -main [] \"a.cpp\")"));
}

