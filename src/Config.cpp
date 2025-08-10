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

std::vector<Server*>& Config::getServers()
{
	return (servers);
}

const Server* Config::getServer(const std::string& host, const std::string& port) const
{
	std::string new_host = host;
	if (host == "localhost")
		new_host = "127.0.0.1";
	for (size_t i = 0; i < servers.size(); ++i)
	{
		Server	*server = servers[i];
		for (size_t i = 0; i < server->getListen().size(); ++i)
		{
			if ((server->getListen()[i].first == new_host
					&& server->getListen()[i].second == port) || (server->getListen()[i].first == "0.0.0.0"
					&& server->getListen()[i].second == port))
				return (server);
		}
	}
	return (NULL);
}

const Server* Config::getServerName(const std::string& server_name, const std::string& port) const
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		Server	*server = servers[i];
		for (size_t i = 0; i < server->getListen().size(); ++i)
		{
			if (server->getListen()[i].second == port)
			{
				for (size_t i = 0; i < server->getServerNames().size(); ++i)
				{
					if (server->getServerNames()[i] == server_name)
						return (server);
				}
			}
		}
	}
	return (NULL);
}