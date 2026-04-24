#include <jank/nrepl/server.hpp>

#include <atomic>
#include <chrono>
#include <thread>

/* A test harness that creates a `jank::nrepl::server::native_server`
 * and establishes paired driver and System Under Test (SUT) endpoints
 * for controlled interaction, and is torn down after use.
 */
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

  /* Stops harness activity and releases resources. */
  void teardown()
  {
    if(driver) driver->close();
    if(sut)    sut->close();

    if(thread)
    {
      if(thread->joinable())
      {
        thread->join();
      }
      delete thread;
      thread = nullptr;
    }
    if(driver)
    {
      delete driver;
      driver = nullptr;
    }
    if(sut)
    {
      delete sut;
      sut = nullptr;
    }
    if(server)
    {
      delete server;
      server = nullptr;
    }
  }

  /* Creates a connected environment with both SUT and driver
   * endpoints ready for interaction.
   */
  static connection_harness* make()
  {
    auto harness = new connection_harness();
    harness->server = new jank::nrepl::server::native_server();

   /* std::promise would be cleaner, but not usable yet due to JIT
    * emutls issue:
    * https://github.com/jank-lang/jank/discussions/729 */
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
