#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <vector>
#include <set>
#include <map>
#include <any>
#include <functional>

namespace jank
{
  namespace detail
  {
    using integer = int64_t;
    using real = double;
    using boolean = bool;
    using string = std::string;

    struct function
    {
      template <typename T>
      using value_type = std::function<T>;

      template <typename R, typename... Args>
      function(R (* const f)(Args...)) : function(value_type<R (Args...)>{ f })
      { }
      template <typename R, typename... Args>
      function(value_type<R (Args...)> &&f) : value{ std::move(f) }
      { }
      template <typename R, typename... Args>
      function(value_type<R (Args...)> const &f) : value{ f }
      { }

      template <typename F>
      F const* get() const
      { return std::any_cast<F>(&value); }

      std::any value;
    };
    inline bool operator==(function const &, function const &)
    { return true; }
    inline bool operator!=(function const &, function const &)
    { return false; }
    inline bool operator<(function const &, function const &)
    { return true; }

    struct nil
    { };
    inline bool operator==(nil const &, nil const &)
    { return true; }
    inline bool operator!=(nil const &, nil const &)
    { return false; }
    inline bool operator<(nil const &, nil const &)
    { return true; }

    template <typename T, typename Enable = void>
    struct conversion
    { using type = T; };
    template <typename T>
    struct conversion<T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, boolean>>>
    { using type = integer; };
    template <typename T>
    struct conversion<T, std::enable_if_t<std::is_floating_point_v<T>>>
    { using type = real; };
    template <typename T>
    struct conversion<T, std::enable_if_t<std::is_convertible_v<T, char const*>>>
    { using type = string; };
    template <typename R, typename... Args>
    struct conversion<R (*)(Args...)>
    { using type = function; };
    template <typename R, typename... Args>
    struct conversion<function::value_type<R (Args...)>>
    { using type = function; };

    template <typename T>
    using conversion_t = typename conversion<T>::type;
  }

  class object
  {
    public:
      enum class kind
      { nil, integer, real, boolean, string, vector, set, map, function };

      using vector_type = std::vector<object>;
      using set_type = std::set<object>;
      using map_type = std::map<object, object>;

      object() = default;
      ~object()
      { unset(); }

      template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, object>, bool> = true>
      object(T &&data)
      { set(std::forward<T>(data)); }
      object(object &&o)
      { *this = std::move(o); }
      object(object const &o)
      { *this = o; }

      template <typename F>
      auto visit(F const &f) const -> decltype(f(detail::nil{}))
      {
        switch(current_kind)
        {
          case object::kind::integer:
            return f(current_data.int_data);
          case object::kind::real:
            return f(current_data.real_data);
          case object::kind::boolean:
            return f(current_data.bool_data);
          case object::kind::string:
            return f(current_data.string_data);
          case object::kind::vector:
            return f(current_data.vector_data);
          case object::kind::set:
            return f(current_data.set_data);
          case object::kind::map:
            return f(current_data.map_data);
          case object::kind::function:
            return f(current_data.function_data);
          case object::kind::nil:
          default:
            return f(current_data.nil_data);
        }
      }
      template <typename F>
      auto visit_with(F const &f, object const &other) const
      {
        return visit
        (
          [&](auto const &left)
          {
            return other.visit
            (
              [&](auto const &right)
              { return f(left, right); }
            );
          }
        );
      }


      object& operator=(object &&o)
      {
        if(&o == this)
        { return *this; }

        unset();
        current_kind = o.current_kind;

        switch(o.current_kind)
        {
          case object::kind::string:
            set(std::move(o.current_data.string_data));
            break;
          case object::kind::vector:
            set(std::move(o.current_data.vector_data));
            break;
          case object::kind::set:
            set(std::move(o.current_data.set_data));
            break;
          case object::kind::map:
            set(std::move(o.current_data.map_data));
            break;
          case object::kind::function:
            set(std::move(o.current_data.function_data));
            break;
          default:
            *this = static_cast<object const&>(o);
        }

        o.current_kind = kind::nil;
        return *this;
      }
      object& operator=(object const &o)
      {
        if(&o == this)
        { return *this; }

        unset();

