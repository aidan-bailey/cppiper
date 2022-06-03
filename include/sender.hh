#ifndef SENDER_HH_
#define SENDER_HH_
#include <condition_variable>
#include <memory>
#include <mutex>
#include <spdlog/logger.h>
#include <string>
#include <thread>
#include <vector>
#include <spdlog/spdlog.h>

namespace cppiper {

class Sender {
private:
  std::thread thread;
  std::condition_variable msg_conditional;
  std::mutex lock;
  bool stop;
  bool msg_ready;
  std::string buffer;
  const std::string pipepath;
  const std::string name;
  int statuscode;

  const static void sender(const std::string pipepath,
                           std::string &buffer, int &statuscode,
                           bool &msg_ready, const bool &stop, std::mutex &lock,
                           std::condition_variable &msg_conditional);

public:
  Sender(void) = delete;
  Sender(const std::string name, const std::string pipepath);
  const int get_status_code(void) const;
  const bool send(const std::string msg);
  const bool terminate(void);
};

} // namespace cppiper

#endif // SENDER_HH_
