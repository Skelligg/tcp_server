#pragma once

#include "addr_info.hpp"
#include <netdb.h>
#include <string>

AddrInfo resolveAddress(std::string &port);