        switch(o.current_kind)
        {
          case object::kind::nil:
            set(o.current_data.nil_data);
            break;
          case object::kind::integer:
            set(o.current_data.int_data);
            break;
          case object::kind::real:
            set(o.current_data.real_data);
            break;
          case object::kind::boolean:
            set(o.current_data.bool_data);
            break;
          case object::kind::string:
            set(o.current_data.string_data);
            break;
          case object::kind::vector:
            set(o.current_data.vector_data);
            break;
          case object::kind::set:
            set(o.current_data.set_data);
            break;
          case object::kind::map:
            set(o.current_data.map_data);
            break;
          case object::kind::function:
            set(o.current_data.function_data);
            break;
        }

        return *this;
      }
      template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, object>, bool> = true>
      object& operator=(T &&data)
      {
        unset();
        set(std::forward<T>(data));
        return *this;
      }

      bool operator==(object const &o) const
      { return !(*this != o); }
      bool operator!=(object const &o) const
      {
        if(&o == this)
        { return false; }
        else if(current_kind == o.current_kind)
        { return false; }
        return visit_with
        (
          [](auto const &l, auto const &r) -> bool
          {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            if constexpr(std::is_same_v<L, R>)
            { return l != r; }
            else
            { return true; }
          },
          o
        );
      }

      bool operator<(object const &o) const
      {
        if(&o == this)
        { return false; }
        else if(static_cast<size_t>(current_kind) < static_cast<size_t>(o.current_kind))
        { return true; }
        return visit_with
        (
          [](auto const &l, auto const &r) -> bool
          {
            using L = std::decay_t<decltype(l)>;
            using R = std::decay_t<decltype(r)>;

            /* TODO: Handle comparable types. */
            if constexpr(std::is_same_v<L, R>)
            { return l < r; }
            else
            { return true; }
          },
          o
        );
      }

      /* TODO: Add `expect` and return a ref; assert kind. */
      template <typename T>
      T const * get() const
      {
        return visit
        (
          [](auto const &data) -> T const*
          {
            using D = std::decay_t<decltype(data)>;
            if constexpr(std::is_same_v<T, D>)
            { return &data; }
            else
            { return nullptr; }
          }
        );
      }

      kind get_kind() const
      { return current_kind; }

    private:
      template <typename T>
      static kind constexpr type_to_kind()
      {
        if constexpr(std::is_same_v<detail::nil, T>)
        { return kind::nil; }
        else if constexpr(std::is_same_v<detail::integer, T>)
        { return kind::integer; }
        else if constexpr(std::is_same_v<detail::real, T>)
        { return kind::real; }
        else if constexpr(std::is_same_v<detail::boolean, T>)
        { return kind::boolean; }
        else if constexpr(std::is_same_v<detail::string, T>)
        { return kind::string; }
        else if constexpr(std::is_same_v<vector_type, T>)
        { return kind::vector; }
        else if constexpr(std::is_same_v<set_type, T>)
        { return kind::set; }
        else if constexpr(std::is_same_v<map_type, T>)
        { return kind::map; }
        else if constexpr(std::is_same_v<detail::function, T>)
        { return kind::function; }
        else
        {
          static_assert((T*)nullptr, "invalid type_to_kind");
          return kind::nil; /* Please the compiler */
        }
      }

      template <typename T>
      void set(T &&new_data)
      {
        using converted_type = detail::conversion_t<std::decay_t<T>>;

        kind constexpr k{ type_to_kind<converted_type>() };

        if constexpr(k == kind::nil)
        { current_data.nil_data = new_data; }
        else if constexpr(k == kind::integer)
        { current_data.int_data = new_data; }
        else if constexpr(k == kind::real)
        { current_data.real_data = new_data; }
        else if constexpr(k == kind::boolean)
        { current_data.bool_data = new_data; }
        else if constexpr(k == kind::string)
        { new (&current_data.string_data) detail::string(std::forward<T>(new_data)); }
        else if constexpr(k == kind::vector)
        { new (&current_data.vector_data) vector_type(std::forward<T>(new_data)); }
        else if constexpr(k == kind::set)
        { new (&current_data.set_data) set_type(std::forward<T>(new_data)); }
        else if constexpr(k == kind::map)
        { new (&current_data.map_data) map_type(std::forward<T>(new_data)); }
        else if constexpr(k == kind::function)
        { new (&current_data.function_data) detail::function(std::forward<T>(new_data)); }
        else
        { static_assert((T*)nullptr, "invalid variant input"); }

        current_kind = k;
      }

