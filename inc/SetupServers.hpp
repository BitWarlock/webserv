/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:59:30 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/03 18:07:30 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SETUPSERVERS_HPP
#define SETUPSERVERS_HPP

#include "WebServ.hpp"
#include "Config.hpp"
#define MAX_SOCKET 1024
#define INFINITE   -1

class SetupServers
{
    public:
        SetupServers(Config& config);
        ~SetupServers();

        void    CheckPortIp(const std::string& host, const std::string& port, size_t pos_server);
        void    FlagSharedPortIp();
        void    CreateSocket(Server& server);
        void    setAddrForBound(std::string& host, std::string& port, struct sockaddr_in &add_server);
        void    Binding(Server& server, size_t index);
        void    StartSetup(void);
        void    Run(void);
        void    advance();
        
    private:
        const Config&       config;
        size_t              sock_number;
        std::vector<int>    fd_sockets;
        std::vector<int>    fd_clients;
};

#endif
