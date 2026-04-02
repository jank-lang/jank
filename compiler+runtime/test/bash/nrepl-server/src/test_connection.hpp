#include <jank/nrepl/server.hpp>

#include <iostream>
#include <thread>

struct session
{
  std::thread* thread = nullptr;
  jank::nrepl::server::native_server* server = nullptr;
  jank::nrepl::server::native_client* server_client = nullptr;
  jank::nrepl::server::native_client* test_client = nullptr;
};



session* session_make()
{
  session* ssn = new session();
  ssn->server = new jank::nrepl::server::native_server();
  std::cout << ":thread\n";
  ssn->thread = new std::thread([&ssn]() {
    std::cout << ":accepting\n";
    ssn->server_client = ssn->server->accept();
  });
  ssn->test_client = ssn->server->connect_test_client();
  return ssn;
}


