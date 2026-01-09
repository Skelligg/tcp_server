#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>

#include "client_connection.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include "tcp_server.hpp"
#include "utils.hpp"

TcpServer::TcpServer(std::string port, int backlog, sa_family_t ss_family)
    : port_{port}, backlog_{backlog}, ss_family_{ss_family},
      serverSocket_{-1}, epoll_{-1}, clients_{} {}

void TcpServer::start() {
  auto addr{resolveAddress(port_)};

  for (auto p{addr.get()}; p != NULL; p = p->ai_next) {
    if (tryBindAndListen(p)) {
      Socket epSocket{epoll_create1(0)};
      if (epSocket.fd() == -1)
        throw std::system_error(errno, std::system_category(), "socket");

      epoll_ = std::move(epSocket);
      epoll_event ev{};
      ev.events = EPOLLIN;
      ev.data.fd = serverSocket_.fd();
      epoll_ctl(epoll_.fd(), EPOLL_CTL_ADD, serverSocket_.fd(), &ev);

      return;
    }
  }

  throw std::runtime_error("Failed to bind to any address");
}

void TcpServer::run() {
  int MAX_EVENTS{10};
  epoll_event events[MAX_EVENTS];
  int nfds;
  while (true) {
    nfds = epoll_wait(epoll_.fd(), events, MAX_EVENTS, -1);
    if (nfds == -1) {
      throw std::runtime_error("epoll_wait failed");
    }

    for (int n{0}; n < nfds; ++n) {
      if (events[n].data.fd == serverSocket_.fd()) {
        acceptConnection();
      } else {
        handleClientConnection(events[n].data.fd, events[n].events);
      }
    }
  }
}

void TcpServer::acceptConnection() {
  sockaddr_storage theirAddr;
  socklen_t addrSize{sizeof(theirAddr)};

  Socket s{accept(serverSocket_.fd(), (sockaddr *)&theirAddr, &addrSize)};
  if (s.fd() == -1)
    throw std::system_error(errno, std::system_category(), "accept");

  char theirAddrName[INET6_ADDRSTRLEN];

  inet_ntop(theirAddr.ss_family, get_in_addr((sockaddr *)&theirAddr),
            theirAddrName, sizeof theirAddrName);
  printf("server: got connection from %s\n", theirAddrName);

  fcntl(s.fd(), F_SETFL, O_NONBLOCK);

  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = s.fd();
  epoll_ctl(epoll_.fd(), EPOLL_CTL_ADD, s.fd(), &ev);

  clients_.emplace(s.fd(), ClientConnection(std::move(s)));
}

void TcpServer::handleClientConnection(int fd, uint32_t events) {
  auto it{clients_.find(fd)};
  if (it == clients_.end()) {
    return;
  }

  ClientConnection &client = it->second;

  if (events & EPOLLIN) {
    auto response{client.recvMsg()};

    switch (response) {
    case RecvResult::WouldBlock:
      break;

    case RecvResult::MessageRead: {
      std::string msg{"PONG"};
      client.queueMsg(MessageType::PING, msg);
      epoll_event ev{};
      ev.events = EPOLLIN | EPOLLOUT;
      ev.data.fd = fd;
      epoll_ctl(epoll_.fd(), EPOLL_CTL_MOD, fd, &ev);
      break;
    }

    case RecvResult::PartialMessageRead:
      break;

    case RecvResult::Disconnected:
      removeClient(fd);
      break;

    case RecvResult::Error:
      removeClient(fd);
      break;
    }
  }

  if (events & EPOLLOUT) {
    auto response{client.sendMsg()};

    switch (response) {
    case SendResult::WouldBlock:
      break;

    case SendResult::MessageSent: {
      epoll_event ev{};
      ev.events = EPOLLIN;
      epoll_ctl(epoll_.fd(), EPOLL_CTL_MOD, fd, &ev);
      break;
    }

    case SendResult::PartialSend:
      break;

    case SendResult::Disconnected:
      removeClient(fd);
      break;

    case SendResult::Error:
      removeClient(fd);
      break;
    }
  }

  if (events & (EPOLLHUP | EPOLLERR)) {
    removeClient(fd);
  }
}

void TcpServer::removeClient(int fd) {
  epoll_event ev{};
  epoll_ctl(fd, EPOLL_CTL_DEL, fd, &ev);

  auto it{clients_.find(fd)};
  clients_.erase(it);
}

bool TcpServer::tryBindAndListen(addrinfo *p) {
  Socket s{socket(p->ai_family, p->ai_socktype, p->ai_protocol)};

  if (s.fd() == -1)
    return false;

  if (p->ai_family == AF_INET6) {
    int no = 0;
    if (setsockopt(s.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no)) == -1) {
      return false;
    }
  }

  int yes{1};
  if (setsockopt(s.fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    return false;

  if (bind(s.fd(), p->ai_addr, p->ai_addrlen) == -1)
    return false;

  if (listen(s.fd(), backlog_) == -1)
    return false;

  fcntl(s.fd(), F_SETFL, O_NONBLOCK);

  serverSocket_ = std::move(s);
  return true;
}