      void unset()
      {
        switch(current_kind)
        {
          case kind::string:
            using detail::string;
            current_data.string_data.~string();
            break;
          case kind::vector:
            current_data.vector_data.~vector_type();
            break;
          case kind::set:
            current_data.set_data.~set_type();
            break;
          case kind::map:
            current_data.map_data.~map_type();
            break;
          case kind::function:
            using detail::function;
            current_data.function_data.~function();
            break;
          default:
            break;
        }
        current_kind = kind::nil;
      }

      kind current_kind{ kind::nil };
      union data_union
      {
        data_union()
        { }
        ~data_union()
        { }

        detail::nil nil_data;
        detail::integer int_data;
        detail::real real_data;
        detail::boolean bool_data;
        detail::string string_data;
        vector_type vector_data;
        set_type set_data;
        map_type map_data;
        detail::function function_data;
      } current_data;

      friend std::ostream& operator<<(std::ostream&, object const&);
  };

  std::ostream& operator<<(std::ostream &os, object const &o)
  {
    switch(o.current_kind)
    {
      case object::kind::nil:
        os << "nil";
        break;
      case object::kind::integer:
        os << o.current_data.int_data;
        break;
      case object::kind::real:
        os << o.current_data.real_data;
        break;
      case object::kind::boolean:
        os << (o.current_data.bool_data ? "true" : "false");
        break;
      case object::kind::string:
        os << "\"" << o.current_data.string_data << "\"";
        break;
      case object::kind::vector:
        os << "[";
        std::copy
        (
          std::begin(o.current_data.vector_data),
          std::end(o.current_data.vector_data),
          std::experimental::make_ostream_joiner(os, ", ")
        );
        os << "]";
        break;
      case object::kind::set:
        os << "#{";
        std::copy
        (
          std::begin(o.current_data.vector_data),
          std::end(o.current_data.vector_data),
          std::experimental::make_ostream_joiner(os, ", ")
        );
        os << "}";
        break;
      case object::kind::map:
        os << "{";
        for(auto i(o.current_data.map_data.begin()); i != o.current_data.map_data.end(); ++i)
        {
          /* TODO: Delim. */
          os << i->first << " " << i->second << " ";
        }
        os << "}";
        break;
      case object::kind::function:
        os << "<function>";
        break;
    }
    return os;
  }

  namespace detail
  {
    using vector = object::vector_type;
    using set = object::set_type;
    using map = object::map_type;
  }

  /* TODO: Get rid of these. */
  using JANK_OBJECT = object;
  using JANK_INTEGER = object;
  using JANK_REAL = object;
  using JANK_BOOL = object;
  using JANK_STRING = object;

  template<typename... Ts>
  object JANK_VECTOR(Ts &&... args)
  { return object{ detail::vector{ std::forward<Ts>(args)... } }; }
  template<typename... Ts>
  object JANK_SET(Ts &&... args)
  { return object{ detail::set{ std::forward<Ts>(args)... } }; }

  inline detail::map::value_type JANK_MAP_ENTRY(object const &k, object const &v)
  { return { k, v }; }
  template<typename... Ts>
  object JANK_MAP(Ts &&... entries)
  { return object{ detail::map{ std::forward<Ts>(entries)... } }; }

  static jank::object const JANK_NIL{ detail::nil{} };
  static jank::object const JANK_TRUE{ true };
  static jank::object const JANK_FALSE{ false };

  namespace detail
  {
    template <size_t N, typename... Args>
    struct build_arity
    { using type = typename build_arity<N - 1, Args..., object>::type; };
    template <typename... Args>
    struct build_arity<0, Args...>
    { using type = object (Args const&...); };

    template <typename F, typename... Args>
    object invoke(F const &f, Args &&... args)
    {
      if constexpr(std::is_function_v<std::remove_pointer_t<std::decay_t<decltype(f)>>>)
      { return f(std::forward<Args>(args)...); }
      else
      {
        size_t constexpr arg_count{ sizeof...(args) };
        using arity = typename build_arity<arg_count>::type;
        using function_type = detail::function::value_type<arity>;

        auto const * const func(f->template get<detail::function>());
        if(!func)
        {
          /* TODO: Throw error. */
          std::cout << "object is not a function" << std::endl;
          return JANK_NIL;
        }

        auto const * const func_ptr(func->template get<function_type>());
        if(!func_ptr)
        {
          /* TODO: Throw error. */
          std::cout << "invalid function arity" << std::endl;
          return JANK_NIL;
        }

        return (*func_ptr)(std::forward<Args>(args)...);
      }
    }
  }
}
