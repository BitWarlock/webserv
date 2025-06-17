/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/17 20:49:02 by hael-ghd         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(){}

Response::~Response(){}

bool    Response::CheckRootLocation(std::string& path)
{
    std::vector<Location*>  locations = ServerBlock.getLocations();
    
    if (path.size() != 1)
        return (true);

    for (size_t i(0); i < locations.size(); i++)
    {
        if (locations[i]->getPath().size() == 1 &&
            path == locations[i]->getPath())
        {
            SetLocation(*(locations[i]));
            return (false);
        }
    }
    return (false);
}    

void    Response::CheckLocations(std::string& path)
{
    std::vector<Location*>  locations = ServerBlock.getLocations();
    bool                    flag = false;

    if (CheckRootLocation(path) == false)
        return ;

    for (size_t i(0); i < locations.size(); i++)
    {
        size_t  pos = path.find(locations[i]->getPath());
        
        if (flag == false && pos != std::string::npos)
        {
            SetLocation(*(locations[i]));
            flag = true;
        }
        else if (flag == true && pos != std::string::npos)
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
            path = location.getRoot() + path;
        else if (!ServerBlock.getRoot().empty())
            path = ServerBlock.getRoot() + path;
        else
        {
            state = Internal_Server_Error;
            ResponseWithError();
            return (false);
        }
    }
    else
    {
        if (ServerBlock.getRoot().empty())
        {
            state = Internal_Server_Error;
            ResponseWithError();
            return (false);
        }
        path = ServerBlock.getRoot() + path;
    }
    return (true);
}

void    Response::GetPageResponse(void)
{
    std::string path = Request.getUri();
    struct stat info;
    
    CheckLocations(path);

    if (GetFullPath(path) == false)
        return ;

    if (stat(path.c_str(), &info) != 0)
        throw std::runtime_error("Warning: stat() function failed to get file status\n");
        
    if (S_ISDIR(info.st_mode))
    {
        if (ServerBlock.getAutoindex() == true)
            GetListingPage();
        else
            CHeckForIndex
    }
    else
    {
        
    } 
}

void    Response::ResponseWithOk(void)
{
    std::string Method = Request.getMethod();

    if (Method == GET)
        GetPageResponse();
    else if (Method == POST)
        AploadContentResponse();
    else
        DeleteContentResponse();   
}

void    Response::StartForResponse(ParseRequest request, Server BlockServer)
{
    SetRequest(request);
    SetBlockServer(BlockServer);

    if (request.getErrorNumber() != OK)
        ResponseWithError();
    else
        ResponseWithOk();

    SendResponse();
}

void    Response::ResponseWithError(void){}

int     Response::getState() const {return state;}

void    Response::SetRequest(ParseRequest Request){this->Request = Request;}

void    Response::SetBlockServer(Server BlockServer){this->ServerBlock = BlockServer;}

void    Response::SetLocation(Location location){this->location = location; this->FromLocation = true;}
