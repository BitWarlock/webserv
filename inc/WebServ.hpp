#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Lexer.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Config.hpp"
#include "ParseDirective.hpp"
#include "SetupServers.hpp"
#include "ParseRequest.hpp"
#include "Response.hpp"

#include <sys/stat.h>
#include <fstream>
#include <iterator>
#include <cstring>
#include <cstdlib>

#define GRN "\e[0;32m"
#define YLW "\033[1;33m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

#endif // !WEBSERV_HPP
