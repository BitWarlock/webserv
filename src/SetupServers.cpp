/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:57:28 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/02 21:52:05 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "../inc/SetupServers.hpp"

SetupServers::SetupServers(Config& config) : config(config), sock_number(0){
    this->StartSetup();
}

SetupServers::~SetupServers(){
    for (size_t i(0); i < sock_number; i++)
        close(fd_sockets[i]);
}

void    SetupServers::CheckPortIp(const std::string& host, const std::string& port, size_t pos_server)
{
    std::vector<Server*>& servers = const_cast<std::vector<Server*>&> (config.getServers());

    for (size_t i(0); i < pos_server; i++)
    {
        for (size_t s(0); s < servers[i]->getListen().size(); s++)
        {
            if ((servers[i]->getListen()[s].first == host || host == "0.0.0.0")
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
                std::cerr << YLW"Warning:make socket() function failed to create endpoint "
                << "for " << server.getListen()[i].first << ":"<< port << ".\n" << RESET;
                throw -1;
            }
            this->fd_sockets.push_back(fd_server);
            advance();
        }
    }
}

void    SetupServers::setAddrForBound(std::string& host, std::string& port, struct sockaddr_in& add_server)
{

    uint16_t            port_number = atoi(port.c_str());
    
    std::memset(&add_server, 0, sizeof(add_server));
    add_server.sin_family = AF_INET;
    add_server.sin_port = htons(port_number);
    add_server.sin_addr.s_addr = inet_addr(host.c_str());
}

void    SetupServers::Binding(Server& server, size_t index)
{
    size_t              s(0);

    for (size_t i(0); i < server.getListen().size(); i++)
    {
        struct sockaddr_in  add_server;
        std::string         host = server.getListen()[i].first;
        std::string         port = server.getListen()[i].second;

        if (port[port.size() - 1] != 'T')
        {
            setAddrForBound(host, port, add_server);
            if (bind(fd_sockets[s + index],  (struct sockaddr*) &add_server, sizeof(add_server)))
            {
                std::cerr << YLW"Warning: bind() function failed to bound "
                << host << ":"<< port << ".\n" << RESET;
                throw -1;
            }
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
        try {
            CreateSocket(*(servers[i]));
            Binding(*(servers[i]), index);
            while (index < sock_number)
            {
                if (listen(fd_sockets[index], SOMAXCONN) < 0)
                    std::cerr << YLW"Warning: listen() function failed to listen.\n"<< RESET;
                index++;
            }
        }
        catch (...){}
    }
}

void    SetupServers::Run()
{
    int                 fd_epoll;
    struct epoll_event  events[MAX_SOCKET];
    

    fd_epoll = epoll_create(MAX_SOCKET);
    if (fd_epoll < -1)
    {
        std::cerr << YLW"Warning: epoll_create() failed to create epoll instance"
        << ".\n" << RESET;
        throw -1;
    }
    for (size_t i(0); i <= sock_number; i++)
    {
        if (!epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_sockets[i], NULL))
        {
            std::cerr << YLW"Warning: epoll_ctl() failed to monitor a socket"
            << ".\n" << RESET;
            throw -1;
        }
    }
    while (1)
    {
        int event = epoll_wait(fd_epoll, events, MAX_SOCKET, INFINITE);
        if (event < 0)
        {
            std::cerr << YLW"Warning: epoll_wait() failed to waits for events"
            << ".\n" << RESET;
            throw -1;
        }
        else if (event == 0)
            continue ;
        else
        {
            for (size_t i(0); i < event; i++)
            {
                uint32_t    flag = events[i].events;
                if (flag == EPOLLIN)
                {
                    if (fd_sockets.end() != find(fd_sockets.begin(), fd_sockets.end(), events[i].data.fd))
                    {
                        int fd = accept(events[i].data.fd, NULL, 0);
                        fcntl(fd, F_SETFL, O_NONBLOCK);
                        fd_clients.push_back(fd);
                        if (!epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_clients.back(), NULL))
                        {
                            std::cerr << YLW"Warning: epoll_ctl() failed to monitor a socket"
                            << ".\n" << RESET;
                            throw -1;
                        }
                    }
                    else
                        recv(events[i].data.fd);
                }
                else if (flag == EPOLLOUT)
                {
                    
                }
            }
        }
    }
}

void    SetupServers::advance(void) {this->sock_number++;}
