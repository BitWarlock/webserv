/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:59:30 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/08/08 21:44:49 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SETUPSERVERS_HPP
#define SETUPSERVERS_HPP

#include "WebServ.hpp"
#include "Config.hpp"
#include "SessionManager.hpp"
#include "ParseRequest.hpp"
#include <sys/epoll.h>

#define MAX_SOCKET  1024
#define MAX_EVENTS  1024
#define MAX_REQUEST 1024
#define INFINITE    -1

class SetupServers
{
    public:
        SetupServers(Config& config);
        ~SetupServers();

        void                CheckPortIp(const std::string& host, const std::string& port, size_t pos_server, size_t pos_listen);
        void                FlagSharedPortIp();
        void                CreateSocket(Server& server);
        void                setAddrForBound(std::string& host, std::string& port, struct sockaddr_in &add_server);
        void                Binding(Server& server, size_t index);
        void                CreateEpoll(void);
        void                WaitEpoll(void);
        struct epoll_event  InitEvents(int fd, int event);
        void                AddSocketToEpoll(int fd, int event, int job);
        void                RemoveSocketFromEpoll(int fd, int job);
        void                AcceptConnection(int fd);
        void                EraseFd(int fd);
        void                StartSetup(void);
        void                Run(void);
        void                Advance(void);
        void                Retreat(void);
        void                handleSessionManagement(ParseRequest& request);

        Server              GetBlockServer(int block);
        
    private:
        Config&                 config;
        size_t                  sock_number;
        std::vector<int>        fd_sockets;
        int                     fd_epoll;
        int                     number_events;
        struct epoll_event      events[MAX_EVENTS];
        size_t                  endpoints;
        SessionManager          sessionManager;
};

#endif
