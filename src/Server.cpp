#include "../inc/Server.hpp"

Server::Server() 
	: client_body_limit(0), autoindex(false)
{
	listen.push_back(std::make_pair("0.0.0.0", "8000"));
	return_d.enabled = false;
}

Server::~Server()
{
	for (size_t i = 0; i < locations.size(); ++i)
		delete locations[i];
}

// Setters
void Server::addAllowedMethod(const std::string& method) {allowed_methods.push_back(method);}

void Server::setListen(std::pair<std::string, std::string> host_port) {listen.push_back(host_port);}

void Server::addServerName(const std::string& name) {server_names.push_back(name);}

void Server::addErrorPage(int code, const std::string& path) {error_pages[code] = path;}

void Server::addReturnDirective(const ReturnDirective rd) {return_d = rd;}

void Server::addLocation(Location* location) {locations.push_back(location);}

void Server::setRoot(const std::string& root) {this->root = root;}

void Server::setClientBodyLimit(size_t limit) {client_body_limit = limit;}

void Server::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Server::addIndex(const std::string& index) {this->index.push_back(index);}

// Getters
const std::vector<std::string>& Server::getAllowedMethods() const {return allowed_methods;}

const ReturnDirective	Server::getReturnDirective() const {return return_d;}

const std::vector<std::pair<std::string, std::string> > Server::getListen() const {return listen;}

const std::vector<std::string>& Server::getServerNames() const {return server_names;}

const std::vector<Location*>& Server::getLocations() const {return locations;}

const std::string& Server::getRoot() const {return root;}

size_t Server::getClientBodyLimit() const {return client_body_limit;}

bool Server::getAutoindex() const {return autoindex;}

const std::vector<std::string>& Server::getIndex() const {return index;}

const std::string& Server::getErrorPage(int code) const
{
	static const std::string empty;
	std::map<int, std::string>::const_iterator it = error_pages.find(code);
	return it != error_pages.end() ? it->second : empty;
}
