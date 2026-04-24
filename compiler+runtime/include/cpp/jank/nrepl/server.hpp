#pragma once

#include <string>
#include <memory>

#include <jtl/immutable_string.hpp>

namespace jank::nrepl::server
{
  struct native_client
  {
    struct impl;

    native_client(std::unique_ptr<impl> impl);

    /* Indicates that reading and writing may be performed on this client. */
    bool is_connected() const;

    /* Block until one or more bytes of data is read from the client. */
    std::string read_some() const;

    /* Write some data to the client. */
    void write_some(std::string const &data) const;

    std::unique_ptr<impl> impl_;
  };

  struct native_server
  {
    struct impl;

    native_server();

    jtl::immutable_string get_endpoint() const;
    u16 get_port() const;
    /* Block until a client connects. */
    native_client *accept() const;

    std::shared_ptr<impl> impl_;
  };
}
