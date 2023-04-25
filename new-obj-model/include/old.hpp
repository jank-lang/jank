#pragma once

namespace old_model
{
  struct object : gc
  { };

  struct countable
  {
    virtual ~countable() = default;
    virtual size_t count() const = 0;
  };

  struct seqable
  {
    virtual ~seqable() = default;
    virtual void* seq() const = 0;
  };

  struct metadatable
  {
    virtual ~metadatable() = default;
    virtual void* with_meta() const = 0;

    void* meta{};
  };

  struct associatively_readable
  {
    virtual ~associatively_readable() = default;
    virtual void* get(void* key) const = 0;
  };

  struct associatively_writable
  {
    virtual ~associatively_writable() = default;
    virtual void* assoc(void* key, void* val) const = 0;
  };

  struct map : object, countable, seqable, metadatable, associatively_readable, associatively_writable
  {
    size_t count() const override
    { return 0; }
    void* seq() const override
    { return nullptr; }
    void * with_meta() const override
    { return nullptr; }
    void * get(void *) const override
    { return nullptr; }
    void * assoc(void *, void *) const override
    { return nullptr; }

    void* data{};
  };
}
