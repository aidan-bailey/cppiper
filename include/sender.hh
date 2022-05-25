#ifndef SENDER_HH_
#define SENDER_HH_
#include <barrier>
#include <mutex>
#include <string>
#include <thread>

namespace cppiper {



class Sender {
private:
  std::thread thread;
  std::barrier<> barrier;
  std::mutex lock;
  bool running;
  bool success;
  char *err_msg;
  char *buffer;
  std::string pipe_path;
  int msg_size;

  const static void sender(std::string pipe_path, int &msg_size, char *&buffer,
                         bool &success, char *&err_msg, bool &running,
                         std::mutex &lock, std::barrier<> &barrier
                         );

public:
  Sender(void) = delete;
  ~Sender(void);
  Sender(std::string pipe_path);
  const std::string get_err_msg(void);
  const bool send(char *msg, int size);
  const bool terminate(void);
};
} // namespace cppiper

#endif // SENDER_HH_
