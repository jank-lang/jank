#include <jank/nrepl/server.hpp>

#include <atomic>
#include <chrono>
#include <thread>

struct session_fixture
{
  /* Thread blocking in accept() to establish the SUT connection. */
  std::thread* thread = nullptr;

  /* Server that accepts the SUT connection and creates the driver connection. */
  jank::nrepl::server::native_server* server = nullptr;

  /* System under test connection returned by server->accept(). */
  jank::nrepl::server::native_client* sut = nullptr;

  /* Test driver connection initiated via server. */
  jank::nrepl::server::native_client* driver = nullptr;

  /* Stops session activity and closes active connections. */
  void teardown()
  {
    if(thread && thread->joinable())
    {
      thread->join();
    }
    if(driver)
    {
      driver->close();
    }
    if(sut)
    {
      sut->close();
    }
  }

  /* Creates a connected test fixture with both SUT and driver endpoints
   * ready for interaction.
   */
  static session_fixture* make()
  {
    auto session = new session_fixture();
    session->server = new jank::nrepl::server::native_server();

    std::atomic<bool> ready{false};
    session->thread = new std::thread([&ready, session]() {
      session->sut = session->server->accept();
      ready.store(true, std::memory_order_release);
    });


    session->driver = session->server->_create_test_connection();
    while (!ready.load(std::memory_order_acquire)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return session;
  }
};


