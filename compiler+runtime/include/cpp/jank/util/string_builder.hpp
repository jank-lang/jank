#pragma once

#include <jtl/ptr.hpp>

#include <jank/type.hpp>

namespace jank::util
{
  enum class terminal_color : u8;

  struct string_builder
  {
    static constexpr usize initial_capacity{ 32 };

    using value_type = char;
    using traits_type = std::char_traits<value_type>;

    string_builder();
    string_builder(usize capacity);
    string_builder(string_builder const &) = delete;
    string_builder(string_builder &&) = delete;
    ~string_builder();

    string_builder &operator=(string_builder const &) = delete;
    string_builder &operator=(string_builder &&) = delete;

    string_builder &operator()(bool d) &;
    string_builder &operator()(float d) &;
    string_builder &operator()(double d) &;
    string_builder &operator()(uhash d) &;
    string_builder &operator()(void const *d) &;
    string_builder &operator()(jtl::ptr<void> d) &;
    string_builder &operator()(int d) &;
    string_builder &operator()(long d) &;
    string_builder &operator()(long long d) &;
    string_builder &operator()(native_big_integer const &d) &;
    string_builder &operator()(unsigned long d) &;
    string_builder &operator()(unsigned long long d) &;
    string_builder &operator()(char d) &;
    string_builder &operator()(char32_t d) &;
    string_builder &operator()(char const *d) &;
    string_builder &operator()(native_transient_string const &d) &;
    string_builder &operator()(jtl::immutable_string const &d) &;
    string_builder &operator()(terminal_color c) &;

    template <template <typename> typename V, typename T>
    requires(std::same_as<V<T>, std::vector<T>> || std::same_as<V<T>, native_vector<T>>)
    string_builder &operator()(V<T> const &d) &
    {
      (*this)("[ ");
      for(size_t i{}; i < d.size(); ++i)
      {
        (*this)(d[i]);
        if(i + 1 != d.size())
        {
          (*this)(',');
        }
        (*this)(' ');
      }
      (*this)(']');
      return *this;
    }

    void push_back(bool d) &;
    void push_back(float d) &;
    void push_back(double d) &;
    void push_back(uhash d) &;
    void push_back(void const *d) &;
    void push_back(int d) &;
    void push_back(long d) &;
    void push_back(long long d) &;
    void push_back(unsigned long d) &;
    void push_back(unsigned long long d) &;
    void push_back(char d) &;
    void push_back(char32_t d) &;
    void push_back(char const *d) &;
    void push_back(native_transient_string const &d) &;
    void push_back(jtl::immutable_string const &d) &;

    void reserve(usize capacity);
    value_type *data() const;
    usize size() const;

    jtl::immutable_string release();
    native_transient_string str() const;
    native_persistent_string_view view() const &;

    value_type *buffer{};
    usize pos{};
    usize capacity{ initial_capacity };
  };
}
