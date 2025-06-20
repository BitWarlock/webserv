/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/20 21:43:03 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Response.hpp"

Response::Response(){}

Response::~Response(){}   

void    Response::CheckLocations(std::string& path)
{
    std::vector<Location*>  locations = ServerBlock.getLocations();
    bool                    flag = false;

    for (size_t i(0); i < locations.size(); i++)
    {
        size_t  pos = path.find(locations[i]->getPath());
        
        if (flag == false && pos == 0)
        {
            SetLocation(*(locations[i]));
            flag = true;
        }
        else if (flag == true && pos == 0)
        {
            if (locations[i]->getPath().size() > location.getPath().size())
                SetLocation(*(locations[i]));
        }
    }
}

bool    Response::GetFullPath(std::string& path)
{
    if (FromLocation == true)
    {    
        if (!location.getRoot().empty())
            SetPath(location.getRoot() + path.substr(location.getPath().size()));
        else if (!ServerBlock.getRoot().empty())
        {
            if (ServerBlock.getRoot()[ServerBlock.getRoot().size() - 1] == '/')
                SetPath(ServerBlock.getRoot() + path.substr(1));
            else
                SetPath(ServerBlock.getRoot() + path);
        }
        else
        {
            state = Internal_Server_Error;
            // ResponseWithError();
            return (false);
        }
    }
    else
    {
        if (ServerBlock.getRoot().empty())
        {
            state = Internal_Server_Error;
            // ResponseWithError();
            return (false);
        }
        if (ServerBlock.getRoot()[ServerBlock.getRoot().size() - 1] == '/')
            SetPath(ServerBlock.getRoot() + path.substr(1));
        else
            SetPath(ServerBlock.getRoot() + path);
    }
    return (true);
}

bool    Response::CheckAutoIndex(void)
{
    if (FromLocation == true)
        return (location.getAutoindex());

    return (ServerBlock.getAutoindex());
}

void    Response::CheckIndexAccess(std::vector<std::string> indexs)
{
    for (size_t i(0); i < indexs.size(); i++)
    {
        std::string join = path + indexs[i];
        if (access(join.c_str(), F_OK) == 0)
        {
            SetPath(join);
            break ;
        }
        else if (i + 1 == indexs.size())
        {
            SetState(Forbidden);
            // ResponseWithError();
            return ;
        }
    }
    fd = open(path.c_str(), O_RDONLY);

    if (fd == -1)
    {
        SetState(Internal_Server_Error);
        // ResponseWithError();
        return ;
    }
    
    BuildGetResponse();
}

void    Response::SearchForIndex(void)
{
    if (FromLocation == true)
    {
        if (location.getIndex().empty())
        {
            SetState(Forbidden);
            // ResponseWithError();
        }
        else
            CheckIndexAccess(location.getIndex());
    }
    else
    {
        if (ServerBlock.getIndex().empty())
        {
            SetState(Forbidden);
            // ResponseWithError();
        }
        else
            CheckIndexAccess(ServerBlock.getIndex());
    }
}

std::string Response::getStrState(void) const
{
    switch (getState())
    {
        case Bad_Request:
            return ("Bad Request");
        case Forbidden:
            return ("Forbidden");
        case Not_Found:
            return ("Not Found");
        case Method_Not_Allowed:
            return ("Method Not Allowed");
        case Content_Too_Large:
            return ("Content Too Large");
        case URI_Too_Long:
            return ("URI Too Long");
        case Request_Header_Fields_Too_Large:
            return ("Request Header Fields Too Large");
        case Internal_Server_Error:
            return ("Internal Server Error");
        case Not_Implemented:
            return ("Not Implemented");
        case HTTP_Version_Not_Supported:
            return ("HTTP Version Not Supported");
        default:
            return ("OK");
    }
}

// std::string Response::getMIME(void) const
// {
    
// }

void    Response::BuildGetResponse(void)
{
    std::ostringstream  oss;

    oss << getState();

    std::string header = "HTTP/1.1 " + oss.str() + " " + getStrState() + "\r\n";
    header += "Content-Type: 100\r\n";
    
    int size(0);
    std::string body;
    
    while (1337)
    {
        char    buff[BUFFER_SIZE];
        int bytes = read(fd, buff, BUFFER_SIZE - 1);
        
        if (bytes == 0)
            break ;
        buff[bytes] = 0;
        size += bytes;
        body += buff;      
    }
        
    oss << body.size();

    header += "Content-Length: " + oss.str() + "\r\n";
    header += "Connection: " + Request.getHeaderValue("connection") + "\r\n\r\n";
    
    std::string response = header + body;
    std::cout << response;
    exit (0);
}

void    Response::CheckIfReadable(void)
{
    if (access(path.c_str(), R_OK) != 0)
    {
        std::cout << "Warning: The file " << path << " is not readable\n";
        SetState(Forbidden);
        // ResponseWithError();
        return ;
    }

    fd = open(path.c_str(), O_RDONLY);

    BuildGetResponse();
}

void    Response::GetPageResponse(void)
{
    struct stat info;
    
    CheckLocations(path);

    if (GetFullPath(path) == false)
        return ;
    std::cout << path << "\n";
    if (stat(path.c_str(), &info) == -1)
    {
        std::cout << "Warning: stat() function failed\n" << strerror(errno) << "\n";
        SetState(Not_Found);
        exit(0);
        // ResponseWithError();
        return ;
    }
        
    if (S_ISDIR(info.st_mode))
    {
        // if (CheckAutoIndex() == ON)
        //     GetListingPage();
        // else
            SearchForIndex();
    }
    else
        CheckIfReadable();
}

void    Response::ResponseWithOk(void)
{
    std::string Method = Request.getMethod();

    if (Method == GET)
        GetPageResponse();
    // else if (Method == POST)
    //     AploadContentResponse();
    // else
    //     DeleteContentResponse();   
}

void    Response::StartForResponse(ParseRequest request, Server BlockServer)
{
    SetRequest(request);
    SetBlockServer(BlockServer);
    SetState(request.getErrorNumber());
    SetPath(request.getUri());

    // if (getState() != OK)
    //     ResponseWithError();
    // else
        ResponseWithOk();
}

void    Response::ResponseWithError(void){}

int     Response::getState() const {return state;}

void    Response::SetRequest(ParseRequest Request){this->Request = Request;}

void    Response::SetBlockServer(Server BlockServer){this->ServerBlock = BlockServer;}

void    Response::SetLocation(Location location){this->location = location; this->FromLocation = true;}

void    Response::SetState(int state){this->state = state;}

void    Response::SetPath(std::string path){this->path = path;}
