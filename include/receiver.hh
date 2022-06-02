#ifndef RECEIVER_HH_
#define RECEIVER_HH_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace cppiper {

class Receiver {
private:
  std::thread thread;
  std::condition_variable queue_condition;
  std::mutex queue_lock;
  std::queue<std::string> msg_queue;
  int statuscode;
  bool msg_ready;
  std::string pipepath;
  std::string name;

  const static void receiver(std::string pipepath, bool &msg_ready,
                             int &statuscode,
                             std::queue<std::string> &msg_queue,
                             std::mutex &queue_lock,
                             std::condition_variable &queue_condition);

public:
  Receiver(void) = delete;
  Receiver(std::string name, std::string pipepath);
  ~Receiver(void);
  const std::optional<const std::string> receive(bool wait);
  const bool wait(void);
};

} // namespace cppiper

#endif // RECEIVER_HH_
