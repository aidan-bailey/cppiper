#include "cppiperconfig.hh"
#include "receiver.hh"
#include "sender.hh"
#include <chrono>
#include <iostream>
#include <pipemanager.hh>
#include <ratio>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <tuple>

using namespace std::string_literals;

int main(int argc, char *argv[]) {
  std::cout << "cppiper v" << CPPIPER_VERSION_MAJOR << '.'
            << CPPIPER_VERSION_MINOR << std::endl;
  if (DEV) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Dev mode enabled");
  } else
    spdlog::set_level(spdlog::level::err);
  cppiper::PipeManager pm("pipemanager");
  std::string pipe_name = pm.make_pipe();
  cppiper::Sender server_sender("Server", pipe_name);
  cppiper::Receiver server_receiver("Client", pipe_name);
  std::string msg = "AAA\0AA"s;
  int testset_size = 100000;
  std::cout << "Message: ";
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
  pm.remove_pipe(pipe_name);
  return 0;
}
