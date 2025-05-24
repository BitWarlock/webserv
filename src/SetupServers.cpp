/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/24 21:39:33 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../inc/SetupServers.hpp"

SetupServers::SetupServers(Config& config) : config(config), sock_number(0){
    for (size_t i(0); i < MAX_SOCKET; i++)
        fd_sockets[i] = 0;
    this->StartSetup();
}

SetupServers::~SetupServers(){
    for(size_t i(0); i < sock_number; i++)
        close(fd_sockets[i]);
}

void    SetupServers::CheckPortIp(const std::string& host, const std::string& port, size_t pos_server)
{
    std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

    for (size_t i(0); i < pos_server; i++)
    {
        for (size_t s(0); s < servers[i]->getListen().size(); s++)
        {
            if (servers[i]->getListen()[s].first == host
					&& servers[i]->getListen()[s].second == port)
            {
                std::string&    _port = const_cast<std::string&> (port);
                _port += "T";
                return ;
            }
        }
    }
}

void    SetupServers::FlagSharedPortIp(void)
{
    std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

    for (size_t i(0); i < servers.size(); i++)
    {
        for (size_t s(0); s < servers[i]->getListen().size(); s++)
        {
            const std::string &host = servers[i]->getListen()[s].first;
            const std::string &port = servers[i]->getListen()[s].second;

            CheckPortIp(host, port, i);
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
            this->fd_sockets[sock_number] = fd_server;
            advance();
        }
    }
}

void    SetupServers::setAddrForBound(std::string& host, std::string& port, struct sockaddr_in& add_server)
{

    uint16_t            port_number = atoi(port.c_str());
    
    bzero(&add_server, sizeof(add_server));
    add_server.sin_family = AF_INET;
    add_server.sin_port = htons(port_number);
    add_server.sin_addr.s_addr = inet_addr(host.c_str());
}

void    SetupServers::Binding(Server& server, size_t index)
{
    for (size_t i(0); i < server.getListen().size(); i++)
    {
        struct sockaddr_in  add_server;
        std::string         host = server.getListen()[i].first;
        std::string         port = server.getListen()[i].second;
        size_t              s(0);

        if (port[port.size() - 1] != 'T')
        {
            setAddrForBound(host, port, add_server);
            if (bind(fd_sockets[s + index],  (struct sockaddr*) &add_server, sizeof(add_server)))
                std::cerr << RED"Failed bind() to bound Ip and Port\n" << RESET;
            s++;
        }
        else
        {
            std::string& str = const_cast<std::string&> ((server.getListen()[i].second));
            str.erase(port.size() - 1);
        }
    }
}

void    SetupServers::StartSetup(void)
{
    static size_t         index(0);
    std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

    FlagSharedPortIp();
    for (size_t i(0); i < servers.size(); i++)
    {
        CreateSocket(*(servers[i]));
        Binding(*(servers[i]), index);
        while (index < sock_number)
        {
            if (listen(fd_sockets[index], SOMAXCONN))
                std::cerr << "Failed listen() to listen for connections on a socket\n";
            index++;
        }
    }
}

void    SetupServers::Run()
{}

void    SetupServers::advance(void) {this->sock_number++;}
