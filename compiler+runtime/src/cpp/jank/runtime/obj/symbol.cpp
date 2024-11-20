#include <fmt/compile.h>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  template <typename S>
  void separate(obj::symbol &sym, S &&s)
  {
    auto const found(s.find('/'));
    if(found != native_persistent_string::npos && s.size() > 1)
    {
      sym.ns = s.substr(0, found);
      sym.name = s.substr(found + 1);
    }
    else
    {
      sym.name = std::forward<S>(s);
    }
  }

  obj::symbol::static_object(native_persistent_string const &d)
  {
    separate(*this, d);
  }

  obj::symbol::static_object(native_persistent_string &&d)
  {
    separate(*this, std::move(d));
  }

  obj::symbol::static_object(native_persistent_string const &ns, native_persistent_string const &n)
    : ns{ ns }
    , name{ n }
  {
  }

  obj::symbol::static_object(native_persistent_string &&ns, native_persistent_string &&n)
    : ns{ std::move(ns) }
    , name{ std::move(n) }
  {
  }

  obj::symbol::static_object(native_persistent_string const &ns,
                             native_persistent_string const &n,
                             object_ptr const meta)
    : ns{ ns }
    , name{ n }
    , meta{ meta }
  {
  }

  obj::symbol::static_object(object_ptr const ns, object_ptr const n)
    : ns{ runtime::to_string(ns) }
    , name{ runtime::to_string(n) }
  {
  }

  native_bool obj::symbol::equal(object const &o) const
  {
    if(o.type != object_type::symbol)
    {
      return false;
    }

    auto const s(expect_object<obj::symbol>(&o));
    return ns == s->ns && name == s->name;
  }

  native_bool obj::symbol::equal(obj::symbol const &s) const
  {
    return ns == s.ns && name == s.name;
  }

  native_integer obj::symbol::compare(object const &o) const
  {
    return visit_type<obj::symbol>([this](auto const typed_o) { return compare(*typed_o); }, &o);
  }

  native_integer obj::symbol::compare(obj::symbol const &s) const
  {
    if(equal(s))
    {
      return 0;
    }
    if(ns.empty() && !s.ns.empty())
    {
      return -1;
    }
    if(!ns.empty())
    {
      if(s.ns.empty())
      {
        return 1;
      }

      auto const ns_compare(ns.compare(s.ns));
      if(ns_compare != 0)
      {
        return ns_compare;
      }
    }
    return name.compare(s.name);
  }

  void to_string_impl(native_persistent_string const &ns,
                      native_persistent_string const &name,
                      fmt::memory_buffer &buff)
  {
    if(!ns.empty())
    {
      format_to(std::back_inserter(buff), FMT_COMPILE("{}/{}"), ns, name);
    }
    else
    {
      format_to(std::back_inserter(buff), FMT_COMPILE("{}"), name);
    }
  }

  void obj::symbol::to_string(fmt::memory_buffer &buff) const
  {
    to_string_impl(ns, name, buff);
  }

  native_persistent_string obj::symbol::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(ns, name, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::symbol::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::symbol::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(hash::string(name), hash::string(ns));
  }

  obj::symbol_ptr obj::symbol::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::symbol>(ns, name));
    ret->meta = meta;
    return ret;
  }

  native_persistent_string const &obj::symbol::get_name() const
  {
    return name;
  }

  native_persistent_string const &obj::symbol::get_namespace() const
  {
    return ns;
  }

  bool obj::symbol::operator==(obj::symbol const &rhs) const
  {
    return ns == rhs.ns && name == rhs.name;
  }

  bool obj::symbol::operator<(obj::symbol const &rhs) const
  {
    return to_hash() < rhs.to_hash();
  }

  void obj::symbol::set_ns(native_persistent_string const &s)
  {
    ns = s;
    hash = 0;
  }

  void obj::symbol::set_name(native_persistent_string const &s)
  {
    name = s;
    hash = 0;
  }
}
