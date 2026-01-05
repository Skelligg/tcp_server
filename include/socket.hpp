#pragma once

#include <unistd.h>

class Socket {
public:
  // Constructor
  explicit Socket(int fd) : fd_(fd) {}

  // Destructor
  ~Socket() {
    if (fd_ >= 0)
      ::close(fd_);
  }

  // Copy Constructor
  Socket(const Socket &) = delete;

  // Copy Assignment Constructor
  Socket &operator=(const Socket &) = delete;

  // Move Constructor
  Socket(Socket &&other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
  // Move Assignment Constructor
  Socket &operator=(Socket &&other) noexcept {
    if (this != &other) {
      if (fd_ >= 0)
        ::close(fd_);

      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  int fd() const { return fd_; }

private:
  int fd_{-1};
};
