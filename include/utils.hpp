#pragma once

#include "addr_info.hpp"
#include <netdb.h>
#include <string>

AddrInfo resolveAddress(const std::string &port, const std::string &host = "");
