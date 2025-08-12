#include "../inc/Server.hpp"

Server::Server() 
	: client_max_body_size(0), autoindex(false)
{
	return_d.enabled = false;
}

Server::Server(const Server &other)  :
	return_d(other.return_d),
	listen(other.listen),
	allowed_methods(other.allowed_methods),
	server_names(other.server_names),
	error_pages(other.error_pages),
	root(other.root),
	client_max_body_size(other.client_max_body_size),
	autoindex(other.autoindex),
	index(other.index)
{
	for (size_t i = 0; i < other.locations.size(); i++)
		locations.push_back(new Location(*other.locations[i]));
}

Server	&Server::operator=(const Server &other)
{
	if (this != &other)
	{
		for (size_t i = 0; i < locations.size(); i++)
			delete locations[i];
		locations.clear();

		return_d = other.return_d;
		listen = other.listen;
		allowed_methods = other.allowed_methods;
		server_names = other.server_names;
		error_pages = other.error_pages;
		root = other.root;
		client_max_body_size = other.client_max_body_size;
		autoindex = other.autoindex;
		index = other.index;

		for (size_t i = 0; i < other.locations.size(); i++)
			locations.push_back(new Location(*other.locations[i]));
	}
	return (*this);
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

void Server::setClientBodyLimit(size_t limit) {client_max_body_size = limit;}

void Server::addClientTimeout(size_t timeout) {client_timeout = timeout;}

void Server::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Server::addIndex(const std::string& index) {this->index.push_back(index);}

// Getters
const std::vector<std::string>& Server::getAllowedMethods() const {return allowed_methods;}

const ReturnDirective	Server::getReturnDirective() const {return return_d;}

const std::vector<std::pair<std::string, std::string> >& Server::getListen() const {return listen;}

std::vector<std::pair<std::string, std::string> >& Server::getListen() {return listen;}

const std::vector<std::string>& Server::getServerNames() const {return server_names;}

const std::vector<Location*>& Server::getLocations() const {return locations;}

const std::string& Server::getRoot() const {return root;}

size_t Server::getClientBodyLimit() const {return client_max_body_size;}

size_t Server::getClientTimeout() const {return client_timeout;}

bool Server::getAutoindex() const {return autoindex;}

const std::vector<std::string>& Server::getIndex() const {return index;}

const std::string& Server::getErrorPage(int code) const
{
	static const std::string empty;
	std::map<int, std::string>::const_iterator it = error_pages.find(code);
	return it != error_pages.end() ? it->second : empty;
}
