#include "../inc/Cgi.hpp"

Cgi::Cgi(const std::string& script_path, const std::string& interpreter)
	: script_path(script_path), interpreter(interpreter)
{
	char abs_path[1024];
	if (realpath(script_path.c_str(), abs_path) != NULL) {
		this->script_path = abs_path;
	}
}

Cgi::~Cgi() {
}

CgiResult	Cgi::execute(ParseRequest& request)
{
	if (access(script_path.c_str(), F_OK | R_OK | X_OK) != 0)
	{
		return CgiResult(false, "Script not accessible: " + std::string(strerror(errno)));
	}
	
	setupEnv(request);
	
	return executePipes(request);
}

CgiResult Cgi::executePipes(ParseRequest& request)
{
	int pipe_in[2], pipe_out[2];
	
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
	{
		return CgiResult(false, "Failed to create pipes for CGI execution");
	}
	
	pid_t pid = fork();
	if (pid == -1) {
		return CgiResult(false, "Fork failed");
	}
	
	if (pid == 0) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		
		close(pipe_in[0]);
		close(pipe_out[1]);
		
		char* args[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(script_path.c_str()), NULL};
		char** env_array = getEnv(request);
		
		execve(interpreter.c_str(), args, env_array);
		
		cleanEnv(env_array);
		exit(1);
	}
	
	close(pipe_in[0]);
	close(pipe_out[1]);
	
	writeCgiInput(request, pipe_in[1]);
	close(pipe_in[1]);
	
	std::string output = readCgiOutput(pipe_out[0], pid);
	close(pipe_out[0]);
	
	int status;
	waitpid(pid, &status, 0);
	
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
	{
		return parseCgiOutput(output);
	}
	else
	{
		return CgiResult(false, "CGI script execution failed");
	}
}

std::string	Cgi::readCgiOutput(int pipe_fd, pid_t pid, int timeout_seconds)
{
	(void)pid;
	std::string output;
	char buffer[4096];
	fd_set read_fds;
	struct timeval timeout;
	
	FD_ZERO(&read_fds);
	FD_SET(pipe_fd, &read_fds);
	
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = 0;
	
	while (select(pipe_fd + 1, &read_fds, NULL, NULL, &timeout) > 0)
	{
		ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1);
		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			output += buffer;
		}
		else if (bytes_read == 0)
		{
			break;
		}
		else
		{
			break;
		}
	}
	
	return output;
}

void	Cgi::writeCgiInput(ParseRequest& request, int pipe_fd)
{
	std::string body = request.getBufferBody();
	if (!body.empty())
	{
		write(pipe_fd, body.c_str(), body.length());
	}
}

void	Cgi::killProcess(pid_t pid)
{
	kill(pid, SIGTERM);
	usleep(100000);
	kill(pid, SIGKILL);
}

CgiResult	Cgi::parseCgiOutput(const std::string& raw_output)
{
	CgiResult result;
	result.success = true;
	result.status_code = 200;
	
	size_t header_end = raw_output.find("\r\n\r\n");
	if (header_end == std::string::npos)
	{
		header_end = raw_output.find("\n\n");
	}
	
	if (header_end != std::string::npos)
	{
		std::string headers_str = raw_output.substr(0, header_end);
		result.body = raw_output.substr(header_end + 4);
		
		std::istringstream	header_stream(headers_str);
		std::string			line;
		
		while (std::getline(header_stream, line))
		{
			if (line.empty() || line == "\r") continue;
			
			size_t colon_pos = line.find(':');
			if (colon_pos != std::string::npos)
			{
				std::string key = line.substr(0, colon_pos);
				std::string value = line.substr(colon_pos + 1);
				
				value.erase(0, value.find_first_not_of(" \t"));
				value.erase(value.find_last_not_of(" \t") + 1);
				
				if (key == "Status")
				{
					size_t space_pos = value.find(' ');
					if (space_pos != std::string::npos)
						result.status_code = std::atoi(value.substr(0, space_pos).c_str());
				}
				else
					result.headers += key + ": " + value + "\r\n";
			}
		}
	}
	else
	{
		result.body = raw_output;
	}
	
	if (result.headers.find("Content-Type") == std::string::npos)
		result.headers = "Content-Type: text/html\r\n" + result.headers;
	
	return result;
}

std::string	Cgi::intToString(int value) const
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void	Cgi::setupEnv(ParseRequest& request)
{
	env_vars.clear();

	setBasicEnv(request);
	setRequestEnv(request);
	setServerEnv();
}

void	Cgi::setBasicEnv(ParseRequest& request)
{
	env_vars["REQUEST_METHOD"] = request.getMethod();
    env_vars["QUERY_STRING"] = request.getQueryString();
    env_vars["CONTENT_LENGTH"] = intToString(request.getContentLength());
    env_vars["CONTENT_TYPE"] = request.getHeaderValue("Content-Type");
    env_vars["SCRIPT_NAME"] = request.getUri();
    env_vars["PATH_INFO"] = "";
    env_vars["PATH_TRANSLATED"] = script_path;
}

void	Cgi::setRequestEnv(ParseRequest& request)
{
	std::vector<std::pair<std::string, std::string> > headers = request.getHeaders();

	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = headers.begin();
			it != headers.end(); ++it)
	{
		std::string env_name = "HTTP_" + it->first;

		std::transform(env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
		std::replace(env_name.begin(), env_name.end(), '-', '_');

		env_vars[env_name] = it->second;
	}
}

void	Cgi::setServerEnv()
{
	env_vars["SERVER_NAME"] = "localhost";
	env_vars["SERVER_PORT"] = "8003";
	env_vars["SERVER_PROTOCOL"] = "HTTP/1.1";
	env_vars["SERVER_SOFTWARE"] = "WebServ/1.0";
	env_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
}

char**	Cgi::getEnv(ParseRequest& request) const
{
	(void)request;
	char**	env_array = new char*[env_vars.size() + 1];
	int		i = 0;

	for (std::map<std::string, std::string>::const_iterator it = env_vars.begin();
			it != env_vars.end(); ++it, ++i)
	{
		std::string env_var = it->first + "=" + it->second;
		env_array[i] = new char[env_var.length() + 1];
		std::strcpy(env_array[i], env_var.c_str());
	}

	env_array[i] = NULL;
	return env_array;
}

void	Cgi::cleanEnv(char **env) const
{
	for (int i = 0; env[i] != NULL; i++)
		delete[] env[i];
	delete[] env;
}

Cgi::Cgi(const Cgi& other)
	: env_vars(other.env_vars), script_path(other.script_path), interpreter(other.interpreter)
{
}

Cgi& Cgi::operator=(const Cgi& other)
{
	if (this != &other) {
		script_path = other.script_path;
		interpreter = other.interpreter;
		env_vars = other.env_vars;
	}
	return *this;
}
