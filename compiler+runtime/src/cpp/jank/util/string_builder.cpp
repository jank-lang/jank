#include <bit>
#include <codecvt>
#include <locale>

#include <jtl/assert.hpp>

#include <jank/util/string_builder.hpp>

namespace jank::util
{
  using allocator_type = native_allocator<string_builder::value_type>;
  using allocator_traits = std::allocator_traits<allocator_type>;

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  static allocator_type allocator;

  static void realloc(string_builder &sb, usize const required)
  {
    auto const new_capacity{ std::bit_ceil(required) };
    /* TODO: Pointer-free GC alloc. */
    auto const new_data{ allocator_traits::allocate(allocator, new_capacity) };
    string_builder::traits_type::copy(new_data, sb.buffer, sb.pos);
    allocator_traits::deallocate(allocator, sb.buffer, sb.pos);
    sb.buffer = new_data;
    sb.capacity = new_capacity;
  }

  static void maybe_realloc(string_builder &sb, usize const additional_size)
  {
    auto const required_size{ sb.pos + additional_size + 1 };
    if(sb.capacity < required_size)
    {
      realloc(sb, required_size);
    }
  }

  static void write(string_builder &sb, char const * const str, usize const size)
  {
    string_builder::traits_type::copy(sb.buffer + sb.pos, str, size);
    sb.pos += size;
  }

  string_builder::string_builder()
  {
    realloc(*this, capacity);
  }

  string_builder::string_builder(usize const capacity)
    : capacity{ capacity }
  {
    realloc(*this, capacity);
  }

  string_builder::~string_builder()
  {
    allocator_traits::deallocate(allocator, buffer, pos);
  }

  string_builder &string_builder::operator()(bool const d) &
  {
    if(d)
    {
      auto const required{ 4 };
      maybe_realloc(*this, required);
      write(*this, "true", required);
    }
    else
    {
      auto const required{ 5 };
      maybe_realloc(*this, required);
      write(*this, "false", required);
    }

    return *this;
  }

  string_builder &string_builder::operator()(float const d) &
  {
    /* snprintf %f implicitly casts to double anyway. */
    return (*this)(static_cast<double>(d));
  }

  string_builder &string_builder::operator()(double const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%f", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%f", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(uhash const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%d", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%d", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(void const * const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%p", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%p", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(int const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%d", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%d", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(long const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%li", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%li", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(long long const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%lld", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%lld", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(unsigned long const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%lu", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%lu", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(unsigned long long const d) &
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const required{ snprintf(nullptr, 0, "%llu", d) };
    maybe_realloc(*this, required);

    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    snprintf(buffer + pos, capacity - pos, "%llu", d);
    pos += required;

    return *this;
  }

  string_builder &string_builder::operator()(char const d) &
  {
    maybe_realloc(*this, 1);

    buffer[pos] = d;
    ++pos;

    return *this;
  }

  string_builder &string_builder::operator()(char32_t const d) &
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
#pragma clang diagnostic pop
    return (*this)(converter.to_bytes(d));
  }

  string_builder &string_builder::operator()(char const * const d) &
  {
    auto const required{ strlen(d) };
    maybe_realloc(*this, required);

    write(*this, d, required);

    return *this;
  }

  string_builder &string_builder::operator()(native_transient_string const &d) &
  {
    auto const required{ d.size() };
    maybe_realloc(*this, required);

    write(*this, d.data(), required);

    return *this;
  }

  string_builder &string_builder::operator()(jtl::immutable_string const &d) &
  {
    auto const required{ d.size() };
    maybe_realloc(*this, required);

    write(*this, d.data(), required);

    return *this;
  }

  void string_builder::push_back(bool const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(float const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(double const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(uhash const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(void const * const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(int const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(long const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(long long const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(unsigned long const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(unsigned long long const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char32_t const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(char const * const d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(native_transient_string const &d) &
  {
    (*this)(d);
  }

  void string_builder::push_back(jtl::immutable_string const &d) &
  {
    (*this)(d);
  }

  void string_builder::reserve(usize const new_capacity)
  {
    if(capacity < new_capacity)
    {
      realloc(*this, new_capacity);
    }
  }

  string_builder::value_type *string_builder::data() const
  {
    return buffer;
  }

  usize string_builder::size() const
  {
    return pos;
  }

  jtl::immutable_string string_builder::release()
  {
    jank_debug_assert(pos < capacity);

    jtl::immutable_string ret;
    if(pos <= jtl::immutable_string::max_small_size)
    {
      ret.init_small(buffer, pos);
    }
    else
    {
      ret.init_large_shared(buffer, pos);
    }

    pos = capacity = 0;
    buffer = nullptr;

    return ret;
  }

  native_transient_string string_builder::str() const
  {
    jank_debug_assert(pos < capacity);
    buffer[pos] = 0;
    return { buffer, pos };
  }

  native_persistent_string_view string_builder::view() const &
  {
    jank_debug_assert(pos < capacity);
    buffer[pos] = 0;
    return { buffer, pos };
  }
}
