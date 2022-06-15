#include "../include/receiver.hh"
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

void cppiper::Receiver::receiver(const std::string pipepath, bool &msg_ready,
                                 int &statuscode,
                                 std::queue<std::string> &msg_queue,
                                 std::mutex &queue_lock,
                                 std::condition_variable &queue_condition) {
  DLOG(INFO) << "Initialising receiver thread for " << pipepath << " pipe"
             << std::endl;
  int retcode;
  DLOG(INFO) << "Opening receiver end of pipe " << pipepath << "..."
             << std::endl;
  const int pipe_fd = open(pipepath.c_str(), O_RDONLY);
  if (pipe_fd == -1) {
    LOG(ERROR) << "Failed to open receiver pipe " << pipepath << std::endl;
    statuscode = 1;
    queue_lock.unlock();
    return;
  }
  char hexbuffer[8];
  int bytes_read;
  const int buffering_limit(65536);
  DLOG(INFO) << "Entering receiver loop for pipe " << pipepath << "..."
             << std::endl;
  while (true) {
    DLOG(INFO) << "Reading message size bytes from pipe " << pipepath << "..."
               << std::endl;
    bytes_read = read(pipe_fd, hexbuffer, 8);
    if (bytes_read == -1) {
      LOG(ERROR) << "Failed to read size bytes from pipe " << pipepath
                 << std::endl;
      statuscode = errno;
      continue;
    } else if (bytes_read == 0) {
      DLOG(INFO) << "Breaking from receiver loop for pipe " << pipepath
                 << std::endl;
      break;
    } else if (bytes_read != 8) {
      LOG(ERROR) << "Read an unexpected number of message size bytes from pipe "
                 << pipepath << std::endl;
      statuscode = errno;
      break;
    }
    queue_lock.lock();
    statuscode = 0;
    std::stringstream ss;
    ss << std::hex << hexbuffer;
    int msg_size;
    ss >> msg_size;
    if (msg_size < 1) {
      LOG(ERROR) << "Parsed message size less than 1 (" << msg_size
                 << ") from pipe " << pipepath << std::endl;
      statuscode = errno;
      continue;
    }
    std::vector<char> subbuffer(msg_size);
    DLOG(INFO) << "Reading message bytes from pipe " << pipepath << "..."
               << std::endl;
    int total_bytes_read(0);
    while ((bytes_read = read(
                pipe_fd, &subbuffer.front() + total_bytes_read,
                std::min(msg_size - total_bytes_read, buffering_limit))) > 0 and
           (msg_size - (total_bytes_read += bytes_read) > 0))
      ;
    if (bytes_read == -1) {
      LOG(ERROR) << "Failed to read message bytes from pipe " << pipepath
                 << std::endl;
      statuscode = errno;
      continue;
    } else if (bytes_read == 0) {
      DLOG(INFO) << "Breaking from receiver loop for pipe " << pipepath
                 << std::endl;
      break;
    }
    msg_queue.emplace(std::string(&subbuffer.front(), msg_size));
    msg_ready = true;
    queue_lock.unlock();
    queue_condition.notify_one();
  }
  retcode = close(pipe_fd);
  if (retcode == -1) {
    LOG(ERROR) << "Failed to close receiver end for pipe " << pipepath
               << std::endl;
    statuscode = errno;
  } else {
    DLOG(INFO) << "Closed receiver end for pipe " << pipepath << std::endl;
  }
  queue_lock.unlock();
  queue_condition.notify_one();
}

cppiper::Receiver::Receiver(const std::string name, const std::string pipepath)
    : name(name), pipepath(pipepath), msg_ready(false), statuscode(0),
      msg_queue(), queue_lock(), queue_condition(),
      thread(receiver, pipepath, std::ref(msg_ready), std::ref(statuscode),
             std::ref(msg_queue), std::ref(queue_lock),
             std::ref(queue_condition)) {
  LOG(INFO) << "Constructed receiver instance " << name << " for pipe "
            << pipepath << std::endl;
}

std::optional<const std::string> cppiper::Receiver::receive(bool wait) {
  std::unique_lock lk(queue_lock);
  DLOG(INFO) << "Trying to receive message on receiver instance " << name << std::endl;
  if (not msg_ready and wait) {
    DLOG(INFO) << "Waiting to receive message on receiver instance " << name << std::endl;
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
  DLOG(INFO) << "Returning received message from receiver instance " << name << std::endl;
  return msg;
}

bool cppiper::Receiver::wait(void) {
  if (not thread.joinable()) {
    return true;
  }
  thread.join();
  return true;
}

std::string cppiper::Receiver::get_pipe(void) const { return pipepath; }
