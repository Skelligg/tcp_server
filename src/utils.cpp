#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

#include "utils.hpp"

AddrInfo resolveAddress(const std::string &port, const std::string &host) {
  addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo *res{nullptr};
  if (host.empty()) {
    hints.ai_flags = AI_PASSIVE;

    int rv = getaddrinfo(NULL, port.c_str(), &hints, &res);
    if (rv != 0) {
      throw std::runtime_error(gai_strerror(rv));
    }
  } else {
    int rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (rv != 0) {
      throw std::runtime_error(gai_strerror(rv));
    }
  }

  return AddrInfo(res);
}
