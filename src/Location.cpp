#include "../inc/Location.hpp"
#include "../inc/Server.hpp"
#include <map>

Location::Location(){}

Location::Location(const std::string& path) 
	: path(path), autoindex(false), client_max_body_size(0) {return_d.enabled = false;}

Location::~Location() {}

bool	Location::isCgiRequest(const std::string& uri) const
{
	size_t	dot_pos = uri.find_last_of(".");

	if (dot_pos == std::string::npos)
		return false;

	std::string	extension = uri.substr(dot_pos);
	return (cgi_extensions.find(extension) != cgi_extensions.end());
}
	// Setters
void Location::addAllowedMethod(const std::string& method) {allowed_methods.push_back(method);}

void Location::setRoot(const std::string& root) {this->root = root;}

void Location::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Location::addIndex(const std::string& index) {this->index.push_back(index);}

void Location::setClientBodyLimit(size_t limit) {client_max_body_size = limit;}

void Location::setCgiExtension(const std::string& ext,
		const std::string& interpreter) {cgi_extensions[ext] = interpreter;}

void Location::setReturn(const std::string& return_url) {this->return_url = return_url;}

void Location::setUploadStore(const std::string& path) {upload_store = path;}

void Location::addReturnDirective(const ReturnDirective rd) {return_d = rd;}

void Location::inheritFrom(const Server* server)
{
	if (index.empty())
		index = server->getIndex();

	if (autoindex == false)
		autoindex = server->getAutoindex();
	
	if (client_max_body_size == 0)
		client_max_body_size = server->getClientBodyLimit();
}

// Getters
const ReturnDirective	Location::getReturnDirective() const {return return_d;}

const std::string& Location::getPath() const {return path;}

const std::vector<std::string>& Location::getAllowedMethods() const {return allowed_methods;}

const std::string& Location::getRoot() const {return root;}

bool Location::getAutoindex() const {return autoindex;}

const std::vector<std::string>& Location::getIndex() const {return index;}

size_t Location::getClientBodyLimit() const {return client_max_body_size;}

const std::string& Location::getReturn() const {return return_url;}

const std::string& Location::getUploadStore() const {return upload_store;}

bool Location::getIsRedirect() const {return is_redirect;}

bool Location::getUploadEnabled() const {return upload_enabled;}

std::string Location::getCgiInterpreter(const std::string& ext) const
{
	std::map<std::string, std::string>::const_iterator	it = cgi_extensions.find(ext);
	return (it != cgi_extensions.end() ? it->second : "");
}

const std::string& Location::getCgiInfo(const std::string& ext) const
{
	static std::string empty_string = "";
	std::map<std::string, std::string>::const_iterator it = cgi_extensions.find(ext);
	return (it != cgi_extensions.end() ? it->second : empty_string);
}

const std::map<std::string, std::string>&	Location::getCgiExtensions() const {return cgi_extensions;}
