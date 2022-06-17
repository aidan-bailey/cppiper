#include "../include/pipemanager.hh"
#include "cppiperconfig.hh"
#include <filesystem>
#include <glog/logging.h>
#include <mutex>
#include <string>
#include <sys/stat.h>

std::string cppiper::random_hex(int len) {
  const static char hexChar[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                 '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  std::stringstream ss;
  for (int j = 0; j < len; j++)
    ss << hexChar[rand() % 16];
  return ss.str();
}

cppiper::PipeManager::PipeManager(std::string pipedir)
    : lock(), pipedir(pipedir) {
  if (not std::filesystem::exists(pipedir)) {
    DLOG(INFO) << "Creating pipe directory " << pipedir << "...";
    std::filesystem::create_directories(pipedir);
  }
  DLOG(INFO) << "Constructed pipe manager for pipe directory " << pipedir;
}

std::string cppiper::PipeManager::make_pipe(void) {
  std::string pipepath;
  std::lock_guard lk(lock);
  while (std::filesystem::exists(pipepath = pipedir + "/" + random_hex(32))) {
    DLOG(INFO) << "Pipe miss at " << pipepath;
  };
  mkfifo(pipepath.c_str(), 00666);
  DLOG(INFO) << "New pipe created at " << pipepath;
  return pipepath;
}

bool cppiper::PipeManager::remove_pipe(std::string pipepath) {
  std::lock_guard lk(lock);
  if (not std::filesystem::exists(pipepath)) {
    LOG(WARNING) << "Failed to remove pipe at " << pipepath
                 << " as it does not exist";
    return false;
  }
  std::filesystem::remove(pipepath);
  DLOG(INFO) << "Removed pipe at " << pipepath;
  return true;
};

void cppiper::PipeManager::clear(void) {
  DLOG(INFO) << "Clearing pipes...";
  if (not std::filesystem::exists(pipedir)) {
    LOG(ERROR) << "Attempt to clear non-existent pipe directory " << pipedir
               << ", " << errno;
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(pipedir)) {
      std::filesystem::remove(entry.path());
      DLOG(INFO) << "Removed pipe at " << entry.path().string();
    }
  }
}
