#include <string>
#include <memory>

namespace nrepl_server
{
  // Hide the implementation details behind a PIMPL so that we don't need to
  // expose ASIO in the header file.
  //
  // fwd-decl
  namespace detail
  {
    class nrepl_server_impl;
    class nrepl_client_impl;
  }

  class nrepl_client final
  {
  public:
    /* Indicates that reading and writing may be performed on this client. */
    bool is_connected();

    /* Block until one or more bytes of data is read from the client. */
    std::string read_some();

    /* Write some data to the client. */
    void write_some(std::string const &data);

  protected:
    nrepl_client(std::unique_ptr<detail::nrepl_client_impl> impl);

  private:
    std::unique_ptr<detail::nrepl_client_impl> impl_;

    // for protected ctor access
    friend class nrepl_server;
  };

  class nrepl_server final
  {
  public:
    nrepl_server(int port);
    ~nrepl_server();

    /* Block until a client connects. */
    nrepl_client *accept();

  private:
    std::unique_ptr<detail::nrepl_server_impl> impl_;
  };
} // namespace nrepl_server
