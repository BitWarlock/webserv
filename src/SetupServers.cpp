/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/23 19:28:57 by hael-ghd         ###   ########.fr       */
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
    std::vector<std::pair<std::string, std::string>>    listen = servers[i]->getListen();
    for (int i(0); i < pos_server; i++)
    {
        for (int s(0); s < listen.size(); s++)
            if ( == server.getListen())
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

void    SetupServers::CreateSocket(bool flag)
{
    if (flag)
    {
        int fd_server = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_server < 0)
        {
            std::cerr << RED"Error: socket() function failed to create endpoint.\n" << RESET;
            throw 1;
        }
    }
    return -1;
}

void    SetupServers::Binding(const Server& server, int index)
{
    std::vector<std::pair<std::string, std::string>>    host_port;
    host_port = server.getListen();
    std::string port_str = host_port.second;
    uint16_t    port = atoi(port_str.c_str());
    struct sockaddr_in  add_server;
    add_server.sin_family = AF_INET;
    add_server.sin_port = htons(port);
    if (inet_pton(AF_INET, host_port.first, &add_server.sin_addr) <= 0)
    {
        std::cerr << "Invalid Ip\n";
        return ;
    }
    if (bind(fd_sockets[index],  (struct sockaddr*) &add_server, sizeof(add_server)))
        std::cerr << "Failed bind() to bound Ip and Port\n";
}

void    SetupServers::StartSetup(void)
{
    std::vector<Server*> servers = config.getServers();
    for (int i(0); i < servers.size(); i++)
    {
        CreateSocket(servers[i]->getDeja_vu());
        if (fd_sockets[i] != -1)
            this->Binding(*(servers[i]), i);
        if (listen(fd_sockets, SOMAXCONN))
            std::cerr << "Failed listen() to listen for connections on a socket\n";
    }
}

