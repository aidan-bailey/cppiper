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
#include <glog/logging.h>


using namespace std::string_literals;

int main(int argc, char *argv[]) {
  std::cout << "cppiper v" << CPPIPER_VERSION_MAJOR << '.'
            << CPPIPER_VERSION_MINOR << std::endl;
  fLS::FLAGS_log_dir = "./";
  google::InitGoogleLogging(argv[0]);
  cppiper::PipeManager pm("pipemanager");
  std::string pipe_name = pm.make_pipe();
  cppiper::Sender server_sender("Server", pipe_name);
  cppiper::Receiver server_receiver("Client", pipe_name);
  std::string msg = cppiper::random_hex(128);
  int testset_size = 100000;
  double net = 0;
  for (int i = 0; i < testset_size; i++) {
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    server_sender.send(msg);
    server_receiver.receive(true);
    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> delta = (end - start);
    net += delta.count();
  }
  std::cout << "Avg of " << net / testset_size << "us/msg" << std::endl;
  server_sender.terminate();
  server_receiver.wait();
  pm.remove_pipe(pipe_name);
  return 0;
}
