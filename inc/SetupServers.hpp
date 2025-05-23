/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SetupServers.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 20:59:30 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/05/23 17:04:22 by hael-ghd         ###   ########.fr       */
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

        bool    CheckPortIp(Server& server, int pos_server);
        void    FlagSharedPortIp(void);
        void    StartSetup(void);
        void    bindServer(const Server& server);
        
    private:
        const Config& config;
};

#endif
