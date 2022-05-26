#include "sender.hh"
#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const void cppiper::Sender::sender(std::string pipe_path, int &msg_size,
                                   char *&buffer, bool &success, char *&err_msg,
                                   bool &running, std::mutex &lock,
                                   std::barrier<> &barrier) {
  lock.lock();
  int ret_code;
  if (not std::filesystem::exists(pipe_path)) {
    ret_code = mkfifo(pipe_path.c_str(), 00666);
    if (ret_code == -1) {
      success = false;
      if (err_msg != nullptr)
        delete[] err_msg;
      err_msg = (char *)"failed to create sender pipe";
      running = false;
      lock.unlock();
      return;
    }
  }
  int pipe_fd = open(pipe_path.c_str(), O_RDWR | O_APPEND);
  if (pipe_fd == -1) {
    success = false;
    if (err_msg != nullptr)
      delete[] err_msg;
    err_msg = (char *)"failed to open sender pipe";
    running = false;
    lock.unlock();
    return;
  }
  std::stringstream ss;
  lock.unlock();
  while (true) {
    std::cout << "Waiting" << std::endl;
    barrier.arrive_and_wait();
    lock.lock();
    success = true;
    std::cout << "Done waiting" << std::endl;
    if (not running) {
      std::cout << "Not running so break" << std::endl;
      break;
    }
    if (msg_size < 0) {
      success = false;
      if (err_msg != nullptr)
        delete[] err_msg;
      err_msg = (char *)"message size is < 1";
      lock.unlock();
      barrier.arrive_and_wait();
      continue;
    }
    std::cout << "Message size: " << msg_size << std::endl;
    if (buffer == nullptr) {
      success = false;
      if (err_msg != nullptr)
        delete[] err_msg;
      err_msg = (char *)"attempt to send empty message";
      lock.unlock();
      barrier.arrive_and_wait();
      continue;
    }
    std::cout << "Buffer: " << buffer << std::endl;
    ss << std::setfill('0') << std::setw(8) << std::hex << msg_size;
    ss << buffer;
    std::cout << "Message: " << ss.rdbuf() << std::endl;
    ret_code = write(pipe_fd, ss.str().c_str(), msg_size + 8);
    if (ret_code == -1) {
      success = false;
      if (err_msg != nullptr)
        delete[] err_msg;
      err_msg = (char *)"attempt to send empty message";
      lock.unlock();
      barrier.arrive_and_wait();
      continue;
    }
    std::cout << "Success: " << success << std::endl;
    lock.unlock();
    barrier.arrive_and_wait();
    ss.flush();
  }
  std::cout << "Ending thread" << std::endl;
  ret_code = close(pipe_fd);
  if (ret_code == -1) {
    success = false;
    if (err_msg != nullptr)
      delete[] err_msg;
    err_msg = (char *)"failed to close sender pipe";
  }
  lock.unlock();
}

cppiper::Sender::~Sender(void) {
  terminate();
  if (err_msg != nullptr)
    delete[] err_msg;
  if (buffer != nullptr)
    delete[] buffer;
}

cppiper::Sender::Sender(std::string pipe_path)
    : msg_size(0), pipe_path(pipe_path), buffer(nullptr), err_msg(nullptr),
      success(true), running(true), lock(), barrier(2),
      thread(sender, std::ref(pipe_path), std::ref(msg_size), std::ref(buffer),
             std::ref(success), std::ref(err_msg), std::ref(running),
             std::ref(lock), std::ref(barrier)) {}

const std::string cppiper::Sender::get_err_msg(void) {
  return err_msg == nullptr ? "" : err_msg;
}

const bool cppiper::Sender::send(char *msg, int size) {
  std::cout << "sending" << std::endl;
  if (not running)
    return false;
  lock.lock();
  if (buffer != nullptr)
    delete[] buffer;
  msg_size = size;
  buffer = new char[msg_size + 1];
  for (auto i = 0; i < msg_size; i++)
    buffer[i] = msg[i];
  buffer[msg_size] = '\0';
  lock.unlock();
  barrier.arrive_and_wait();
  barrier.arrive_and_wait();
  lock.lock();
  msg_size = 0;
  lock.unlock();
  return success;
}

const bool cppiper::Sender::terminate(void) {
  if (not thread.joinable())
    return false;
  lock.lock();
  running = false;
  lock.unlock();
  barrier.arrive_and_wait();
  thread.join();
  return true;
}
