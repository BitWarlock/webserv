/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/09/15 18:12:08 by mrezki           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Response.hpp"
#include <ctime>

static bool	hasParentDirectorySegment(const std::string& uri)
{
	size_t	start = 0;

	while (start <= uri.size())
	{
		size_t end = uri.find('/', start);
		std::string segment = uri.substr(start,
				end == std::string::npos ? std::string::npos : end - start);
		if (segment == "..")
			return true;
		if (end == std::string::npos)
			break;
		start = end + 1;
	}
	return false;
}

static bool	isSafeUploadFilename(const std::string& filename)
{
	return (!filename.empty()
		&& filename != "."
		&& filename != ".."
		&& filename.find('/') == std::string::npos
		&& filename.find('\\') == std::string::npos
		&& filename.find('\0') == std::string::npos);
}

Response::Response() : fd(-1), fd_client(-1), state(OK), state_path(0),
	                      location(NULL), sessionManager(NULL),
	                      responseBody(""), buildRes(false), ignore(false),
	                      bytes(0), file_stream(NULL), file_size(0),
	                      bytes_sent(0), is_large_file_response(false) {}

Response::~Response()
{
    if (file_stream)
    {
        file_stream->close();
        delete file_stream;
    }
}

void    Response::GetFullPath(std::string& path)
{
	if (location)
	{
		std::string	loc_path = location->getPath();
		std::string	temp_path = path.substr(loc_path.size());
		
			if (!temp_path.empty() && temp_path[0] == '/')
				temp_path = temp_path.erase(0, 1);

			if (!location->getRoot().empty())
				SetPath(location->getRoot() + temp_path);

			else if (!ServerBlock.getRoot().empty())
			{
				std::string request_path = path;
				if (!request_path.empty() && request_path[0] == '/')
					request_path.erase(0, 1);
				SetPath(ServerBlock.getRoot() + request_path);
			}
		}

		else
		{
			if (path.empty() || path[0] != '/')
				return SetPath(ServerBlock.getRoot() + path);

			std::string request_path = path;
			request_path.erase(0, 1);
			SetPath(ServerBlock.getRoot() + request_path);
		}

	if (Request.getVersion().empty())
		Request.setVersion("HTTP/1.1");
}

bool    Response::CheckAutoIndex(void)
{
	if (location)
		return (location->getAutoindex());

	return (ServerBlock.getAutoindex());
}

void    Response::CheckIndexAccess(std::vector<std::string> indexs)
{
	bool found = false;

	for (size_t i(0); i < indexs.size(); i++)
	{
		if (path[path.size() - 1] != '/')
			path += "/";

		std::string	join = path + indexs[i];

		if (access(join.c_str(), F_OK) == 0)
		{
			SetPath(join);
			found = true;
			break;
		}
	}

	if (found)
		BuildGetResponse();

	else if (CheckAutoIndex() == ON)
		GetListingPage();

	else
		SetState(Forbidden), ResponseWithError(NONE);
}

void    Response::SearchForIndex(void)
{
	if (location)
	{
		if (!location->getIndex().empty())
			CheckIndexAccess(location->getIndex());

		else if (CheckAutoIndex() == ON)
			GetListingPage();

		else
			return (SetState(Forbidden), ResponseWithError(NONE));
	}
	else
	{
		if (!ServerBlock.getIndex().empty())
			CheckIndexAccess(ServerBlock.getIndex());

		else if (CheckAutoIndex() == ON)
			GetListingPage();

		else
			return (SetState(Forbidden), ResponseWithError(NONE));
	}
}

