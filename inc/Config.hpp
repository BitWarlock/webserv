#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>
#include "Server.hpp"

class Config
{
	public:
		Config();
		~Config();

		void						addServer(Server* server);
		const std::vector<Server*>&	getServers() const;
		std::vector<Server*>&		getServers();
		const Server*				getServerName(const std::string& server_name,
												const std::string& port) const;
		const Server*				getServer(const std::string& host,
												const std::string& port) const;

	private:
		std::vector<Server*> servers;
};

#endif
