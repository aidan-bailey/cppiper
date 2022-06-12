#ifndef SENDER_HH_
#define SENDER_HH_
#include <condition_variable>
#include <memory>
#include <mutex>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <vector>

namespace cppiper {

//! A class responsible for sending messages.
class Sender {
private:
  //! Sender thread.
  std::thread thread;
  //! Conditional used to synchronise with the thread.
  std::condition_variable msg_conditional;
  //! Lock utilised by conditional.
  std::mutex lock;
  //! Flag used to stop thread.
  bool stop;
  //! Flag used to signal message is ready to be sent.
  bool msg_ready;
  //! Message buffer.
  const std::string *buffer;
  //! Path to the sender pipe.
  const std::string pipepath;
  //! Identifying name of this sender instance (for debugging).
  const std::string name;
  //! Code representing current status of sender thread.
  int statuscode;

  //! Static method to be executed in the sender thread.
  /*!
   \param pipepath path to the sender pipe.
   \param buffer reference to message buffer.
   \param statuscode reference to status code.
   \param msg_ready reference to message ready signal bool.
   \param stop reference to stop signal bool.
   \param lock reference to conditional lock.
   \param msg_conditional reference to conditional.
   */
  const static void sender(const std::string pipepath, const std::string **buffer,
                           int &statuscode, bool &msg_ready, const bool &stop,
                           std::mutex &lock,
                           std::condition_variable &msg_conditional);

public:
  //! Deleted.
  Sender(void) = delete;

  //! Construct a sender.
  /*!
    \param name identifying name of this sender instance (for debugging).
    \param pipepath path to the sender pipe.
   */
  Sender(const std::string name, const std::string pipepath);

  //! Get the sender thread's current status code.
  /*!
    \return The current status code.
   */
  const int get_status_code(void) const;

  //! Send a message over the pipe.
  /*!
    \param msg a message to send.
    \return Whether or not the send was successful.
   */
  const bool send(const std::string &msg);

  //! Terminate the pipe connection.
  /*!
   \return Whether or not the termination was successful.
   */
  const bool terminate(void);
};

} // namespace cppiper

#endif // SENDER_HH_
