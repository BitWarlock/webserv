#include "../inc/HttpResponse.hpp"

HttpResponse::HttpResponse() 
	: status_code_(200), status_message_("OK"), keep_alive_(true) {}

	HttpResponse::~HttpResponse()
{}

void HttpResponse::generateResponse(ParseRequest& request, const Server& server)
{
	status_code_ = 200;
	status_message_ = "OK";
	headers_.clear();
	body_.clear();

	const Location* location = findMatchingLocation(request, server);
	if (!location)
		return (generateErrorResponse(404, server));

	const std::vector<std::string>& allowed_methods = location->getAllowedMethods();
	if (std::find(allowed_methods.begin(),
				allowed_methods.end(), request.getMethod()) == allowed_methods.end())
		return (generateErrorResponse(405, server));

	if (request.getMethod() == "GET")
	{
		std::string file_path = resolveFilePath(request, location);

		if (!fileExistsAndReadable(file_path))
			return (generateErrorResponse(404, server));

		struct stat stat_buf;
		if (stat(file_path.c_str(), &stat_buf) == 0)
		{
			if (S_ISDIR(stat_buf.st_mode))
			{
				if (location->getAutoindex())
					return generateAutoindexResponse(file_path, request.getUri());
				else
					return (generateErrorResponse(403, server));
			}
		}

		generateFileResponse(file_path, server);
	}
	else
	{
		// TODO : POST, DELETE
		setStatus(200, "OK");
		setDefaultHeaders();
	}
}

std::string HttpResponse::serialize() const
{
	std::ostringstream response_stream;

	response_stream << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); 
			it != headers_.end(); ++it)
		response_stream << it->first << ": " << it->second << "\r\n";

	response_stream << "\r\n";

	if (!body_.empty())
		response_stream << body_;

	return response_stream.str();
}

std::vector<std::string> HttpResponse::serializeChunked() const
{
	std::vector<std::string> chunks;

	std::ostringstream header_stream;
	header_stream << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); 
			it != headers_.end(); ++it)
		header_stream << it->first << ": " << it->second << "\r\n";
	header_stream << "\r\n";

	chunks.push_back(header_stream.str());

	if (!body_.empty())
	{
		const size_t chunk_size = 4096;
		size_t pos = 0;

		while (pos < body_.size())
		{
			size_t end_pos = std::min(pos + chunk_size, body_.size());
			std::string chunk = body_.substr(pos, end_pos - pos);
			pos = end_pos;

			std::ostringstream chunk_stream;
			chunk_stream << std::hex << chunk.size() << "\r\n";
			chunk_stream << chunk << "\r\n";
			chunks.push_back(chunk_stream.str());
		}
	}

	chunks.push_back("0\r\n\r\n");
	return chunks;
}

void HttpResponse::generateFileResponse(const std::string& file_path, const Server& server)
{
	size_t file_size = getFileSize(file_path);

	if (file_size > 1024 * 1024)
	{
		setHeader("Transfer-Encoding", "chunked");
		setStatus(200, "OK");
		setDefaultHeaders();
		setContentType(determineContentType(file_path));

		std::ifstream file(file_path.c_str(), std::ios::binary);
		if (!file)
			return generateErrorResponse(500, server);

		const size_t	buffer_size = 4096;
		char			buffer[buffer_size];

		while (file.read(buffer, buffer_size))
			body_.append(buffer, file.gcount());
		body_.append(buffer, file.gcount());
	}
	else
	{
		std::string content = readFileContent(file_path);
		if (content.empty())
			return generateErrorResponse(500, server);

		setStatus(200, "OK");
		setDefaultHeaders();
		setContentType(determineContentType(file_path));
		setBody(content);
		setContentLength();
	}
}

void HttpResponse::generateErrorResponse(int error_code, const Server& server)
{
	const std::string&	error_page = server.getErrorPage(error_code);
	std::string			content;

	if (!error_page.empty() && fileExistsAndReadable(error_page))
		content = readFileContent(error_page);
	else
	{
		std::ostringstream oss;
		oss << "<html><head><title>" << error_code << " " << getStatusMessageForCode(error_code) 
			<< "</title></head><body><h1>" << error_code << " " << getStatusMessageForCode(error_code) 
			<< "</h1></body></html>";
		content = oss.str();
	}

	setStatus(error_code, getStatusMessageForCode(error_code));
	setDefaultHeaders();
	setContentType("text/html");
	setBody(content);
	setContentLength();
}

