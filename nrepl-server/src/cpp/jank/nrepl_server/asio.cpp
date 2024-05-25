#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

namespace jank::nrepl_server::asio
{
  using namespace jank::runtime;
  using boost::asio::ip::tcp;

  struct connection : public std::enable_shared_from_this<connection>
  {
    connection(tcp::socket &&socket)
      : socket_(std::move(socket))
    {
      do_read();
    }

    void do_read()
    {
      auto self(shared_from_this());
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
                              [this, self](boost::system::error_code ec, std::size_t length) {
                                if(!ec)
                                {
                                  do_write(length);
                                }
                              });
    }

    void do_write(std::size_t length)
    {
      auto self(shared_from_this());
      boost::asio::async_write(socket_,
                               boost::asio::buffer(data_, length),
                               [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                 if(!ec)
                                 {
                                   do_read();
                                 }
                               });
    }

    static constexpr size_t max_length = 1024;

    tcp::socket socket_;
    char data_[max_length]{};
  };

  boost::asio::io_context io_context;
  std::unique_ptr<tcp::acceptor> acceptor;
  std::unique_ptr<tcp::socket> socket;

  void accept_connection()
  {
    std::cout << "waiting for a new connection...\n";
    acceptor->async_accept(*socket, [](boost::system::error_code const ec) {
      if(!ec)
      {
        std::cout << "new connection accepted\n";
        std::make_shared<connection>(std::move(*socket));
      }

      accept_connection();
    });
  }

  object_ptr run_server(object_ptr const port, object_ptr const callback)
  {
    auto const p(to_int(port));
    acceptor = std::make_unique<tcp::acceptor>(io_context, tcp::endpoint(tcp::v4(), p));
    socket = std::make_unique<tcp::socket>(io_context);

    accept_connection();

    /* This blocks. */
    io_context.run();
    return obj::nil::nil_const();
  }

  /* TODO: Make base class for ns which does this_object_ptr */
  struct __ns : behavior::callable
  {
    /* TODO: Remove const from all of these. */
    object_ptr call() const override
    {
      auto const ns(__rt_ctx->intern_ns("jank.nrepl-server.asio"));
      ns->intern_var("run!")->bind_root(make_box<obj::native_function_wrapper>(&run_server));
      return obj::nil::nil_const();
    }

    object_ptr this_object_ptr() const override
    {
      return obj::nil::nil_const();
    }
  };
}
