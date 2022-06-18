#include "cppiperconfig.hh"
#include "receiver.hh"
#include "sender.hh"
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <glog/logging.h>
#include <iostream>
#include <mutex>
#include <pipemanager.hh>
#include <ratio>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <tuple>

using namespace std::string_literals;

double sender_net(0);
double receiver_net(0);

void sender(const int msg_size, const int msg_count,
            const std::filesystem::path pipepath,
            std::condition_variable &conditional, std::mutex &lock,
            bool &start) {
  const std::string msg = cppiper::random_hex(msg_size);
  cppiper::Sender server_sender("Server", pipepath);
  std::unique_lock lk(lock);
  conditional.wait(lk, [&]() { return start; });
  std::chrono::high_resolution_clock::time_point start_time =
      std::chrono::high_resolution_clock::now();
  for (int i = 0; i < msg_count; i++) {
    server_sender.send(msg);
  }
  std::chrono::high_resolution_clock::time_point end_time =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> delta = (end_time - start_time);
  sender_net += delta.count();
  server_sender.terminate();
};

void receiver(const int msg_count, const std::filesystem::path pipepath,
              std::condition_variable &conditional, std::mutex &lock,
              bool &start) {
  cppiper::Receiver server_receiver("Client", pipepath);
  std::unique_lock lk(lock);
  conditional.wait(lk, [&]() { return start; });
  std::chrono::high_resolution_clock::time_point start_time =
      std::chrono::high_resolution_clock::now();
  for (int i = 0; i < msg_count; i++) {
    server_receiver.receive(true);
  }
  std::chrono::high_resolution_clock::time_point end_time =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> delta = (end_time - start_time);
  receiver_net += delta.count();
  server_receiver.wait();
};

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "usage: benchmark <msg_count> <msg_size>" << std::endl;
    exit(1);
  }
  const int msg_count(atoi(argv[1]));
  const int msg_size(atoi(argv[2]));
  std::cout << "cppiper v" << CPPIPER_VERSION_MAJOR << '.'
            << CPPIPER_VERSION_MINOR << " benchmark" << std::endl
            << "Message Count: " << msg_count << std::endl
            << "Message Size:  " << msg_size << "B" << std::endl;

  fLS::FLAGS_log_dir = "./";
  google::InitGoogleLogging(argv[0]);
  cppiper::PipeManager pm("pipemanager");
  std::filesystem::path pipepath = pm.make_pipe();
  bool start(false);
  std::condition_variable conditional;
  std::mutex sender_lock{};
  std::mutex receiver_lock{};
  std::thread sender_thread(sender, msg_size, msg_count, pipepath,
                            std::ref(conditional), std::ref(sender_lock),
                            std::ref(start));
  std::thread receiver_thread(receiver, msg_count, pipepath,
                              std::ref(conditional),
                              std::ref(receiver_lock), std::ref(start));
  sleep(1);
  start = true;
  conditional.notify_all();
  std::cout << "running..." << std::endl;
  sender_thread.join();
  receiver_thread.join();
  std::cout << "finished" << std::endl;
  std::cout << "Result: " << (sender_net + receiver_net) / msg_count << "us/msg"
            << std::endl;
  pm.remove_pipe(pipepath.filename());
  return 0;
}
