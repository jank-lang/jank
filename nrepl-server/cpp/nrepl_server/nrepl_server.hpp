#pragma once

#include <string>
#include <memory>

namespace nrepl_server
{
  // Hide the implementation details behind a PIMPL so that we don't need to
  // expose ASIO in the header file.

  class native_client final
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
    native_client(std::unique_ptr<native_client::impl> impl);

  private:
    std::unique_ptr<native_client::impl> impl_;

    // for protected ctor access
    friend class native_server;
  };

  class native_server final
  {
  private:
    struct impl;

  public:
    native_server(int port);
    ~native_server();

    /* Block until a client connects. */
    native_client *accept();


  private:
    std::shared_ptr<impl> impl_;
  };
} // namespace nrepl_server
