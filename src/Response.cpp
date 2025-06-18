/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/18 21:53:19 by hael-ghd         ###   ########.fr       */
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
        {
            path = path.substr(location.getPath().size());
            path = location.getRoot() + path;
        }
        else if (!ServerBlock.getRoot().empty())
            path = ServerBlock.getRoot() + path.substr(1);
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
        path = ServerBlock.getRoot() + path.substr(1);
    }
    return (true);
}

bool    Response::CheckAutoIndex(void)
{
    if (FromLocation == true)
        return (location.getAutoindex());

    return (ServerBlock.getAutoindex());
}

void    Response::CheckIndexAccess(void)
{
    if (access(path.c_str(), R_OK) == -1)
    {
        SetState(Forbidden);
        ResponseWithError();
        return ;
    }

    std::ifstream   file(path.c_str());
    
    if (!file.is_open())
    {
        SetState(Internal_Server_Error);
        ResponseWithError();
        return ;
    }
    
}

void    Response::SearchForIndex(void)
{
    if (FromLocation == true)
    {
        if (location.getIndex().empty())
        {
            SetState(Forbidden);
            ResponseWithError();
        }
        else
            CheckIndexAccess();
    }
    else
    {
        if (ServerBlock.getIndex().empty())
        {
            SetState(Forbidden);
            ResponseWithError();
        }
        else
            CheckIndexAccess();
    }
}

void    Response::GetPageResponse(void)
{
    this->path = Request.getUri();
    struct stat info;
    
    CheckLocations(path);

    if (GetFullPath(path) == false)
        return ;

    if (stat(path.c_str(), &info) == -1)
    {
        SetState(Not_Found);
        ResponseWithError();
        return ;
    }
        
    if (S_ISDIR(info.st_mode))
    {
        if (CheckAutoIndex() == ON)
            GetListingPage();
        else
            SearchForIndex();
    }
    // else
    // {
        
    // } 
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

    if (getState() != OK)
        ResponseWithError();
    else
        ResponseWithOk();

    // SendResponse();
}

void    Response::ResponseWithError(void){}

int     Response::getState() const {return state;}

void    Response::SetRequest(ParseRequest Request){this->Request = Request;}

void    Response::SetBlockServer(Server BlockServer){this->ServerBlock = BlockServer;}

void    Response::SetLocation(Location location){this->location = location; this->FromLocation = true;}

void    Response::SetState(int state){this->state = state;}

void    Response::SetPath(std::string path){this->path = path;}
