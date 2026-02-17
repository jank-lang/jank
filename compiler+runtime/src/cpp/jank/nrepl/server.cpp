#include <boost/asio.hpp>

#include <jtl/string_builder.hpp>

#include <jank/nrepl/server.hpp>
#include <jank/util/fmt.hpp>

namespace jank::nrepl::server
{
  using namespace boost::asio;
  using namespace boost::asio::ip;

  struct native_client::impl
  {
    impl(boost::asio::io_context &io_context)
      : socket_{ io_context }
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

      auto const length{ socket_.read_some(buffer(rx_buf_, rx_capacity), error) };
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

    tcp::socket socket_;
    bool connected_{ false };

    static constexpr std::size_t rx_capacity{ 1024ul * 1024ul }; // 1MiB
    char rx_buf_[rx_capacity]{};
  };

  native_client::native_client(std::unique_ptr<native_client::impl> impl)
    : impl_{ std::move(impl) }
  {
  }

  bool native_client::is_connected() const
  {
    return impl_->is_connected();
  }

  std::string native_client::read_some() const
  {
    return impl_->read_some();
  }

  void native_client::write_some(std::string const &data) const
  {
    impl_->write_some(data);
  }

  // server
  struct native_server::impl
  {
    impl(tcp::endpoint const &endpoint)
      : acceptor_{ io_context_, endpoint }
    {
    }

    std::unique_ptr<native_client::impl> accept()
    {
      auto impl{ std::make_unique<native_client::impl>(io_context_) };
      impl->accept(acceptor_);

      return impl;
    }

    io_context io_context_;
    tcp::acceptor acceptor_;
  };

  native_server::native_server()
    : impl_{ std::make_shared<native_server::impl>(tcp::endpoint(ip::address_v4::loopback(), 0)) }
  {
  }

  jtl::immutable_string native_server::get_endpoint() const
  {
    jtl::string_builder sb;
    auto const &endpoint{ impl_->acceptor_.local_endpoint() };
    util::format_to(sb, "nrepl://{}:{}", endpoint.address().to_string(), endpoint.port());
    return sb.release();
  }

  native_client *native_server::accept() const
  {
    auto impl{ impl_->accept() };

    // TODO: This leaks memory but jank complains about not being able to delete
    // an opaque type if we return unique_ptr<native_client>.
    return new native_client(std::move(impl));
  }
}
