/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:28:12 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/17 20:51:30 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <sstream>
#include "WebServ.hpp"
#include "Lexer.hpp"
#include "Location.hpp"
#include "Server.hpp"
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

enum ResponseNumberState
{
    OK = 200,
    Bad_Request = 400,
    Forbidden = 403,
    Not_Found = 404,
    Method_Not_Allowed = 405,
    Content_Too_Large = 413,
    URI_Too_Long = 414,
    Request_Header_Fields_Too_Large = 431,
    Internal_Server_Error = 500,
    Not_Implemented = 501,
    HTTP_Version_Not_Supported = 505,   
};

class Response{
    
    private:
        std::string     Res;
        int             state;
        ParseRequest    Request;
        Server          ServerBlock;
        Location        location;
        bool            FromLocation;
        
    public:
        Response();
        ~Response();

        int     getState(void) const;
        void    StartForResponse(ParseRequest Request, Server BlockServer);

        void    SetRequest(ParseRequest Request);
        void    SetBlockServer(Server BlockServer);
        void    SetLocation(Location location);        
        
        void    ResponseWithError(void);
        void    ResponseWithOk(void);
        void    SendResponse(void);
        void    GetPageResponse(void);
        void    AploadContentResponse(void);
        void    DeleteContentResponse(void);
        void    CheckLocations(std::string& path);
        bool    CheckRootLocation(std::string& path);
        bool    GetFullPath(std::string& path);
        void    GetListingPage();
        void    CHeckForIndex();
        
};

#endif