std::string Response::getStrState(void) const
{
	switch (getState())
	{
		case Created:
			return ("Created");
		case Moved_Permanently:
			return ("Moved Permanently");
		case Found:
			return ("Found");
		case See_Other:
			return ("See Other");
		case Temporary:
			return ("Temporary");
		case Permanent_Redirect:
			return ("Permanent Redirect");
		case Bad_Request:
			return ("Bad Request");
		case Forbidden:
			return ("Forbidden");
		case Not_Found:
			return ("Not Found");
		case Method_Not_Allowed:
			return ("Method Not Allowed");
		case Content_Too_Large:
			return ("Content Too Large");
		case URI_Too_Long:
			return ("URI Too Long");
		case Request_Header_Fields_Too_Large:
			return ("Request Header Fields Too Large");
		case Request_Timeout:
			return ("Request Timeout");
		case Internal_Server_Error:
			return ("Internal Server Error");
		case Not_Implemented:
			return ("Not Implemented");
		case HTTP_Version_Not_Supported:
			return ("HTTP Version Not Supported");
		case Bad_Gateway:
			return ("Bad Gateway");
		case Gateway_Timeout:
			return ("Gateway Timeout");
		case Unsupported_Media_Type:
			return ("Unsupported Media Type");
		case Conflict:
			return ("Conflict");
		case Length_Required:
			return ("Length Required");
		default:
			return ("OK");
	}
}

std::string Response::MIME_Type(void) const
{
	size_t  pos = path.rfind('.');

	if (pos == std::string::npos)
		return ("application/octet-stream");

	std::string extenstion = path.substr(pos + 1);

	if (extenstion == html || extenstion == htm)
		return ("text/html");
	else if (extenstion == css)
		return ("text/css");
	else if (extenstion == js)
		return ("application/javascript");
	else if (extenstion == json || extenstion == pdf || extenstion == zip
			|| extenstion == xml)
		return ("application/" + extenstion);
	else if (extenstion == txt)
		return ("text/plain");
	else if (extenstion == jpg || extenstion == jpeg)
		return ("image/jpeg");
	else if (extenstion == png || extenstion == gif)
		return ("image/" + extenstion);
	else if (extenstion == svg)
		return ("image/svg+xml");
	else if (extenstion == ico)
		return ("image/x-icon");
	else if (extenstion == ogg)
		return ("audio/ogg");
	else if (extenstion == mp3)
		return ("audio/mpeg");
	else if (extenstion == woff || extenstion == woff2 || extenstion == ttf)
		return ("font/" + extenstion);
	else if (extenstion == eot)
		return ("application/vnd.ms-fontobject");
	else if (extenstion == mp4 || extenstion == webm)
		return ("video/" + extenstion);
	return ("application/octet-stream");
}

bool    Response::IsFileComplete() const { return bytes_sent >= file_size; }
size_t  Response::GetFileSize() const { return file_size; }
size_t  Response::GetBytesSent() const { return bytes_sent; }

bool Response::IsLargeFileResponse() const
{
    const size_t LARGE_FILE_THRESHOLD = 1024 * 1024; // 1MB
    return is_large_file_response && file_size > LARGE_FILE_THRESHOLD;
}

std::string Response::GetFileChunk(size_t file_offset, size_t chunk_size)
{
    if (!is_large_file_response || !file_stream || file_offset >= file_size)
        return "";

    file_stream->clear();
    file_stream->seekg(file_offset, std::ios::beg);
    if (!(*file_stream))
        return "";

    size_t remaining = file_size - file_offset;
    size_t read_size = (remaining > chunk_size) ? chunk_size : remaining;
    std::vector<char> buffer(read_size);

    file_stream->read(&buffer[0], read_size);
    size_t bytes_read = file_stream->gcount();

    if (bytes_read > 0)
        return std::string(&buffer[0], bytes_read);

    return "";
}

void    Response::BuildGetResponse(void)
{
    std::ostringstream  oss;
    oss << getState();

    struct stat info;
    stat(path.c_str(), &info);
    file_size = info.st_size;

    std::ostringstream  os;
    os << file_size;

    responseBody = Request.getVersion() + " " + oss.str() + " " + getStrState() + "\r\n";
    responseBody += "Content-Type: " + MIME_Type() + "\r\n";
    responseBody += "Content-Length: " + os.str() + "\r\n";
    addCookiesToHeaders();
    responseBody += "Connection: close\r\n\r\n";

    const size_t LARGE_FILE_THRESHOLD = 1024 * 1024; // 1MB
    if (file_size > LARGE_FILE_THRESHOLD)
    {
        is_large_file_response = true;
        bytes_sent = 0;
        
        if (file_stream)
        {
            file_stream->close();
            delete file_stream;
        }
        
        file_stream = new std::ifstream(path.c_str(), std::ios::in | std::ios::binary);
        
        if (!file_stream || !file_stream->is_open())
        {
            is_large_file_response = false;
            ReadEntireFileIntoResponse();
        }
    }
    else
        ReadEntireFileIntoResponse();
}

