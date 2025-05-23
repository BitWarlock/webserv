/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/23 17:03:28 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "SetupServers.hpp"

SetupServers::SetupServers(const Config& config) : config(config){}

SetupServers::~SetupServers(){}

bool    SetupServers::CheckPortIp(Server& server, int pos_server)
{
    std::vector<Server*>   servers = config.getServers();
    for (int i(0); i < pos_server; i++)
    {
        if (servers[i]->getListen() == server.getListen())
            return false;
    }
    return true;
}

void    SetupServers::FlagSharedPortIp(void)
{
    const std::vector<Server*>&   serv = config.getServers();
    std::vector<Server*>&   servers = const_cast<std::vector<Server*>&> (serv);
    for (int i(0); i < servers.size(); i++)
    {
        bool    flag = CheckPortIp(*(servers[i]), i);
        servers[i]->setDeja_vu(flag);
    }
}

void    SetupServers::StartSetup(void)
{
    std::vector<Server*> servers = config.getServers();
}

void    SetupServers::bindServer(const Server& server)
{
    int fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_server < 0)
    {
        std::cerr << RED"Error: socket() function failed to create endpoint.\n" << RESET;
        throw 1;
    }
    struct sockaddr_in  add_server;
}
