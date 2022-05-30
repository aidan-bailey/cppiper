#include "cppiperconfig.hh"
#include "receiver.hh"
#include "sender.hh"
#include <chrono>
#include <iostream>
#include <ratio>
#include <spdlog/common.h>
#include <string>
#include <thread>
#include <tuple>

int main(int argc, char *argv[]) {
  std::cout << "cppiper v" << CPPIPER_VERSION_MAJOR << '.'
            << CPPIPER_VERSION_MINOR << std::endl;
  std::cout << "Dev Mode: " << DEV << std::endl;
  if (DEV)
    spdlog::set_level(spdlog::level::debug);
  else
    spdlog::set_level(spdlog::level::err);
  cppiper::Sender server_sender("Server", "server");
  cppiper::Receiver server_receiver("Client", "server");
  std::vector<char> msg(5, 'A');
  int testset_size = 20;
  for (char i : msg)
    std::cout << i;
  std::cout << std::endl;
  std::chrono::high_resolution_clock::time_point start =
      std::chrono::high_resolution_clock::now();
  for (int i = 0; i < testset_size; i++) {
    server_sender.send(msg);
    server_receiver.receive(true);
  }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> delta = (end - start);
  std::cout << "Avg of " << delta.count() / testset_size << "us/msg"
            << std::endl;
  server_sender.terminate();
  return 0;
}
