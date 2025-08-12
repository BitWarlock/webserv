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

std::string	generateTempName()
{
	std::stringstream ss;
	ss << "/tmp/cgi_temp_" << "_" << std::time(NULL);
	return ss.str();
}

std::string	Cgi::replacePlaceholders(const std::string& cmd, const std::string& input, const std::string& output)
{
	std::string	result = cmd;
	size_t		pos = 0;

	while ((pos = result.find("{INPUT}")) != std::string::npos)
	{
		result.replace(pos, 7, input);
		pos += input.length();
	}

	pos = 0;

	while ((pos = result.find("{OUTPUT}")) != std::string::npos)
	{
		result.replace(pos, 8, output);
		pos += output.length();
	}
	return result;
}

int	Cgi::childProcessWait(pid_t pid, int max_timeout_seconds)
{
	int status;
	int elapsed = 0;

	while (true)
	{
		int ret = waitpid(pid, &status, WNOHANG);
		if (ret == pid)
			return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : -1;

		if (ret == -1)
			return -1;

		if (elapsed >= max_timeout_seconds)
		{
			killProcess(pid);
			waitpid(pid, &status, 0);
			return 1;
		}

		usleep_(1000000);
		++elapsed;
	}
}

void Cgi::runChildProcess(int pipe_in[2], int pipe_out[2], ParseRequest& request,
				const std::string& script_path, const std::string& interpreter)
{
	close(pipe_in[1]);
	close(pipe_out[0]);

	dup2(pipe_in[0], STDIN_FILENO);
	dup2(pipe_out[1], STDOUT_FILENO);
	dup2(pipe_out[1], STDERR_FILENO);

	close(pipe_in[0]);
	close(pipe_out[1]);

	char** env_array = getEnv(request);

	if (interpreter.find("{INPUT}") != std::string::npos
			|| interpreter.find("{OUTPUT}") != std::string::npos)
	{
		std::string output_path = script_path + ".out";
		std::string full_cmd = replacePlaceholders(interpreter, script_path, output_path);
		char* argv[] = { const_cast<char*>("/bin/sh"), const_cast<char*>("-c"), const_cast<char*>(full_cmd.c_str()), NULL };
		execve("/bin/sh", argv, env_array);
	}
	else
	{
		char* args[] = { const_cast<char*>(interpreter.c_str()), const_cast<char*>(script_path.c_str()), NULL };
		execve(interpreter.c_str(), args, env_array);
	}

	cleanEnv(env_array);
	exit(1);
}

void Cgi::parentPipe(ParseRequest& request, int pipe_in[2])
{
	close(pipe_in[0]);

	if (request.getMethod() == "POST")
		writeCgiInput(request, pipe_in[1]);

	close(pipe_in[1]);
}

CgiResult Cgi::execute(ParseRequest& request)
{
	const size_t	timeout = request.getBlockServer().getClientTimeout();

	std::cout << "Timeout " << timeout << std::endl;

	if (access(script_path.c_str(), F_OK | R_OK) != 0)
		return CgiResult(false, "Script not accessible: " + script_path, 403);

	setupEnv(request);

	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
		return CgiResult(false, "Failed to create pipes for CGI execution", 500);

	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		return CgiResult(false, "Fork failed", 500);
	}

	if (pid == 0)
		runChildProcess(pipe_in, pipe_out, request, script_path, interpreter);

	parentPipe(request, pipe_in);

	close(pipe_out[1]);
	std::string output = readCgiOutput(pipe_out[0], pid, timeout / 2);
	close(pipe_out[0]);

	int wait_result = childProcessWait(pid, timeout / 2);

	if (wait_result == 1)
		return CgiResult(false, "CGI script execution timed out", 504);
	if (wait_result == -1)
		return CgiResult(false, "Error waiting for CGI script", 500);

	return parseCgiOutput(output);
}

void	usleep_(int mseconds)
{
	clock_t	start = clock();

	clock_t ticks_wait = mseconds * (CLOCKS_PER_SEC / 1000000.0);
	while (clock() - start <= ticks_wait) {}
}

std::string Cgi::readCgiOutput(int pipe_fd, pid_t pid, int timeout_seconds)
{
	std::string		output;
	char			buffer[4096];
	const size_t	max_output_size = 1024 * 1024 * 10; // 10 MB
	time_t			start_time = std::time(NULL);
	int				flags = fcntl(pipe_fd, F_GETFL, 0);

	fcntl(pipe_fd, F_SETFL, flags | O_NONBLOCK);

	int			failed_reads = 0;
	const int	max_failed_reads = 1000;
	bool		got_data = false;

	while (std::time(NULL) - start_time < timeout_seconds)
	{
		ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			output.append(buffer, bytes_read);
			got_data = true;
			failed_reads = 0;
			if (output.size() > max_output_size) break;
			continue;
		}

		if (bytes_read == 0 && (got_data || waitpid(pid, NULL, WNOHANG) != 0))
			break;

		if (++failed_reads > max_failed_reads)
		{
			if (got_data || waitpid(pid, NULL, WNOHANG) != 0) break;
			failed_reads = 0;
		}

		usleep_(1000);
	}

	fcntl(pipe_fd, F_SETFL, flags);
	return output;
}

void	Cgi::writeCgiInput(ParseRequest& request, int pipe_fd)
{
	if (request.getMethod() == "POST") {
		std::string body = request.getBufferBody();
		if (!body.empty()) {
			const char* data = body.c_str();
			size_t remaining = body.length();
			size_t total_written = 0;
			
			while (remaining > 0) {
				ssize_t written = write(pipe_fd, data + total_written, remaining);
				
				if (written < 0) {
					throw std::runtime_error("Failed to write POST data to CGI script");
				}
				
				total_written += written;
				remaining -= written;
			}
		}
	}
}

void	Cgi::killProcess(pid_t pid)
{
	kill(pid, SIGTERM);
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
	if (header_end == std::string::npos)
	{
		header_end = raw_output.find("\n\r\n");
	}
	if (header_end == std::string::npos)
	{
		header_end = raw_output.find("\n");
	}
	
	if (header_end != std::string::npos)
	{
		std::string headers_str = raw_output.substr(0, header_end);
		size_t body_start = header_end;
		if (raw_output.substr(header_end, 4) == "\r\n\r\n")
			body_start += 4;
		else if (raw_output.substr(header_end, 2) == "\n\n")
			body_start += 2;
		else if (raw_output.substr(header_end, 3) == "\n\r\n")
			body_start += 3;
		else if (raw_output.substr(header_end, 1) == "\n")
			body_start += 1;
		result.body = raw_output.substr(body_start);
		
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
	env_vars["SCRIPT_FILENAME"] = script_path;
	env_vars["REQUEST_URI"] = request.getUri();
	env_vars["PATH"] = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";
	env_vars["LIBRARY_PATH"] = "/usr/lib/x86_64-linux-gnu:/usr/lib";

	if (request.getMethod() == "POST") {
		if (env_vars["CONTENT_TYPE"].empty()) {
			env_vars["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
		}
		if (env_vars["CONTENT_LENGTH"].empty()) {
			env_vars["CONTENT_LENGTH"] = "0";
		}
	}
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

	try {
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
	catch (...) {
		for (int j = 0; j < i; j++)
			delete[] env_array[j];
		delete[] env_array;
		throw;
	}
}

void	Cgi::cleanEnv(char **env) const
{
	if (!env) return;
	
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
