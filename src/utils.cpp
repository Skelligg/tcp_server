#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

#include "utils.hpp"

AddrInfo resolveAddress(std::string &port) {
  addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  addrinfo *res{nullptr};
  int rv = getaddrinfo(NULL, port.c_str(), &hints, &res);
  if (rv != 0) {
    throw std::runtime_error(gai_strerror(rv));
  }

  return AddrInfo(res);
}
