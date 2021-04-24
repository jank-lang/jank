#pragma once

#include <prelude/object.hpp>

namespace jank
{
  /* TODO: Laziness. */
  inline object mapv(object const &f, object const &seq)
  {
    return seq.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_set(std::is_same_v<T, detail::set>);
        auto constexpr is_map(std::is_same_v<T, detail::map>);

        if constexpr(is_vector || is_set || is_map)
        {
          auto const * const func_ptr(detail::extract_function<object const*, object>(&f));
          if(!func_ptr)
          {
            /* TODO: Throw an error. */
            std::cout << "(mapv) not a function: " << f << std::endl;
            return JANK_NIL;
          }

          detail::vector_transient ret;

          if constexpr(is_vector || is_set)
          {
            for(auto const &e : data)
            { ret.push_back((*func_ptr)(e)); }
          }
          else if constexpr(is_map)
          {
            for(auto const &p : data)
            { ret.push_back((*func_ptr)(object{ detail::vector{ p.first, p.second } })); }
          }

          return object{ ret.persistent() };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(mapv) not a seq: " << seq << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object reduce(object const &f, object const &initial, object const &seq)
  {
    return seq.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_set(std::is_same_v<T, detail::set>);

        if constexpr(is_vector || is_set)
        {
          auto const * const func_ptr(detail::extract_function<object const*, object, object>(&f));
          if(!func_ptr)
          {
            /* TODO: Throw an error. */
            std::cout << "(reduce) not a function: " << f << std::endl;
            return JANK_NIL;
          }

          object acc{ initial };

          for(auto const &e : data)
          { acc = (*func_ptr)(acc, e); }

          return acc;
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(reduce) not a seq: " << seq << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* TODO: Laziness */
  inline object partition(object const &n, object const &seq)
  {
    if(n.get_kind() != object::kind::integer)
    {
      /* TODO: throw error */
      std::cout << "partition size must be an integer" << std::endl;
      return JANK_NIL;
    }
    auto const partition_size(*n.get<detail::integer>());

    return seq.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        /* TODO: set + map support */

        if constexpr(is_vector)
        {
          detail::vector_transient ret;
          auto const partitions(data.size() / partition_size);

          for(size_t i{}; i < partitions; ++i)
          {
            detail::vector_transient partition;

            for(size_t k{ i * partition_size }; k < (i * partition_size) + partition_size; ++k)
            { partition.push_back(data[k]); }

            ret.push_back(object{ partition.persistent() });
          }

          return object{ ret.persistent() };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(partition) not a seq: " << seq << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* TODO: Laziness */
  inline object range(object const &start, object const &end)
  {
    if(start.get_kind() != object::kind::integer
       || end.get_kind() != object::kind::integer)
    {
      /* TODO: throw error */
      std::cout << "range start/end must be an integer" << std::endl;
      return JANK_NIL;
    }
    else if(end < start)
    {
      /* TODO: throw error */
      std::cout << "range start must be < end" << std::endl;
      return JANK_NIL;
    }

    auto const start_int(*start.get<detail::integer>());
    auto const end_int(*end.get<detail::integer>());

    detail::vector_transient ret;
    for(auto i(start_int); i < end_int; ++i)
    { ret.push_back(i); }
    return ret.persistent();
  }

  inline object reverse(object const &seq)
  {
    return seq.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);

        if constexpr(is_vector)
        {
          detail::vector_transient ret;

          for(auto it(data.rbegin()); it != data.rend(); ++it)
          { ret.push_back(*it); }

          return ret.persistent();
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(range) not a seq: " << seq << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* TODO: This is too slow, due to copying out of the object. */
  inline object get(object const &o, object const &key)
  {
    switch(o.get_kind())
    {
      case object::kind::vector:
      {
        if(key.get_kind() != object::kind::integer)
        { return JANK_NIL; }

        auto const &data(o.expect<detail::vector>());
        auto const i(key.expect<detail::integer>());
        if(i < 0 || i >= data.size())
        { return JANK_NIL; }

        return data[i];
      }
      case object::kind::map:
      {
        auto const &data(o.expect<detail::map>());
        if(auto * const found = data.find(key))
        { return *found; }
        else
        { return JANK_NIL; }
      }
      default:
      {
        /* TODO: throw error */
        std::cout << "can only call get on associative types, not " << o << std::endl;
        return JANK_NIL;
      }
    }
  }

  inline object conj(object const &o, object const &val)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);

        /* TODO: Map support. */
        if constexpr(is_vector)
        { return object{ data.push_back(val) }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(conj) not a seq: " << o << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object assoc(object const &o, object const &key, object const &val)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_map(std::is_same_v<T, detail::map>);

        if constexpr(is_vector)
        {
          if(key.get_kind() != object::kind::integer)
          {
            /* TODO: throw error */
            std::cout << "vector assoc key must be an integer: " << key << std::endl;
            return JANK_NIL;
          }

          auto const i(*key.get<detail::integer>());
          if(i < 0 || i >= data.size())
          {
            /* TODO: Throw error */
            std::cout << "vector assoc key out of bounds: " << key << std::endl;
            return JANK_NIL;
          }

          return object{ data.set(i, val) };
        }
        else if constexpr(is_map)
        { return object{ data.set(key, val) }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "(assoc) not a seq: " << o << std::endl;
          return JANK_NIL;
        }
      }
    );
  }
}