void Response::ReadEntireFileIntoResponse()
{
    std::ifstream fd(path.c_str(), std::ios::in | std::ios::binary);
    if (fd)
    {
        std::ostringstream body;
        body << fd.rdbuf();
        responseBody += body.str();
        fd.close();
    }
    else
    {
        SetState(Internal_Server_Error);
        ResponseWithError(NONE);
    }
}

bool    Response::ReturnDirective(void)
{
	std::ostringstream	statuscode;
	std::string         redirecturl;
	int					code;
	std::string			body;

	if (location)
	{
		if (location->getReturnDirective().enabled == false)
			return (true);

		statuscode << location->getReturnDirective().status_code;
		redirecturl = location->getReturnDirective().target;
		code = location->getReturnDirective().status_code;
		SetState(code);
		body = location->getReturnDirective().target;
	}
	else
	{
		if (ServerBlock.getReturnDirective().enabled == false)
			return (true);

		statuscode << ServerBlock.getReturnDirective().status_code;
		redirecturl = ServerBlock.getReturnDirective().target;
		code = ServerBlock.getReturnDirective().status_code;
		body = ServerBlock.getReturnDirective().target;
		SetState(code);
	}

	if (code == 301 || code == 302 || code == 303
		|| code == 307 || code == 308)
	{
		responseBody = Request.getVersion() + " " + statuscode.str() + " " + getStrState() + "\r\n";
		responseBody += "Location: " + redirecturl + "\r\n";
		responseBody += "Content-Length: 0\r\n";
		responseBody += "Connection: close\r\n\r\n";
	}

	else
	{
		std::ostringstream  os;
		os << body.size();

		responseBody = Request.getVersion() + " " + statuscode.str() + " " + getStrState() + "\r\n";
		responseBody += "Content-Length: " + os.str() + "\r\n";
		responseBody += "Connection: close\r\n\r\n";
		responseBody += body;
	}

	return (false);
}

void    Response::GetPageResponse(void)
{
	struct stat info;

	if (stat(path.c_str(), &info) == -1)
		return (SetState(Not_Found), ResponseWithError(NONE));

	if (!S_ISREG(info.st_mode))
		if (S_ISDIR(info.st_mode))
			return (SearchForIndex());

	if (access(path.c_str(), R_OK) != 0)
		return (SetState(Forbidden), ResponseWithError(NONE));

	BuildGetResponse();
}

void    Response::BuildDeleteResponse(void)
{
	if (unlink(path.c_str()) == -1)
		return (SetState(Internal_Server_Error), ResponseWithError(NONE));

	responseBody = Request.getVersion() + " 204 No Content\r\n";
	responseBody += "Content-Length: 0\r\n";
	responseBody += "Connection: close\r\n\r\n";
}

void    Response::DeleteContentResponse(void)
{
	struct stat info;

	if (stat(path.c_str(), &info) == -1)
	{
		if (errno == ENOENT)
			return (SetState(Not_Found), ResponseWithError(NONE));
		return (SetState(Internal_Server_Error), ResponseWithError(NONE));
	}

	size_t	pos = path.rfind('/');

	if (S_ISDIR(info.st_mode) || pos == std::string::npos)
		return (SetState(Forbidden), ResponseWithError(NONE));

	std::string	dir_path = path.substr(0, pos);

	if (access(dir_path.c_str(), W_OK | X_OK) != 0)
		return (SetState(Forbidden), ResponseWithError(NONE));

	BuildDeleteResponse();
}

