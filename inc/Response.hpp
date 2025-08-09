/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:28:12 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/23 18:23:10 by hael-ghd         ###   ########.fr       */
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
#include "SessionManager.hpp"
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Cgi.hpp"

#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

#define ON true
#define OFF false

#define BUFFER_SIZE 400

#define DEFAULT 0
#define NONE 1
#define NORMAL 2
#define RES_ERROR 3

#define html "html"
#define htm "htm"
#define css "css"
#define js "js"
#define json "json"
#define txt "txt"
#define jpg "jpg"
#define jpeg "jpeg"
#define png "png"
#define gif "gif"
#define svg "svg"
#define ico "ico"
#define woff "woff"
#define woff2 "woff2"
#define ttf "ttf"
#define eot "eot"
#define mp4 "mp4"
#define webm "webm"
#define ogg "ogg"
#define mp3 "mp3"
#define pdf "pdf"
#define zip "zip"
#define xml "xml"

enum ResponseNumberState
{
    OK = 200,
    Created = 201,
    Moved_Permanently = 301,
    Bad_Request = 400,
    Forbidden = 403,
    Not_Found = 404,
    Method_Not_Allowed = 405,
    Conflict = 409,
    Length_Required = 411,
    Content_Too_Large = 413,
    URI_Too_Long = 414,
    Unsupported_Media_Type = 415,
    Request_Header_Fields_Too_Large = 431,
    Internal_Server_Error = 500,
    Not_Implemented = 501,
    Bad_Gateway = 502,
    HTTP_Version_Not_Supported = 505, 
};

class Response{
    
    private:
        int                 fd;
        int                 fd_client;
        int                 state;
        int                 state_path;
        ParseRequest        Request;
        Server              ServerBlock;
        Location            location;
        bool                FromLocation;
        std::string         path;
        char                *env[7];
        std::string         Body;
        SessionManager*		sessionManager;
        std::vector
			<std::string>	cookies_to_set;
		


        bool			shouldExecuteCgi(ParseRequest& request, Server& server);
        void			sendCgiResponse(const CgiResult& cgi_result);
        Location*		findMatchingLocation(const std::string& uri, Server& server);
        std::string		getScriptPath(const Location& location, const std::string& uri);
        std::string		getFileExtension(const std::string& uri);
        void			setCookie(const std::string& name, const std::string& value, 
                                const std::string& path = "/", const std::string& expires = "", 
                                bool http_only = true);
        void           addCookiesToHeaders();
        
    public:
        Response();
        ~Response();

        void   	handleCgiRequest(ParseRequest& request, Server& server);
        void	setSessionManager(SessionManager* sm);
        
        // Setters
        
        std::string         res;
        void    SetRequest(ParseRequest Request);
        void    SetBlockServer(Server BlockServer);
        void    SetState(int state);
        void    SetStatePath(int state_path);
        void    SetLocation(Location location);
        void    SetPath(std::string path);        
        
        // Getters
        
        std::string getPath();
        int         getState(void) const;
        std::string getStrState(void) const;
        std::string MIME_Type(void) const;
        std::string ConnectionState(void) const;
        
        // Startresponse
        
        void    StartForResponse(ParseRequest Request, int fd_client);
        
        void    ResponseWithError(int serve);
        
        void    ResponseWithOk(void);
        
        void    GetPageResponse(void);

        bool    ReturnDirective(void);
        
        void    AploadContentResponse(void);
        
        void    DeleteContentResponse(void);
        
        void    PostContentResponse(void);
        
        void    CheckLocations(std::string& path);
        
        bool    GetFullPath(std::string& path);
        

        
        bool    CheckAutoIndex(void);
        
        void    GetListingPage(void);
        
        void    SearchForIndex(void);
        
        void    CheckIndexAccess(std::vector<std::string> indexs);
        
        void    BuildGetResponse(void);

        void    BuildDeleteResponse(void);

        void    BuildPostResponse(void);

        std::string DefaultForMatchError(void);



        bool    MultiPart(std::string body);

        bool    Chunked(void);
        
};

#endif