void HttpResponse::generateAutoindexResponse(const std::string& dir_path, const std::string& uri)
{
	std::ostringstream content;

	content << "<html><head><title>Index of " << uri << "</title></head><body>";
	content << "<h1>Index of " << uri << "</h1><hr><pre>";

	DIR* dir = opendir(dir_path.c_str());
	if (dir)
	{
		struct dirent* ent;
		while ((ent = readdir(dir)) != NULL)
		{
			std::string name = ent->d_name;
			if (name != "." && name != "..")
			{
				content << "<a href=\"" << uri;
				if (uri[uri.length()-1] != '/') content << "/";
				content << name << "\">" << name << "</a><br>";
			}
		}
		closedir(dir);
	}

	content << "</pre><hr></body></html>";

	setStatus(200, "OK");
	setDefaultHeaders();
	setContentType("text/html");
	setBody(content.str());
	setContentLength();
}

void HttpResponse::setDefaultHeaders()
{
	setDateHeader();
	setServerHeader();
	setConnection(true);
}

void HttpResponse::setContentLength()
{
	std::ostringstream oss;
	oss << body_.size();
	setHeader("Content-Length", oss.str());
}

void HttpResponse::setContentType(const std::string& type)
{
	setHeader("Content-Type", type);
}

void HttpResponse::setConnection(bool keep_alive)
{
	keep_alive_ = keep_alive;
	setHeader("Connection", keep_alive ? "keep-alive" : "close");
}

void HttpResponse::setDateHeader()
{
	char buf[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	setHeader("Date", buf);
}

void HttpResponse::setServerHeader()
{
	setHeader("Server", "webserv/1.0");
}

bool HttpResponse::fileExistsAndReadable(const std::string& path) const
{
	if (access(path.c_str(), F_OK | R_OK) == -1)
		return false;
	return true;
}

size_t HttpResponse::getFileSize(const std::string& path) const
{
	struct stat stat_buf;
	if (stat(path.c_str(), &stat_buf) != 0)
		return 0;
	return stat_buf.st_size;
}

std::string HttpResponse::readFileContent(const std::string& path) const
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
		return "";

	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

std::string HttpResponse::getStatusMessageForCode(int code) const
{
	switch (code)
	{
		case 200: return "OK";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown Error";
	}
}
const Location* HttpResponse::findMatchingLocation(ParseRequest& request, const Server& server) const
{
	const std::vector<Location*>&	locations = server.getLocations();
	const std::string&				path = request.getUri();

	const Location* best_match = NULL;
	size_t			best_length = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{
		const std::string& loc_path = locations[i]->getPath();
		if (path.compare(0, loc_path.length(), loc_path) == 0)
		{
			if (loc_path.length() > best_length)
			{
				best_length = loc_path.length();
				best_match = locations[i];
			}
		}
	}

	return best_match;
}

std::string HttpResponse::resolveFilePath(ParseRequest& request, const Location* location) const
{
	std::string file_path = location->getRoot();
	std::string uri = request.getUri();

	if (uri.find(location->getPath()) == 0)
		uri.erase(0, location->getPath().length());

	if (!uri.empty() && uri[0] != '/')
		file_path += "/";
	file_path += uri;

	struct stat stat_buf;

	if (stat(file_path.c_str(), &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode))
	{
		const std::vector<std::string>& index_files = location->getIndex();
		for (size_t i = 0; i < index_files.size(); ++i)
		{
			std::string index_path = file_path + "/" + index_files[i];
			if (fileExistsAndReadable(index_path))
				return index_path;
		}
	}

	return file_path;
}

std::string HttpResponse::determineContentType(const std::string& file_path) const
{
	size_t dot_pos = file_path.rfind('.');
	if (dot_pos == std::string::npos)
		return "application/octet-stream";

	std::string ext = file_path.substr(dot_pos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "css") return "text/css";
	if (ext == "js") return "application/javascript";
	if (ext == "json") return "application/json";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "png") return "image/png";
	if (ext == "gif") return "image/gif";
	if (ext == "txt") return "text/plain";
	if (ext == "pdf") return "application/pdf";

	return "application/octet-stream";
}

void HttpResponse::setStatus(int code, const std::string& message)
{
	status_code_ = code;
	status_message_ = message;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value)	{ headers_[name] = value; }

void HttpResponse::setBody(const std::string& content)	{ body_ = content; }

void HttpResponse::appendBody(const std::string& content)	{ body_ += content; }

int HttpResponse::getStatusCode() const	{ return status_code_; }

const std::string& HttpResponse::getStatusMessage() const	{ return status_message_; }

const std::map<std::string, std::string>& HttpResponse::getHeaders() const	{ return headers_; }

const std::string& HttpResponse::getBody() const	{ return body_; }

bool HttpResponse::isKeepAlive() const	{ return keep_alive_; }