void    Response::BuildPostResponse(void)
{
	std::ostringstream  os;
	std::ostringstream  oss;

	os << getState();
	std::string body = "<html><body><h1>Upload Successful</h1></body></html>";
	oss << body.size();
	std::string headers = Request.getVersion() + " " + os.str() + " " + getStrState() + "\r\n";
	headers += "Content-Type: text/html\r\n";
	headers += "Content-Length: " + oss.str() + "\r\n";
	headers += "Connection: close\r\n\r\n";

	responseBody = headers + body;
}

bool	Response::MultiPart(std::string body)
{
	struct stat	info;
	std::string	filename;
	size_t pos = body.find("Content-Disposition:");
	ignore = false;

	if (pos != std::string::npos)
	{
		size_t end = body.find('\n', pos);
		if (end != std::string::npos)
		{
			std::string line = body.substr(pos, end - pos);
			if ((pos = line.find("filename=")) != std::string::npos)
			{
				size_t start = line.find('"', pos);
				size_t finish = line.find('"', start + 1);
				if (start != std::string::npos && finish != std::string::npos)
					filename = line.substr(start + 1, finish - (start + 1));
			}
			else
				return (ignore = true, true);
		}
	}
	if (filename.empty())
		return (ignore = true, true);

	if (!isSafeUploadFilename(filename))
		return (SetState(Bad_Request), ResponseWithError(NONE), false);

	size_t clrf = body.find("\r\n\r\n");
	if (clrf != std::string::npos)
		Body += body.substr(clrf + 4);

	std::string upload_path = path;
	if (location && !location->getUploadStore().empty())
	{
		upload_path = location->getUploadStore();
		if (upload_path[upload_path.size() - 1] != '/')
			upload_path += "/";
	}
	else
		return (SetState(Forbidden), ResponseWithError(NONE), false);

	if (stat(upload_path.c_str(), &info) == -1 || !S_ISDIR(info.st_mode)
		|| access(upload_path.c_str(), W_OK) == -1)
		return (SetState(Internal_Server_Error), ResponseWithError(NONE), false);

	SetPath(upload_path + filename);

	return true;
}

bool    Response::Chunked(void)
{
	struct stat info;

	Body = Request.getBufferBody();

	std::string upload_path = path;

	if (location && !location->getUploadStore().empty())
	{
		upload_path = location->getUploadStore();
		if (upload_path[upload_path.size() - 1] != '/')
			upload_path += "/";
	}
	else
		return (SetState(Forbidden), ResponseWithError(NONE), false);

	if (stat(upload_path.c_str(), &info) == -1)
		return (SetState(Internal_Server_Error), ResponseWithError(NONE), false);

	size_t	pos = path.find_last_of("/");

	upload_path += path.substr(pos + 1);
	SetPath(upload_path);

	if (upload_path[upload_path.size() - 1] == '/')
		return (SetState(Bad_Request), ResponseWithError(NONE), false);

	if (stat(upload_path.c_str(), &info) == -1)
		return (SetState(Created), true);

	if (S_ISDIR(info.st_mode))
		return (SetState(Conflict), ResponseWithError(NONE), false);

	if (access(upload_path.c_str(), W_OK) == -1)
		return (SetState(Forbidden), ResponseWithError(NONE), false);

	return (true);
}

void    Response::PostContentResponse(void)
{
	std::string ContentType = Request.getHeaderValue("content-type");
	size_t      multipart = ContentType.find("multipart/form-data");

	if (multipart != std::string::npos)
	{
		std::vector<std::string>    body = Request.getMultipartBuferBody();
		for (size_t i(0); i < body.size(); i++)
		{
			std::string old_path = getPath();
			if (MultiPart(body[i]) == false)
				return ;
			
			if (ignore)
				continue ;

			std::ofstream	fd(path.c_str());

			if (fd)
			{
				fd << Body;
				fd.close();
			}

			SetPath(old_path);
			Body = "";
		}
		BuildPostResponse();
		return ;
	}
	else
	{
		if (Chunked() == false)
			return ;
	}

	std::ofstream	fd(path.c_str());

	if (fd)
	{
		fd << Body;
		fd.close();
	}

	BuildPostResponse();
}

