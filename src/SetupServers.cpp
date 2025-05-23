/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/23 20:40:12 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "SetupServers.hpp"

SetupServers::SetupServers(const Config& config) : config(config){
    pos = 0;
    for (size_t i(0); i < MAX_SOCKET; i++)
        fd_sockets[i] = 0;
}

SetupServers::~SetupServers(){}

bool    SetupServers::CheckPortIp(const std::string& host, const std::string& port, int pos_server)
{
    std::vector<Server*>   servers = config.getServers();
    for (size_t i(0); i < pos_server; i++)
    {
        for (size_t s(0); s < servers[i].getListen().size(); s++)
        {
            if (server[i]->getListen()[s].first == host
					&& server[i]->getListen()[s].second == port)
            {
                server[i]->getListen()[s].second += "T";
                return ;
            }
        }
    }
}

void    SetupServers::FlagSharedPortIp(void)
{
    const std::vector<Server*>&   serv = config.getServers();
    std::vector<Server*>&   servers = const_cast<std::vector<Server*>&> (serv);
    for (size_t i(0); i < servers.size(); i++)
    {
        for (size_t s(0); s < servers[i].getListen().size(); s++)
        {
            CheckPortIp(servers[i]->getListen()[s].first, 
                servers[i]->getListen()[s].first, i);
        }
    }
}

void    SetupServers::CreateSocket(Server& server)
{
    for (size_t i(0); i < server.getListen().size(); i++)
    {
        std::string port = server.getListen()[i].second;
        if (port[port.size() - 1] != 'T')
        {
            int fd_server = socket(AF_INET, SOCK_STREAM, 0);
            if (fd_server < 0)
            {
                std::cerr << RED"Error: socket() function failed to create endpoint.\n" << RESET;
                throw 1;
            }
            this->fd_sockets[pos] = fd_server;
            advance();
        }
    }
}

void    SetupServers::setAddrForBound(std::string host, std::string port)
{

    uint16_t    port_number = atoi(port_str.c_str());
    struct sockaddr_in  add_server;
    add_server.sin_family = AF_INET;
    add_server.sin_port = htons(port_number);
    if (inet_pton(AF_INET, host, &add_server.sin_addr) <= 0)
    {
        std::cerr << "Invalid Ip\n";
        return ;
    }
}

void    SetupServers::Binding(Server& server, int index)
{
    for (size_t i(0); i <= pos; i++)
    {
        std::string host = server.getListen()[i].first;
        std::string port = server.getListen()[i].second;
        if (port[port.size() - 1] != 'T')
        {
            setAddrForBound(host, port);
            if (bind(fd_sockets[index],  (struct sockaddr*) &add_server, sizeof(add_server)))
                std::cerr << "Failed bind() to bound Ip and Port\n";
        }
        else
            server.getListen()[i].second.erase(port.size() - 1);
    }
}

void    SetupServers::StartSetup(void)
{
    static size_t   index;
    std::vector<Server*> servers = config.getServers();
    for (int i(0); i < servers.size(); i++)
    {
        CreateSocket(servers[i]);
        Binding(servers[i]);
        while (index <= pos)
        {
            if (listen(fd_sockets, SOMAXCONN))
                std::cerr << "Failed listen() to listen for connections on a socket\n";
            index++;
        }
    }
}

void    advance(void) {this->pos++;}
