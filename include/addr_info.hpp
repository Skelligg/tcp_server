#include <netdb.h>

class AddrInfo {
public:
  explicit AddrInfo(addrinfo *res) : res_(res) {}
  ~AddrInfo() {
    if (res_)
      freeaddrinfo(res_);
  }

  AddrInfo(const AddrInfo &) = delete;
  AddrInfo &operator=(const AddrInfo &) = delete;

  AddrInfo(AddrInfo &&other) noexcept : res_(other.res_) {
    other.res_ = nullptr;
  }

  addrinfo *get() const { return res_; }

private:
  addrinfo *res_;
};
