#pragma once

#include <string>
#include <memory>

namespace nrepl_server
{
  // Hide the implementation details behind a PIMPL so that we don't need to
  // expose ASIO in the header file.

  class client final
  {
  private:
    struct impl;

  public:
    /* Indicates that reading and writing may be performed on this client. */
    bool is_connected();

    /* Block until one or more bytes of data is read from the client. */
    std::string read_some();

    /* Write some data to the client. */
    void write_some(std::string const &data);

  protected:
    client(std::unique_ptr<client::impl> impl);

  private:
    std::unique_ptr<client::impl> impl_;

    // for protected ctor access
    friend class server;
  };

  class server final
  {
  private:
    struct impl;

  public:
    server(int port);

    /* Block until a client connects. */
    client *accept();


  private:
    std::unique_ptr<impl> impl_;
  };
} // namespace nrepl_server