void    Response::ResponseWithOk(void)
{
	std::string Method = Request.getMethod();

	if (Method == GET)
		GetPageResponse();

	else if (Method == POST)
		PostContentResponse();

	else if (Method == DELETE)
		DeleteContentResponse();

	else
		return (SetState(Not_Implemented), ResponseWithError(NONE));
}

std::string Response::DefaultForMatchError(void)
{
	int error = getState();

	if (error == Moved_Permanently)
		return "./error_pages/Moved_Permanently.html";
	else if (error == Bad_Request)
		return "./error_pages/Bad_Request.html";
	else if (error == Forbidden)
		return "./error_pages/Forbidden.html";
	else if (error == Not_Found)
		return "./error_pages/Not_Found.html";
	else if (error == Method_Not_Allowed)
		return "./error_pages/Method_Not_Allowed.html";
	else if (error == Conflict)
		return "./error_pages/Conflict.html";
	else if (error == Length_Required)
		return "./error_pages/Length_Required.html";
	else if (error == Content_Too_Large)
		return "./error_pages/Content_Too_Large.html";
	else if (error == Request_Timeout)
		return ("./error_pages/Request_Timeout.html");
	else if (error == URI_Too_Long)
		return "./error_pages/URI_Too_Long.html";
	else if (error == Unsupported_Media_Type)
		return "./error_pages/Unsupported_Media_Type.html";
	else if (error == Request_Header_Fields_Too_Large)
		return "./error_pages/Request_Header_Fields_Too_Large.html";
	else if (error == Internal_Server_Error)
		return "./error_pages/Internal_Server_Error.html";
	else if (error == Not_Implemented)
		return "./error_pages/Not_Implemented.html";
	else if (error == Bad_Gateway)
		return "./error_pages/Bad_Gateway.html";
	else if (error == Gateway_Timeout)
		return "./error_pages/Gateway_Timeout.html";
	else
		return "./error_pages/HTTP_Version_Not_Supported.html";
}

void    Response::ResponseWithError(int serve)
{
	if (Request.getVersion().empty())
		Request.setVersion("HTTP/1.1");

	if (serve == DEFAULT)
	{
		SetPath(DefaultForMatchError());
		BuildGetResponse();
		return ;
	}

	struct stat info;
	std::string error_page = ServerBlock.getErrorPage(getState());

	if (!error_page.empty())
	{
		SetPath(error_page);
		Request.setUri(path);
		SetLocation(NULL);
		Request.FindMatchLocation();
		SetLocation(Request.getMatchLocation());

		GetFullPath(path);

		if (stat(path.c_str(), &info) != 0)
			return (SetState(Not_Found), ResponseWithError(DEFAULT));

		else if (S_ISDIR(info.st_mode) || access(path.c_str(), R_OK) != 0)
			return (SetState(Internal_Server_Error), ResponseWithError(DEFAULT));
	}
	else
		return (ResponseWithError(DEFAULT));

	BuildGetResponse();
}

bool	Response::shouldExecuteCgi(ParseRequest& request)
{
	Location* location = this->location;
	if (!location)
		return false;

	bool is_cgi = location->isCgiRequest(request.getUri());
	if (is_cgi && request.getMethod() == "DELETE")
		return false;

	return is_cgi;
}

void    Response::StartForResponse(ParseRequest request, int fd_client)
{
	SetRequest(request);
	SetBlockServer(*(request.getBlockServer()));
	SetState(request.getErrorNumber());
	SetPath(request.getUri());
	SetLocation(request.getMatchLocation());
	this->fd_client = fd_client;

	if (hasParentDirectorySegment(request.getUri()))
		return (SetState(Forbidden), ResponseWithError(NONE));

	GetFullPath(path);

	if (ReturnDirective() == false)
		return ;

	if (shouldExecuteCgi(Request))
	{
		try {
			handleCgiRequest(Request);
		} catch (...) {
			SetState(Internal_Server_Error);
			ResponseWithError(NONE);
		}
		return;
	}

	else if (getState() != OK)
		ResponseWithError(NONE);

	else
		ResponseWithOk();
}

