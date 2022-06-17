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
#include <filesystem>

namespace cppiper {

//! A class responsible for receiving messages.
class Receiver {
private:
  //! Identifying name of this receiver instance (for debugging).
  const std::string name;
  //! Path to the receiver pipe.
  const std::filesystem::path pipepath;
  //! Flag used to signal message is ready to be received.
  bool msg_ready;
  //! Code representing current status of receiver thread.
  int statuscode;
  //! Queue of received messages.
  std::queue<std::string> msg_queue;
  //! Queue lock for the queue conditional.
  std::mutex queue_lock;
  //! Conditional used to synchronise with the message queue.
  std::condition_variable queue_condition;
  //! Sender thread.
  std::thread thread;

  //! Static method to be executed in the receiver thread.
  /*!
    \param pipepath path to the receiver pipe.
    \param msg_ready reference to message ready signal bool.
    \param statuscode reference to status code.
    \param msg_queue reference to message queue.
    \param queue_lock reference to queue conditional lock.
    \param queue_condition reference to queue conditional.
   */
  static void receiver(const std::filesystem::path pipepath, bool &msg_ready,
                             int &statuscode,
                             std::queue<std::string> &msg_queue,
                             std::mutex &queue_lock,
                             std::condition_variable &queue_condition);

public:
  //! Deleted.
  Receiver(void) = delete;

  //! Construct a receiver.
  /*!
    \param name identifying name of this receiver instance (for debugging).
    \param pipepath path to the receiver pipe.
   */
  Receiver(const std::string name, const std::filesystem::path pipepath);

  //! Receive a message.
  /*!
    \param wait block until a message is available.
    \return An optional that contains a message if one was available.
   */
  std::optional<const std::string> receive(bool wait);

  //! Wait for communication line to be closed.
  /*!
    \return Whether or not the wait was successful.
   */
  bool wait(void);

  //! Get the pipe path.
  /*!
    \return Pipe path.
   */
  std::filesystem::path get_pipe(void) const;

};

} // namespace cppiper

#endif // RECEIVER_HH_
