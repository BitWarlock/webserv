#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include "ParseRequest.hpp"
#include "Server.hpp"
#include "Location.hpp"

class HttpResponse
{
	public:
		HttpResponse();
		~HttpResponse();

		void generateResponse(ParseRequest& request, const Server& server);
		void generateErrorResponse(int error_code, const Server& server);
		void generateFileResponse(const std::string& file_path, const Server& server);
		void generateAutoindexResponse(const std::string& dir_path, const std::string& uri);
		void generateRedirectResponse(const std::string& location, int code = 301);

		void setStatus(int code, const std::string& message = "");
		void setHeader(const std::string& name, const std::string& value);
		void setBody(const std::string& content);
		void appendBody(const std::string& content);

		std::string					serialize() const;
		std::vector<std::string>	serializeChunked() const;

		// Getters
		int					getStatusCode() const;
		const std::string&	getStatusMessage() const;
		const std::map
			<std::string,
			std::string>&	getHeaders() const;
		const std::string&	getBody() const;
		bool				isKeepAlive() const;

	private:
		std::map
			<std::string,
			std::string>	headers_;
		int					status_code_;
		std::string			status_message_;
		std::string			body_;
		bool				keep_alive_;

		bool		fileExistsAndReadable(const std::string& path) const;
		size_t		getFileSize(const std::string& path) const;
		std::string	readFileContent(const std::string& path) const;
		std::string	getStatusMessageForCode(int code) const;
		void		setDefaultHeaders();
		void		setContentLength();
		void		setContentType(const std::string& type);
		void		setConnection(bool keep_alive);
		void		setDateHeader();
		void		setServerHeader();

		const Location*	findMatchingLocation(ParseRequest& request, const Server& server) const;
		std::string		resolveFilePath(ParseRequest& request, const Location* location) const;
		std::string		determineContentType(const std::string& file_path) const;
};

#endif