void	Response::handleCgiRequest(ParseRequest& request)
{
	try
	{
		Location* location = this->location;
		if (!location)
			return (SetState(Not_Found), ResponseWithError(NONE));
		
		if (!location->isCgiRequest(request.getUri()))
			return (SetState(Not_Found), ResponseWithError(NONE));

		std::string script_path = path;

		if (script_path.empty() || access(script_path.c_str(), F_OK | R_OK) != 0)
			return (SetState(Not_Found), ResponseWithError(NONE));

		std::string file_extension = getFileExtension(request.getUri());
		std::string interpreter = location->getCgiInterpreter(file_extension);

		if (interpreter.empty())
			return (SetState(Internal_Server_Error), ResponseWithError(NONE));

		Cgi cgi(script_path, interpreter);
		CgiResult result = cgi.execute(request);

		if (result.success)
			sendCgiResponse(result);
		else
			return (SetState(result.status_code), ResponseWithError(NONE));
	}
	catch (const std::exception& e)
	{
		SetState(500);
		ResponseWithError(NONE);
	}
}

void	Response::sendCgiResponse(const CgiResult& cgi_result)
{
	responseBody = Request.getVersion() + " " + intToString(cgi_result.status_code) + " OK\r\n";
	
	responseBody += cgi_result.headers;
	
	for (std::vector<std::string>::iterator it = cookies_to_set.begin(); it != cookies_to_set.end(); ++it)
		responseBody += *it + "\r\n";
	
	responseBody += "Content-Length: " + intToString(cgi_result.body.length()) + "\r\n";
	responseBody += "Connection: close\r\n";
	responseBody += "\r\n";
	responseBody += cgi_result.body;
}

std::string	Response::getFileExtension(const std::string& uri)
{
	size_t dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos) return "";
	return uri.substr(dot_pos);
}

int     Response::getState() const {return state;}

std::string	Response::GetResponseBody() const {return responseBody;}

std::string Response::getPath() {return path;}

bool	Response::getBuildRes() {return buildRes;}

size_t	Response::getBytes() {return bytes;}

void	Response::SetBuildRes(bool buildres) {this->buildRes = buildres;}

void	Response::SetBytes(size_t bytes) {this->bytes = bytes;}

void    Response::SetRequest(ParseRequest Request){this->Request = Request;}

void    Response::SetBlockServer(Server BlockServer){this->ServerBlock = BlockServer;}

void    Response::SetLocation(Location* location){this->location = location;}

void    Response::SetState(int state){this->state = state;}

void    Response::SetPath(std::string path){this->path = path;}

void	Response::setCookie(const std::string& name, const std::string& value, 
						  const std::string& path, const std::string& expires, bool http_only, int max_age_seconds)
{
	std::string cookie = "Set-Cookie: " + name + "=" + value;
	if (!path.empty())
		cookie += "; Path=" + path;
	if (max_age_seconds > 0)
	{
		cookie += "; Max-Age=" + intToString(max_age_seconds);
		time_t now = std::time(NULL);
		time_t exp = now + max_age_seconds;
		char buf[64];
		std::tm* gmt = std::gmtime(&exp);
		if (gmt)
		{
			std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
			cookie += "; Expires=";
			cookie += buf;
		}
	}
	else if (!expires.empty())
		cookie += "; Expires=" + expires;
	if (http_only)
		cookie += "; HttpOnly";
	cookie += "; SameSite=Strict";
	cookies_to_set.push_back(cookie);
}

void	Response::addCookiesToHeaders()
{
	for (std::vector<std::string>::iterator it = cookies_to_set.begin(); it != cookies_to_set.end(); ++it)
		responseBody += *it + "\r\n";
}
void	Response::setSessionManager(SessionManager* sm)
{
	sessionManager = sm;
}
