#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  template <typename S>
  static void separate(symbol &sym, S &&s)
  {
    auto const found(s.find('/'));
    if(found != jtl::immutable_string::npos && s.size() > 1)
    {
      sym.ns = s.substr(0, found);
      sym.name = s.substr(found + 1);
    }
    else
    {
      sym.name = std::forward<S>(s);
    }
  }

  symbol::symbol(jtl::immutable_string const &d)
  {
    separate(*this, d);
  }

  symbol::symbol(jtl::immutable_string &&d)
  {
    separate(*this, std::move(d));
  }

  symbol::symbol(jtl::immutable_string const &ns, jtl::immutable_string const &n)
    : ns{ ns }
    , name{ n }
  {
  }

  symbol::symbol(jtl::immutable_string &&ns, jtl::immutable_string &&n)
    : ns{ std::move(ns) }
    , name{ std::move(n) }
  {
  }

  symbol::symbol(object_ptr const meta,
                 jtl::immutable_string const &ns,
                 jtl::immutable_string const &n)
    : ns{ ns }
    , name{ n }
    , meta{ meta }
  {
  }

  symbol::symbol(object_ptr const ns, object_ptr const n)
    : ns{ runtime::to_string(ns) }
    , name{ runtime::to_string(n) }
  {
  }

  native_bool symbol::equal(object const &o) const
  {
    if(o.type != object_type::symbol)
    {
      return false;
    }

    auto const s(expect_object<symbol>(&o));
    return ns == s->ns && name == s->name;
  }

  native_bool symbol::equal(symbol const &s) const
  {
    return ns == s.ns && name == s.name;
  }

  native_integer symbol::compare(object const &o) const
  {
    return visit_type<symbol>([this](auto const typed_o) { return compare(*typed_o); }, &o);
  }

  native_integer symbol::compare(symbol const &s) const
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

  static void to_string_impl(jtl::immutable_string const &ns,
                             jtl::immutable_string const &name,
                             util::string_builder &buff)
  {
    if(!ns.empty())
    {
      buff(ns)('/')(name);
    }
    else
    {
      buff(name);
    }
  }

  void symbol::to_string(util::string_builder &buff) const
  {
    to_string_impl(ns, name, buff);
  }

  jtl::immutable_string symbol::to_string() const
  {
    util::string_builder buff;
    to_string_impl(ns, name, buff);
    return buff.release();
  }

  jtl::immutable_string symbol::to_code_string() const
  {
    return to_string();
  }

  native_hash symbol::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(hash::string(name), hash::string(ns));
  }

  symbol_ptr symbol::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<symbol>(ns, name));
    ret->meta = meta;
    return ret;
  }

  jtl::immutable_string const &symbol::get_name() const
  {
    return name;
  }

  jtl::immutable_string const &symbol::get_namespace() const
  {
    return ns;
  }

  bool symbol::operator==(symbol const &rhs) const
  {
    return ns == rhs.ns && name == rhs.name;
  }

  bool symbol::operator<(symbol const &rhs) const
  {
    return to_hash() < rhs.to_hash();
  }

  void symbol::set_ns(jtl::immutable_string const &s)
  {
    ns = s;
    hash = 0;
  }

  void symbol::set_name(jtl::immutable_string const &s)
  {
    name = s;
    hash = 0;
  }
}

namespace std
{
  size_t
  hash<jank::runtime::obj::symbol>::operator()(jank::runtime::obj::symbol const &o) const noexcept
  {
    return o.to_hash();
  }

  size_t hash<jank::runtime::obj::symbol_ptr>::operator()(
    jank::runtime::obj::symbol_ptr const &o) const noexcept
  {
    return o->to_hash();
  }

  bool equal_to<jank::runtime::obj::symbol_ptr>::operator()(
    jank::runtime::obj::symbol_ptr const &lhs,
    jank::runtime::obj::symbol_ptr const &rhs) const noexcept
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return false;
    }
    return lhs->equal(*rhs);
  }
}
