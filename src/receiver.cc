#include "../include/receiver.hh"
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <mutex>
#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <vector>

const void cppiper::Receiver::receiver(
    const std::string pipepath, bool &msg_ready, int &statuscode,
    std::queue<std::string> &msg_queue, std::mutex &queue_lock,
    std::condition_variable &queue_condition) {
  // std::lock_guard lk(queue_lock);
  spdlog::debug("Initialising receiver thread for '{}' pipe", pipepath);
  int retcode;
  spdlog::debug("Opening receiver end of pipe '{}'...", pipepath);
  const int pipe_fd = open(pipepath.c_str(), O_RDONLY);
  if (pipe_fd == -1) {
    spdlog::error("Failed to open receiver pipe '{}'", pipepath);
    statuscode = 1;
    queue_lock.unlock();
    return;
  }
  char hexbuffer[8]{};
  std::stringstream ss;
  int i;
  int bytes_read;
  spdlog::debug("Entering receiver loop for pipe '{}'...", pipepath);
  while (true) {
    std::vector<char> buffer;
    spdlog::debug("Reading message size bytes from pipe '{}'...", pipepath);
    bytes_read = read(pipe_fd, hexbuffer, 8);
    if (bytes_read == -1) {
      spdlog::error("Failed to read size bytes from pipe '{}'", pipepath);
      statuscode = 1;
      continue;
    } else if (bytes_read == 0) {
      spdlog::debug("Breaking from receiver loop for pipe '{}'...", pipepath);
      break;
    } else if (bytes_read != 8) {
      spdlog::error(
          "Read an unexpected number of message size bytes from pipe '{}'",
          pipepath);
      statuscode = 2;
      break;
    }
    queue_lock.lock();
    statuscode = 0;
    int msg_size;
    ss << std::hex << hexbuffer;
    ss >> msg_size;
    ss.flush();
    if (msg_size < 1) {
      statuscode = 3;
      spdlog::error("Parsed message size less than 1 ({0}) from pipe '{1}'",
                    msg_size, pipepath);
      continue;
    }
    char subbuffer[msg_size];
    spdlog::debug("Reading message bytes from pipe '{}'...", pipepath);
    bytes_read = read(pipe_fd, subbuffer, msg_size);
    if (bytes_read == -1) {
      spdlog::error("Failed to read message bytes from pipe '{}'", pipepath);
      statuscode = 4;
      continue;
    } else if (bytes_read != msg_size) {
      spdlog::error("Message bytes read from pipe '{}' did not match expected "
                    "message size",
                    pipepath);
      statuscode = 5;
      continue;
    }

    for (auto ptr = subbuffer; ptr < subbuffer + 8; ptr++)
      buffer.push_back(*ptr);
    spdlog::debug("Received message over pipe '{}'", pipepath);
    msg_queue.push(std::string(subbuffer, msg_size));
    msg_ready = true;
    queue_lock.unlock();
    queue_condition.notify_one();
  }
  retcode = close(pipe_fd);
  if (retcode == -1) {
    spdlog::error("Failed to close receiver end for pipe '{}'", pipepath);
    statuscode = 6;
  } else
    spdlog::debug("Closed receiver end for pipe '{}'", pipepath);
  queue_lock.unlock();
  queue_condition.notify_one();
}

cppiper::Receiver::Receiver(const std::string name, const std::string pipepath)
    : name(name), pipepath(pipepath), msg_ready(false), statuscode(0),
      msg_queue(), queue_lock(), queue_condition(),
      thread(receiver, pipepath, std::ref(msg_ready), std::ref(statuscode),
             std::ref(msg_queue), std::ref(queue_lock),
             std::ref(queue_condition)) {
  spdlog::info("Constructed receiver instance '{0}' with pipe '{1}'", name,
               pipepath);
}

cppiper::Receiver::~Receiver(void) {
  wait();
  std::unique_lock lk(queue_lock);
  while (not msg_queue.empty()) {
    msg_queue.pop();
  }
}

const std::optional<const std::string> cppiper::Receiver::receive(bool wait) {
  std::unique_lock lk(queue_lock);
  spdlog::info("Trying to receive message on receiver instance '{}'", name);
  if (not msg_ready and wait) {
    spdlog::debug("Waiting to receive message on receiver instance '{}'", name);
    queue_condition.wait(lk, [this] { return msg_ready; });
  }
  if (msg_queue.empty()) {
    lk.unlock();
    return {};
  }
  std::string msg = msg_queue.front();
  msg_queue.pop();
  if (msg_queue.empty())
    msg_ready = false;
  lk.unlock();
  spdlog::info("Returning received message from receiver instance '{}'", name);
  return msg;
}

const bool cppiper::Receiver::wait(void) {
  if (not thread.joinable()) {
    std::cerr << "Double termination for sender." << std::endl;
    return false;
  }
  thread.join();
  return true;
}
