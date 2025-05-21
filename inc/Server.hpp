#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>
#include "Location.hpp"

class Server
{
	public:
		Server();
		~Server();

		// Setters
		void addAllowedMethod(const std::string& method);
		void setListen(std::pair<std::string, std::string>);
		void addServerName(const std::string& name);
		void addErrorPage(int code, const std::string& path);
		void addLocation(Location* location);
		void setRoot(const std::string& root);
		void setClientBodyLimit(size_t limit);
		void setAutoindex(bool autoindex);
		void addIndex(const std::string& index);
		void addReturnDirective(const ReturnDirective);

		// Getters
		const std::vector<std::pair
	 	  <std::string, std::string> > 	getListen() const;
		const ReturnDirective			getReturnDirective() const;
		const std::vector<std::string>& getAllowedMethods() const;
		const std::vector<std::string>& getServerNames() const;
		const std::string&				getErrorPage(int code) const;
		const std::vector<Location*>&	getLocations() const;
		const std::string&				getRoot() const;
		size_t							getClientBodyLimit() const;
		bool							getAutoindex() const;
		const std::vector<std::string>& getIndex() const;

	private:
		ReturnDirective				return_d;
		std::vector
			<std::pair
			<std::string,
			std::string> >			listen;
		std::vector<std::string>	allowed_methods;
		std::vector<std::string>	server_names;
		std::map<int, std::string>	error_pages;
		std::vector<Location*>		locations;
		std::string					root;
		size_t						client_body_limit;
		bool						autoindex;
		std::vector<std::string>	index;
};

#endif
