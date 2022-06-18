#include "../include/receiver.hh"
#include "cppiperconfig.hh"
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <glog/logging.h>
#include <iostream>
#include <istream>
#include <mutex>
#include <sys/stat.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <vector>

void cppiper::Receiver::run() {
  running = true;
  while (true) {
    DLOG(INFO) << "Reading message size bytes from pipe " << pipepath.filename() << "...";
    char hexbuffer[8];
    int total_bytes_read(0);
    int bytes_read;
    while ((bytes_read =
                read(pipe_fd, hexbuffer + total_bytes_read,
                     std::min(8 - total_bytes_read, buffering_limit))) > 0 and
           (8 - (total_bytes_read += bytes_read) > 0)) {
      if (bytes_read < 0) {
        LOG(ERROR) << "Failed to read size bytes from pipe " << pipepath.filename() << ", "
                   << errno;
        statuscode = errno;
        continue;
      } else if (bytes_read == 0) {
        DLOG(INFO) << "Breaking from receiver loop for pipe " << pipepath.filename();
        break;
      }
    };
    statuscode = 0;
    std::stringstream ss;
    ss << std::hex << hexbuffer;
    int msg_size;
    ss >> msg_size;
    if (msg_size < 1) {
      LOG(ERROR) << "Parsed message size less than 1 (" << msg_size
                 << ") from pipe " << pipepath.filename() << ", " << errno;
      statuscode = errno;
      continue;
    }
    char subbuffer[msg_size];
    DLOG(INFO) << "Reading message bytes from pipe " << pipepath.filename() << "...";
    total_bytes_read = 0;
    while ((bytes_read = read(
                pipe_fd, subbuffer + total_bytes_read,
                std::min(msg_size - total_bytes_read, buffering_limit))) > 0 and
           (msg_size - (total_bytes_read += bytes_read) > 0))
      ;
    if (bytes_read == -1) {
      LOG(ERROR) << "Failed to read message bytes from pipe " << pipepath.filename()
                 << ", " << errno;
      statuscode = errno;
      continue;
    } else if (bytes_read == 0) {
      DLOG(INFO) << "Breaking from receiver loop for pipe " << pipepath.filename();
      break;
    }
    std::lock_guard lk(queue_lock);
    msg_queue.emplace(std::string(subbuffer, msg_size));
    msg_ready = true;
    queue_condition.notify_one();
  }
  running = false;
  queue_condition.notify_one();
}

cppiper::Receiver::Receiver(const std::string name,
                            const std::filesystem::path pipepath)
    : name(name), pipepath(pipepath), buffering_limit(65536), running(false), msg_ready(false),
      statuscode(0), pipe_fd(-1), msg_queue{}, queue_lock{}, queue_condition{} {
  DLOG(INFO) << "Initialising receiver thread for pipe " << pipepath.filename();
  DLOG(INFO) << "Opening receiver end of pipe " << pipepath.filename() << "...";
  pipe_fd = open(pipepath.c_str(), O_RDONLY);
  if (pipe_fd == -1) {
    LOG(ERROR) << "Failed to open receiver pipe " << pipepath << ", " << errno;
    statuscode = errno;
    return;
  }
  thread = std::thread(&Receiver::run, this);
  LOG(INFO) << "Constructed receiver instance " << name << " with pipe "
            << this->pipepath.filename();
}

std::optional<const std::string> cppiper::Receiver::receive(bool wait) {
  std::unique_lock lk(queue_lock);
  LOG(INFO) << "Trying to receive message on receiver instance " << name;
  if (running and not msg_ready and wait) {
    DLOG(INFO) << "Waiting to receive message on receiver instance " << name;
    queue_condition.wait(lk, [this] { return msg_ready; });
  }
  if (msg_queue.empty()) {
    lk.unlock();
    return {};
  }
  const std::string msg = msg_queue.front();
  msg_queue.pop();
  if (msg_queue.empty())
    msg_ready = false;
  lk.unlock();
  LOG(INFO) << "Returning received message from receiver instance " << name;
  return msg;
}

bool cppiper::Receiver::wait(void) {
  if (not thread.joinable()) {
    return true;
  }
  DLOG(INFO) << "Joining thread for receiver instance " << name << "...";
  thread.join();
  DLOG(INFO) << "Joined thread for receiver instance " << name;
  if (close(pipe_fd) == -1) {
    LOG(ERROR) << "Failed to close receiver end of pipe " << pipepath.filename() << ", "
               << errno;
    statuscode = errno;
  } else {
    DLOG(INFO) << "Closed receiver end of pipe " << pipepath.filename();
  }
  LOG(INFO) << "Cleaned up receiver instance " << name;
  return true;
}

std::filesystem::path cppiper::Receiver::get_pipe(void) const {
  return pipepath;
}
