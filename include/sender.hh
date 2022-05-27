#ifndef SENDER_HH_
#define SENDER_HH_
#include <barrier>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace cppiper {

class Sender {
private:
  std::thread thread;
  std::barrier<> processed_barrier;
  std::condition_variable msg_conditional;
  std::mutex lock;
  bool stop;
  bool msg_ready;
  std::vector<char> buffer;
  std::string pipepath;
  int statuscode;

  const static void sender(std::string &pipepath, std::vector<char> &buffer,
                           int &statuscode, bool &msg_ready, bool &stop,
                           std::mutex &lock,
                           std::condition_variable &msg_conditional,
                           std::barrier<> &processed_barrier);

public:
  Sender(void) = delete;
  ~Sender(void);
  Sender(std::string pipe_path);
  const int get_status_code(void);
  const bool send(const std::vector<char> &msg);
  const bool terminate(void);
};

} // namespace cppiper

#endif // SENDER_HH_
