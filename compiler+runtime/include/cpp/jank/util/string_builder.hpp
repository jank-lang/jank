#pragma once

#include <jank/type.hpp>

namespace jank::util
{
  struct string_builder
  {
    static constexpr size_t initial_capacity{ 32 };

    using value_type = char;
    using traits_type = std::char_traits<value_type>;

    string_builder();
    string_builder(size_t capacity);
    string_builder(string_builder const &) = delete;
    string_builder(string_builder &&) = delete;
    ~string_builder();

    string_builder &operator=(string_builder const &) = delete;
    string_builder &operator=(string_builder &&) = delete;

    string_builder &operator()(native_bool d) &;
    string_builder &operator()(native_integer d) &;
    string_builder &operator()(mpz_class const &d) &;
    string_builder &operator()(native_real d) &;
    string_builder &operator()(native_hash d) &;
    string_builder &operator()(void const *d) &;
    string_builder &operator()(int d) &;
    string_builder &operator()(size_t d) &;
    string_builder &operator()(char d) &;
    string_builder &operator()(char32_t d) &;
    string_builder &operator()(char const *d) &;
    string_builder &operator()(native_transient_string const &d) &;
    string_builder &operator()(native_persistent_string const &d) &;

    void push_back(native_bool d) &;
    void push_back(native_integer d) &;
    void push_back(native_real d) &;
    void push_back(native_hash d) &;
    void push_back(void const *d) &;
    void push_back(int d) &;
    void push_back(size_t d) &;
    void push_back(char d) &;
    void push_back(char32_t d) &;
    void push_back(char const *d) &;
    void push_back(native_transient_string const &d) &;
    void push_back(native_persistent_string const &d) &;

    void reserve(size_t capacity);
    value_type *data() const;
    size_t size() const;

    native_persistent_string release();
    native_transient_string str() const;
    native_persistent_string_view view() const &;

    value_type *buffer{};
    size_t pos{};
    size_t capacity{ initial_capacity };
  };
}
