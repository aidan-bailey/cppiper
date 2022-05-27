#include "sender.hh"
#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

const void cppiper::Sender::sender(std::string &pipepath,
                                   std::vector<char> &buffer, int &statuscode,
                                   bool &msg_ready, bool &stop,
                                   std::mutex &lock,
                                   std::condition_variable &msg_conditional,
                                   std::barrier<> &processed_barrier) {
  std::unique_lock lk(lock);
  std::cout << "LOCKING" << std::endl;
  int retcode;
  if (not std::filesystem::exists(pipepath)) {
    retcode = mkfifo(pipepath.c_str(), 00666);
    if (retcode == -1) {
      std::cerr << "Failed to create sender pipe." << std::endl;
      statuscode = 1;
      lk.unlock();
      return;
    }
  }
  std::cout << "STARTING TO READ" << std::endl;
  std::ofstream pipe;
  try {
    pipe.open(pipepath, std::ofstream::out | std::ofstream::ate | std::ofstream::binary);
  } catch (std::ofstream::failure e) {
    std::cerr << "Failed to open sender pipe." << std::endl;
    statuscode = 2;
    lk.unlock();
    return;
  }
  std::cout << "READING" << std::endl;
  std::stringstream ss;
  std::cout << "START LOOP" << std::endl;
  while (true) {
    if (not (msg_ready or stop)) {
      msg_conditional.wait(lk, [&]() { return msg_ready or stop; });
    }
    statuscode = 0;
    if (stop) {
      break;
    }
    if (buffer.empty()) {
      std::cerr << "Attempt to send empty message." << std::endl;
      statuscode = 3;
      msg_ready = false;
      lk.unlock();
      msg_conditional.notify_one();
      continue;
    }
    ss << std::setfill('0') << std::setw(8) << std::hex << buffer.size();
    try {
      pipe << ss.rdbuf();
    } catch (std::ofstream::failure e) {
      std::cerr << "Failed to send message size." << std::endl;
      statuscode = 4;
      ss.flush();
      msg_ready = false;
      lk.unlock();
      msg_conditional.notify_one();
      continue;
    }
    ss.flush();
    try {
      std::for_each(buffer.begin(), buffer.end(),
                    [&pipe](const char &s) { pipe << s; });
    } catch (std::ofstream::failure e) {
      std::cerr << "Failed to send message." << std::endl;
      statuscode = 5;
      msg_ready = false;
      lk.unlock();
      msg_conditional.notify_one();
      continue;
    }
  }
  pipe.flush();
  pipe.close();
  lk.unlock();
}

cppiper::Sender::Sender(std::string pipepath)
    : pipepath(pipepath), buffer(), statuscode(0), msg_ready(false),
      stop(false), lock(), msg_conditional(), processed_barrier(2),
      thread(sender, std::ref(pipepath), std::ref(buffer), std::ref(statuscode),
             std::ref(msg_ready), std::ref(stop), std::ref(lock),
             std::ref(msg_conditional), std::ref(processed_barrier)) {}

cppiper::Sender::~Sender(void) { terminate(); }

const int cppiper::Sender::get_status_code(void) { return statuscode; };

const bool cppiper::Sender::send(const std::vector<char> &msg) {
  if (not thread.joinable() or stop) {
    std::cerr << "Attempt to send message with terminated sender." << std::endl;
    return false;
  }
  {
    std::lock_guard lk(lock);
    msg_ready = true;
  }
  msg_conditional.notify_one();
  processed_barrier.arrive_and_wait();
  return statuscode == 0;
}

const bool cppiper::Sender::terminate(void) {
  if (not thread.joinable() or stop) {
    std::cerr << "Double termination for sender." << std::endl;
    return true;
  }
  {
    std::cout << "Waiting" << std::endl;
    std::lock_guard lk(lock);
    std::cout << "Locked" << std::endl;
    stop = true;
  }
  msg_conditional.notify_one();
  thread.join();
  return true;
}
