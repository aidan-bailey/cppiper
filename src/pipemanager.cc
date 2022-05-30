#include "pipemanager.hh"
#include <filesystem>
#include <mutex>
#include <spdlog/spdlog.h>
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
    spdlog::debug("Creating pipe directory '{}'...", pipedir);
    std::filesystem::create_directories(pipedir);
  }
  spdlog::info("Constructed pipe manager for pipe directory '{}'", pipedir);
}

std::string cppiper::PipeManager::make_pipe(void) {
  std::string pipepath;
  std::lock_guard lk(lock);
  while (std::filesystem::exists(pipepath = pipedir + "/" + random_hex(32))) {
    spdlog::debug("Pipe miss at '{}'", pipepath);
  };
  mkfifo(pipepath.c_str(), 00666);
  spdlog::debug("New pipe created at '{}'", pipepath);
  return pipepath;
}

bool cppiper::PipeManager::remove_pipe(std::string pipepath) {
    std::lock_guard lk(lock);
    if (not std::filesystem::exists(pipepath)){
        spdlog::error("Failed to remove pipe at '{}' as it does not exist", pipepath);
        return false;
    }
    std::filesystem::remove(pipepath);
    spdlog::debug("Removed pipe at '{}'", pipepath);
    return true;
};

cppiper::PipeManager::~PipeManager(void) {
  spdlog::debug("Destructing pipe manager...");
  if (not std::filesystem::exists(pipedir))
    spdlog::warn("Pipe directory at '{}' non-existent at deconstruction",
                 pipedir);
  else {
    for (const auto &entry : std::filesystem::directory_iterator(pipedir)) {
      std::filesystem::remove(entry.path());
      spdlog::debug("Removed pipe at '{}'", entry.path().string());
    }
  }
}
