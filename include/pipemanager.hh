#ifndef PIPEMANAGER_HH_
#define PIPEMANAGER_HH_
#include <mutex>
#include <sstream>
#include <string>
#include <tuple>

namespace cppiper {

std::string random_hex(int len);

class PipeManager {
private:
  std::mutex lock;
  const std::string pipedir;

public:
  PipeManager(void) = delete;
  PipeManager(std::string pipedir);
  std::string make_pipe(void);
  bool remove_pipe(std::string pipepath);
  void clear(void);
};

} // namespace cppiper

#endif // PIPEMANAGER_HH_
