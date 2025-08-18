#include "nrepl_server.hpp"

#include <iostream>

#include <boost/asio.hpp>

namespace nrepl_server
{
  using namespace boost::asio;
  using namespace boost::asio::ip;

  namespace detail
  {
    // client
    class nrepl_client_impl
    {
    public:
      nrepl_client_impl(boost::asio::io_context &io_context)
        : socket_(io_context)
      {
      }

      void accept(tcp::acceptor &acceptor)
      {
        acceptor.accept(socket_);
      }

      bool is_open()
      {
        return socket_.is_open();
      }

      std::string read_some()
      {
        boost::system::error_code error_code;
        std::size_t length = socket_.read_some(buffer(rx_buf_, rx_capacity), error_code);
        return { rx_buf_, length };
      }

      void write_some(std::string const &message)
      {
        boost::system::error_code error_code;
        socket_.write_some(buffer(message), error_code);
      }

    private:
      tcp::socket socket_;

      static constexpr std::size_t rx_capacity = 1024 * 1024; // 1MiB
      char rx_buf_[rx_capacity];
    };

    // server
    class nrepl_server_impl
    {
    public:
      nrepl_server_impl(tcp::endpoint const &endpoint)
        : io_context_()
        , acceptor_(io_context_, endpoint)
      {
      }

      std::unique_ptr<nrepl_client_impl> accept()
      {
        auto impl = std::make_unique<nrepl_client_impl>(io_context_);
        impl->accept(acceptor_);

        return impl;
      }

    private:
      io_context io_context_;
      tcp::acceptor acceptor_;
    };
  }

  // client
  nrepl_client::nrepl_client(std::unique_ptr<detail::nrepl_client_impl> impl)
    : impl_(std::move(impl))
  {
  }

  bool nrepl_client::is_open()
  {
    return impl_->is_open();
  }

  std::string nrepl_client::read_some()
  {
    auto data = impl_->read_some();
    std::cout << "<- " << data << std::endl;
    return data;
  }

  void nrepl_client::write_some(std::string const &data)
  {
    std::cout << "-> " << data << std::endl;
    impl_->write_some(data);
  }

  // server
  nrepl_server::nrepl_server(int port)
    : impl_(std::make_unique<detail::nrepl_server_impl>(
        tcp::endpoint(ip::address_v4::loopback(), port)))
  {
  }

  nrepl_client *nrepl_server::accept()
  {
    auto impl = impl_->accept();

    // TODO: This leaks memory but jank complains about not being able to delete
    // an opaque type if we return unique_ptr<nrepl_client>.
    return new nrepl_client(std::move(impl));
  }
}
