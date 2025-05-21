#include "../inc/Config.hpp"

Config::Config() {}

Config::~Config()
{
	for (size_t i = 0; i < servers.size(); ++i)
		delete servers[i];
}

void Config::addServer(Server* server)
{
	servers.push_back(server);
}

const std::vector<Server*>& Config::getServers() const
{
	return (servers);
}

const Server* Config::getServer(const std::string& host, const std::string& port) const
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (servers[i]->getHost() == host && servers[i]->getPort() == port)
			return servers[i];
	}
	return (NULL);
}
