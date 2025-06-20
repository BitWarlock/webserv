/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:28:12 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/20 21:15:27 by hael-ghd         ###   ########.fr       */
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
#include <fcntl.h>

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"
#define ON true
#define OFF false
#define BUFFER_SIZE 400

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
        int                 fd;
        int                 state;
        ParseRequest        Request;
        Server              ServerBlock;
        Location            location;
        bool                FromLocation;
        std::string         path;
        
    public:
        Response();
        ~Response();
       
        // Setters
        
        void    SetRequest(ParseRequest Request);
        void    SetBlockServer(Server BlockServer);
        void    SetState(int state);
        void    SetLocation(Location location);
        void    SetPath(std::string path);        
        
        // Getters
        
        int         getState(void) const;
        std::string getStrState(void) const;
        // std::string getMIME(void) const;
        std::string ConnectionState(void) const;
        
        // Startresponse
        
        void    StartForResponse(ParseRequest Request, Server BlockServer);
        
        void    ResponseWithError(void);
        
        void    ResponseWithOk(void);
        
        void    GetPageResponse(void);
        
        void    AploadContentResponse(void);
        
        void    DeleteContentResponse(void);
        
        void    CheckLocations(std::string& path);
        
        bool    GetFullPath(std::string& path);
        
        void    CheckIfReadable(void);
        
        bool    CheckAutoIndex(void);
        
        void    GetListingPage(void);
        
        void    SearchForIndex(void);
        
        void    CheckIndexAccess(std::vector<std::string> indexs);
        
        void    BuildGetResponse(void);
        
};

#endif