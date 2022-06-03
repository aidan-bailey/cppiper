#include "../include/sender.hh"
#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

const void cppiper::Sender::sender(const std::string pipepath,
                                   std::string &buffer, int &statuscode,
                                   bool &msg_ready, const bool &stop,
                                   std::mutex &lock,
                                   std::condition_variable &msg_conditional) {
  std::unique_lock lk(lock);
  spdlog::debug("Initialising sender thread for '{}' pipe", pipepath);
  int retcode;
  if (not std::filesystem::exists(pipepath)) {
    spdlog::debug("Pipe '{}' does not exist, creating...", pipepath);
    retcode = mkfifo(pipepath.c_str(), 00666);
    if (retcode == -1) {
      spdlog::error("Failed to create sender pipe '{}'", pipepath);
      statuscode = 1;
      lk.unlock();
      return;
    }
  } else if (not std::filesystem::is_fifo(pipepath)) {
    spdlog::error("File at provided sender path path '{}' is not a fifo",
                  pipepath);
    statuscode = 1;
    lk.unlock();
    return;
  }
  spdlog::debug("Opening sender end of pipe '{}'...", pipepath);
  const int pipe_fd = open(pipepath.c_str(), O_WRONLY | O_APPEND);
  if (pipe_fd == -1) {
    spdlog::error("Failed to open sender pipe '{}'", pipepath);
    statuscode = 2;
    lk.unlock();
    return;
  }
  std::stringstream ss;
  spdlog::debug("Entering sender loop for pipe '{}'...", pipepath);
  while (true) {
    if (not(msg_ready or stop)) {
      spdlog::debug("Waiting on sender request for pipe '{}'...", pipepath);
      msg_conditional.wait(lk, [&]() { return msg_ready or stop; });
    }
    statuscode = 0;
    if (stop) {
      spdlog::debug("Breaking from sender loop for pipe '{}'...", pipepath);
      break;
    }
    spdlog::debug("Send request received for pipe '{}'", pipepath);
    if (buffer.empty()) {
      spdlog::error("Attempt to send empty message over pipe '{}'", pipepath);
      statuscode = 3;
      msg_ready = false;
      lk.unlock();
      continue;
    }
    ss << std::setfill('0') << std::setw(8) << std::hex << buffer.size();
    retcode = write(pipe_fd, ss.str().c_str(), 8);
    ss.flush();
    if (retcode == -1) {
      spdlog::error("Failed to send message size bytes over pipe '{}'",
                    pipepath);
      statuscode = 4;
      msg_ready = false;
      lk.unlock();
      continue;
    }
    spdlog::debug("Sent message size bytes over pipe '{}'", pipepath);
    retcode = write(pipe_fd, &buffer.front(), buffer.size());
    if (retcode == -1) {
      spdlog::error("Failed to send message bytes over pipe '{}'", pipepath);
      statuscode = 5;
      msg_ready = false;
      lk.unlock();
      continue;
    }
    spdlog::debug("Sent message bytes over pipe '{}'", pipepath);
    msg_ready = false;
  }
  retcode = close(pipe_fd);
  if (retcode == -1) {
    spdlog::error("Failed to close sender end for pipe '{}'", pipepath);
    statuscode = 6;
  } else
    spdlog::debug("Closed sender end for pipe '{}'", pipepath);
  lk.unlock();
}

cppiper::Sender::Sender(const std::string name, const std::string pipepath)
    : name(name), pipepath(pipepath), buffer(), statuscode(0), msg_ready(false),
      stop(false), lock(), msg_conditional(),
      thread(sender, pipepath, std::ref(buffer), std::ref(statuscode),
             std::ref(msg_ready), std::ref(stop), std::ref(lock),
             std::ref(msg_conditional)) {
  spdlog::info("Constructed sender instance '{0}' with pipe '{1}'", name,
               pipepath);
}

const int cppiper::Sender::get_status_code(void) const { return statuscode; };

const bool cppiper::Sender::send(const std::string msg) {
  spdlog::info("Sending message on sender instance '{}'", name);
  if (not thread.joinable() or stop) {
    spdlog::error(
        "Attempt to send message on non-running sender instance for '{}'",
        name);
    return false;
  }
  {
    std::lock_guard lk(lock);
    buffer = msg;
    msg_ready = true;
  }
  msg_conditional.notify_one();
  std::lock_guard lk(lock);
  if (statuscode == 0)
    spdlog::info("Message sent on sender instance '{}'", name);
  else
    spdlog::error("Message failed to send on sender instance '{}'", name);
  return statuscode == 0;
}

const bool cppiper::Sender::terminate(void) {
  spdlog::debug("Terminating sender instance '{}'...", name);
  if (not thread.joinable() or stop) {
    spdlog::warn("Double termination for sender instance '{}'", name);
    return true;
  }
  {
    std::lock_guard lk(lock);
    stop = true;
  }
  msg_conditional.notify_one();
  spdlog::debug("Joining thread for sender instance '{}'...", name);
  thread.join();
  spdlog::info("Terminated sender instance '{}'", name);
  return true;
}
