#pragma once

#include <memory>

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <jank/runtime/detail/type.hpp>

namespace jank::runtime
{
  namespace obj
  {
    struct map;
    struct vector;
    struct string;
    struct set;
    struct integer;
    struct real;
  }

  template <typename T>
  union pool_item
  {
    using storage_type = char[sizeof(T)];

    pool_item<T>* get_next_item() const
    { return next; }
    void set_next_item(pool_item<T> *n)
    { next = n; }

    T* get_data()
    { return reinterpret_cast<T*>(data); }

    static pool_item<T>* storage_to_item(T * const t)
    { return reinterpret_cast<pool_item<T>*>(t); }

    pool_item<T> *next;
    alignas(alignof(T)) storage_type data;
  };

  template <typename T>
  struct pool_arena
  {
    pool_arena(size_t const arena_size)
      : items{ std::make_unique<pool_item<T>[]>(arena_size) }
    {
      for(size_t i{ 1 }; i < arena_size; ++i)
      { items[i - 1].set_next_item(&items[i]); }
      items[arena_size - 1].set_next_item(nullptr);
    }

    pool_item<T>* get_items() const
    { return items.get(); }

    void set_next_arena(std::unique_ptr<pool_arena<T>> &&n)
    {
      assert(!next);
      next.reset(n.release());
    }

    std::unique_ptr<pool_item<T>[]> items;
    std::unique_ptr<pool_arena<T>> next;
  };

  /* TODO: Rename to memory_pool. */
  template <typename T>
  struct pool
  {
    pool(size_t arena_size)
      : arena_size{ arena_size }
      , current_arena{ std::make_unique<pool_arena<T>>(arena_size) }
      , free_list{ current_arena->get_items() }
    {}

    template <typename... Args>
    boost::intrusive_ptr<T> allocate(Args &&... args)
    {
      /* If the current arena is full, create a new one. */
      if(free_list == nullptr)
      {
        auto new_arena(std::make_unique<pool_arena<T>>(arena_size));
        /* Link the new arena to the current one. */
        new_arena->set_next_arena(std::move(current_arena));
        current_arena.reset(new_arena.release());
        free_list = current_arena->get_items();
      }

      pool_item<T> *first_item = free_list;
      free_list = first_item->get_next_item();

      T * const result{ first_item->get_data() };
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      new (result) T{ std::forward<Args>(args)... };
      result->owner_pool = this;

      return result;
    }

    void free(T *t)
    {
      assert(t);
      t->T::~T();

      /* Add the item at the beginning of the free list. */
      pool_item<T> *item{ pool_item<T>::storage_to_item(t) };
      item->set_next_item(free_list);
      free_list = item;
    }

    size_t arena_size;
    std::unique_ptr<pool_arena<T>> current_arena;
    pool_item<T> *free_list;
  };

  template <typename C>
  pool<C>& get_pool()
  {
    static pool<C> p{ 16 };
    return p;
  }
  template <>
  pool<obj::integer>& get_pool<obj::integer>();
  template <>
  pool<obj::real>& get_pool<obj::real>();
  template <>
  pool<obj::string>& get_pool<obj::string>();
  template <>
  pool<obj::vector>& get_pool<obj::vector>();
  template <>
  pool<obj::map>& get_pool<obj::map>();
  template <>
  pool<obj::set>& get_pool<obj::set>();

  struct pool_item_common_base
  {
    virtual ~pool_item_common_base() = default;
    virtual void inc_reference() = 0;
    virtual void release() = 0;
  };

  template <typename T>
  struct pool_item_base : virtual pool_item_common_base
  {
    void inc_reference() override
    { ++reference_count; }
    void release() override
    {
      if(--reference_count == 0)
      { owner_pool->free(static_cast<T*>(this)); }
    }

    boost::intrusive_ptr<T> ptr_from_this()
    { return { static_cast<T*>(this), true }; }
    boost::intrusive_ptr<T> ptr_from_this() const
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    { return const_cast<T*>(static_cast<T const*>(this))->ptr_from_this(); }

    pool<T> *owner_pool{};
    size_t reference_count{};
  };

  template <typename T, typename std::enable_if_t<std::is_base_of_v<pool_item_base<T>, T>, bool> = true>
  void intrusive_ptr_add_ref(T * const p)
  { ++p->reference_count; }
  template <typename T, typename std::enable_if_t<std::is_base_of_v<pool_item_base<T>, T>, bool> = true>
  void intrusive_ptr_release(T * const p)
  {
    assert(p->owner_pool);
    if(--p->reference_count == 0)
    { p->owner_pool->free(p); }
  }

  template <typename T, typename std::enable_if_t<std::is_base_of_v<pool_item_common_base, T> && !std::is_base_of_v<pool_item_base<T>, T>, bool> = true>
  void intrusive_ptr_add_ref(T * const p)
  { p->inc_reference(); }
  template <typename T, typename std::enable_if_t<std::is_base_of_v<pool_item_common_base, T> && !std::is_base_of_v<pool_item_base<T>, T>, bool> = true>
  void intrusive_ptr_release(T * const p)
  { p->release(); }

  template <typename T>
  inline detail::box_type<T> make_box(detail::box_type<T> const &o)
  { return o; }
  template <typename C>
  auto make_box()
  { return get_pool<C>().allocate(); }
  template <typename C, typename... Args>
  auto make_box(Args &&... args)
  { return get_pool<C>().allocate(std::forward<Args>(args)...); }
}
