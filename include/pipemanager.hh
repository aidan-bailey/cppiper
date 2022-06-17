#ifndef PIPEMANAGER_HH_
#define PIPEMANAGER_HH_
#include <mutex>
#include <sstream>
#include <string>
#include <tuple>
#include <filesystem>

namespace cppiper {

//! Generate a random hex string of a specified length.
/*!
  \param len a specified length.
  \return The hex string.
 */
std::string random_hex(int len);

//! A pipe managing class.
/*!
  Manages the creation and removal of named pipes in a specific directory (pipe
  directory).
 */
class PipeManager {
private:
  //! Pipe directory lock.
  std::mutex lock;
  //! Pipe directory path.
  const std::filesystem::path pipedir;

public:
  //! Deleted.
  PipeManager(void) = delete;

  //! Construct a pipe manager.
  /*!
    \param pipedir a directory path for the pipes (this directory will be
    created if it does not exist).
   */
  PipeManager(const std::filesystem::path pipedir);

  //! Make a new fifo pipe with a random name.
  /*!
    \return The path to the pipe.
  */
  std::string make_pipe(void);

  //! Remove a pipe from the pipe directory.
  /*!
    \param pipepath path to the pipe.
    \return whether or not the removal was successful.
  */
  bool remove_pipe(const std::filesystem::path pipepath);

  //! Clear the pipe directory of pipes.
  void clear(void);
};

} // namespace cppiper

#endif // PIPEMANAGER_HH_
