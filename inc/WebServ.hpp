#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <sys/stat.h>

#include "Lexer.hpp"

#define GRN "\e[0;32m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

struct ServerConfig
{
	std::string							server_name;
	std::string							listen;
	std::string							root;
	std::vector<std::string>			index;
	std::map<int, std::string>			error_pages;
	std::map<std::string, std::string>	cgi_info;
	size_t								client_body_limit;
	bool								autoindex;
};

#endif // !WEBSERV_HPP
