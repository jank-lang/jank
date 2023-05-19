#include <jank/native_box.hpp>

namespace jank
{
  native_box<runtime::obj::nil> make_box(std::nullptr_t const &)
  { return runtime::JANK_NIL; }
  native_box<runtime::obj::boolean> make_box(native_bool const b)
  { return b ? runtime::obj::JANK_TRUE : runtime::obj::JANK_FALSE; }
  native_box<runtime::obj::integer> make_box(int const i)
  { return make_box<runtime::obj::integer>(static_cast<native_integer>(i)); }
  native_box<runtime::obj::integer> make_box(native_integer const i)
  { return make_box<runtime::obj::integer>(i); }
  native_box<runtime::obj::integer> make_box(size_t const i)
  { return make_box<runtime::obj::integer>(static_cast<native_integer>(i)); }
  native_box<runtime::obj::real> make_box(native_real const r)
  { return make_box<runtime::obj::real>(r); }
  native_box<runtime::obj::string> make_box(native_string_view const &s)
  { return make_box<runtime::obj::string>(s); }
  runtime::obj::list_ptr make_box(runtime::detail::persistent_list const &l)
  { return make_box<runtime::obj::list>(l); }
}
