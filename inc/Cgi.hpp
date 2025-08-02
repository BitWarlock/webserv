#ifndef CGI_HPP
#define CGI_HPP

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include "ParseRequest.hpp"
#include "Server.hpp"
#include <map>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct CgiResult
{
	std::string	headers;
	std::string	body;
	int			status_code;
	bool		success;
	std::string	error_message;
	
	CgiResult() : status_code(200), success(false) {}
	CgiResult(bool s, const std::string& error = "") : status_code(200), success(s), error_message(error) {}
};

class Cgi
{
	public:
		Cgi(const std::string& script_path, const std::string& interpreter);
		Cgi(const Cgi& other);
		Cgi& operator=(const Cgi& other);
		~Cgi();

		CgiResult	execute(ParseRequest& request);

	private:
		std::map<std::string,
			std::string>	env_vars;
		std::string			script_path;
		std::string			interpreter;


		void	setupEnv(ParseRequest& request);
		void	cleanEnv(char **env) const;
		void	writeCgiInput(ParseRequest& request, int pipe_fd);
		void	killProcess(pid_t pid);
		void	setBasicEnv(ParseRequest& request);
		void	setRequestEnv(ParseRequest& request);
		void	setServerEnv();

		CgiResult	executeCompiled(ParseRequest& request);
		CgiResult	executePipes(ParseRequest& request);
		CgiResult	parseCgiOutput(const std::string& raw_output);

		std::string	replacePlaceholders(const std::string& cmd, const std::string& input, const std::string& output);
		std::string	readCgiOutput(int pipe_fd, pid_t pid, int timeout_sec = 30);
		std::string	urlDecode(const std::string& encoded) const;
		std::string	intToString(int value) const;

		char**		getEnv(ParseRequest& request) const;
};

#endif // !CGI_HPP
