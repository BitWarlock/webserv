#include "../inc/Location.hpp"
#include "../inc/Server.hpp"

Location::Location(){}

Location::Location(const std::string& path) 
	: path(path), autoindex(false), client_max_body_size(0) {return_d.enabled = false;}

Location::~Location() {}

	// Setters
void Location::addAllowedMethod(const std::string& method) {allowed_methods.push_back(method);}

void Location::setRoot(const std::string& root) {this->root = root;}

void Location::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Location::addIndex(const std::string& index) {this->index.push_back(index);}

void Location::setClientBodyLimit(size_t limit) {client_max_body_size = limit;}

void Location::setCgiExtension(const std::string& ext,
		const std::string& interpreter) {cgi_info[ext] = interpreter;}

void Location::setReturn(const std::string& return_url) {this->return_url = return_url;}

void Location::setUploadStore(const std::string& path) {upload_store = path;}

void Location::addReturnDirective(const ReturnDirective rd) {return_d = rd;}

void Location::inheritFrom(const Server* server)
{
	// if (root.empty())
	// 	root = server->getRoot();
	if (client_max_body_size == 0)
		client_max_body_size = server->getClientBodyLimit();
	if (index.empty())
		index = server->getIndex();
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

const std::string& Location::getCgiInfo(const std::string& ext) const
{
	static const std::string empty;
	std::map<std::string, std::string>::const_iterator it = cgi_info.find(ext);
	return it != cgi_info.end() ? it->second : empty;
}
