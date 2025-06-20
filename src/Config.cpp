#include "../inc/Config.hpp"

Config::Config() {}

Config::~Config()
{
	for (size_t i = 0; i < servers.size(); ++i)
		delete servers[i];
	servers.clear();
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
		Server	*server = servers[i];
		for (size_t i = 0; i < server->getListen().size(); ++i)
		{
			if (server->getListen()[i].first == host
					&& server->getListen()[i].second == port)
				return (server);
		}
	}
	return (NULL);
}
