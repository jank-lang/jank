#include <jank/nrepl/server.hpp>

#include <atomic>
#include <chrono>
#include <thread>

/* A test harness that creates a `jank::nrepl::server::native_server`
 * and establishes paired driver and System Under Test (SUT) endpoints
 * for controlled interaction, and is torn down after use.
 *
 *  NOTE: Assumes newly created objects are managed by the GC. */
struct connection_harness
{
  /* Thread blocking in accept() to establish the SUT connection. */
  std::thread* thread = nullptr;

  /* Native server used to accept the SUT connection and initiate the driver connection. */
  jank::nrepl::server::native_server* server = nullptr;

  /* SUT connection returned by server->accept(). */
  jank::nrepl::server::native_client* sut = nullptr;

  /* Test driver connection initiated via server for interaction with
     the SUT. */
  jank::nrepl::server::native_client* driver = nullptr;

  /* Stops harness activity and closes active connections. */
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

  /* Creates a connected environment with both SUT and driver
   * endpoints ready for interaction.
   */
  static connection_harness* make()
  {
    auto harness = new connection_harness();
    harness->server = new jank::nrepl::server::native_server();

    std::atomic<bool> ready{false};
    harness->thread = new std::thread([&ready, harness]() {
      harness->sut = harness->server->accept();
      ready.store(true, std::memory_order_release);
    });

    harness->driver = harness->server->_create_test_connection();
    while (!ready.load(std::memory_order_acquire)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return harness;
  }
};
