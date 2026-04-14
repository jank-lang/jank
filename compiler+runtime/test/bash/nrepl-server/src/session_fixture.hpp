#include <jank/nrepl/server.hpp>

#include <future>
#include <iostream>
#include <thread>

struct session_fixture
{
  /* Background thread running the server accept loop */
  std::thread* thread = nullptr;

  jank::nrepl::server::native_server* server = nullptr;
  /* The server-side client of the connection */
  jank::nrepl::server::native_client* server_client = nullptr;
  /* A test client connected to the server */
  jank::nrepl::server::native_client* test_client = nullptr;

  void cleanup()
  {
    if (thread && thread->joinable())
       thread->join();
    if (server_client) server_client->close();
    if (test_client) test_client->close();
    if (server) delete server;
  }

  static session_fixture* make()
  {
    session_fixture* session = new session_fixture();
    session->server = new jank::nrepl::server::native_server();

    std::promise<void> ready;
    std::future<void> fut = ready.get_future();

    session->thread = new std::thread([&ready, session]() {
      session->server_client = session->server->accept();
      ready.set_value(); 
    });

    session->test_client = session->server->connect_test_client();
    fut.wait();

    return session;
  }
};


