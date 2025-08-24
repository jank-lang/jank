#include "nrepl_server.hpp"

#include <boost/asio.hpp>

namespace nrepl_server
{
  using namespace boost::asio;
  using namespace boost::asio::ip;

  // client
  class client::impl
  {
  public:
    impl(boost::asio::io_context &io_context)
      : socket_(io_context)
    {
    }

    void accept(tcp::acceptor &acceptor)
    {
      acceptor.accept(socket_);
      connected_ = true;
    }

    bool is_connected() const
    {
      return connected_;
    }

    std::string read_some()
    {
      boost::system::error_code error;

      std::size_t length = socket_.read_some(buffer(rx_buf_, rx_capacity), error);
      if(error == boost::asio::error::eof || error == boost::asio::error::connection_reset)
      {
        connected_ = false;
        return "";
      }

      return { rx_buf_, length };
    }

    void write_some(std::string const &message)
    {
      boost::system::error_code error_code;
      socket_.write_some(buffer(message), error_code);
    }

  private:
    tcp::socket socket_;
    bool connected_{ false };

    static constexpr std::size_t rx_capacity{ 1024ul * 1024ul }; // 1MiB
    char rx_buf_[rx_capacity]{};
  };

  // client
  client::client(std::unique_ptr<client::impl> impl)
    : impl_(std::move(impl))
  {
  }

  bool client::is_connected()
  {
    return impl_->is_connected();
  }

  std::string client::read_some()
  {
    auto data = impl_->read_some();
    return data;
  }

  void client::write_some(std::string const &data)
  {
    impl_->write_some(data);
  }

  // server
  class server::impl
  {
  public:
    impl(tcp::endpoint const &endpoint)
      : io_context_()
      , acceptor_(io_context_, endpoint)
    {
    }

    std::unique_ptr<client::impl> accept()
    {
      auto impl = std::make_unique<client::impl>(io_context_);
      impl->accept(acceptor_);

      return impl;
    }

  private:
    io_context io_context_;
    tcp::acceptor acceptor_;
  };

  server::server(int port)
    : impl_(std::make_unique<server::impl>(tcp::endpoint(ip::address_v4::loopback(), port)))
  {
  }

  server::~server() = default;

  client *server::accept()
  {
    auto impl = impl_->accept();

    // TODO: This leaks memory but jank complains about not being able to delete
    // an opaque type if we return unique_ptr<client>.
    return new client(std::move(impl));
  }
}
