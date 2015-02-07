#pragma once

/* TODO: ident, real, etc. */
enum class cell_type
{
  integer,
  string,
  list,
  function /* TODO: implement. */
};

template <cell_type C>
char constexpr const* cell_type_string();
template <>
inline char constexpr const* cell_type_string<cell_type::integer>()
{ return "integer"; }
template <>
inline char constexpr const* cell_type_string<cell_type::string>()
{ return "string"; }
template <>
inline char constexpr const* cell_type_string<cell_type::list>()
{ return "list"; }
template <>
inline char constexpr const* cell_type_string<cell_type::function>()
{ return "function"; }

inline char constexpr const* cell_type_string(cell_type const c)
{
  switch(c)
  {
    case cell_type::integer:
      return cell_type_string<cell_type::integer>();
    case cell_type::string:
      return cell_type_string<cell_type::string>();
    case cell_type::list:
      return cell_type_string<cell_type::list>();
    case cell_type::function:
      return cell_type_string<cell_type::function>();
    default:
      return "unknown";
  }
}
