/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:59:30 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/23 20:33:40 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SETUPSERVERS_HPP
#define SETUPSERVERS_HPP

#include "WebServ.hpp"
#include "Config.hpp"

class SetupServers
{
    public:
        SetupServers(const Config& config);
        ~SetupServers();

        bool    CheckPortIp(const std::string& host, const std::string& port, int pos_server);
        void    FlagSharedPortIp(void);
        int     CreateSocket(Server& server);
        void    setAddrForBound(const std::string& host, const std::string& port);
        void    Binding(Server& server);
        void    StartSetup(void);
        void    advance();
        
    private:
        const Config& config;
        int           pos;
        int           fd_sockets[MAX_SOCKET];
};

#endif
