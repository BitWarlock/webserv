#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <vector>
#include <map>
#include <string>

class Server;

class Location
{
	public:
		Location(const std::string& path);
		~Location();

		// Setters
		void addAllowedMethod(const std::string& method);
		void setRoot(const std::string& root);
		void setAutoindex(bool autoindex);
		void addIndex(const std::string& index);
		void setClientBodyLimit(size_t limit);
		void setCgiExtension(const std::string& ext, const std::string& interpreter);
		void setReturn(const std::string& return_url);
		void setUploadStore(const std::string& path);
		void inheritFrom(const Server* server);

		// Getters
		const std::string&				getPath() const;
		const std::vector<std::string>& getAllowedMethods() const;
		const std::string&				getRoot() const;
		bool							getAutoindex() const;
		const std::vector<std::string>& getIndex() const;
		size_t							getClientBodyLimit() const;
		const std::string&				getCgiInfo(const std::string& ext) const;
		const std::string&				getReturn() const;
		bool							getIsRedirect() const;
		const std::string&				getUploadStore() const;
		bool							getUploadEnabled() const;

	private:
		std::string					path;
		std::vector<std::string>	allowed_methods;
		std::string					root;
		bool						autoindex;
		std::vector<std::string>	index;
		size_t						client_body_limit;
		std::map<std::string,
			std::string>			cgi_info;
		std::string					return_url;
		bool						is_redirect;
		std::string					upload_store;
		bool						upload_enabled;
};

#endif
