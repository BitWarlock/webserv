/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/23 19:06:59 by hael-ghd         ###   ########.fr       */
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
            std::string join = path;
            
            if (state_path == NORMAL)
                join = path.erase(location.getPath().size());
            if (join.empty())
                SetPath(location.getRoot());
            else if (join[0] == '/')
                SetPath(location.getRoot() + join.erase(0, 1));
            else
                SetPath(location.getRoot() + join);
        }
        else if (!ServerBlock.getRoot().empty())
            SetPath(ServerBlock.getRoot() + path.erase(0, 1));
        else
        {
            state = Internal_Server_Error;
            ResponseWithError(DEFAULT);
            return (false);
        }
    }
    else
    {
        if (ServerBlock.getRoot().empty())
        {
            state = Internal_Server_Error;
            ResponseWithError(DEFAULT);
            return (false);
        }
        SetPath(ServerBlock.getRoot() + path.erase(0, 1));
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
            ResponseWithError(NONE);
            return ;
        }
    }
    fd = open(path.c_str(), O_RDONLY);

    if (fd == -1)
    {
        SetState(Internal_Server_Error);
        ResponseWithError(NONE);
        return ;
    }
    
    BuildGetResponse();
}

void    Response::GetListingPage(void)
{
    DIR *dir = opendir(path.c_str());
    fd = open("./error_pages/listening_page.html", O_RDWR);
    
    if (!dir || fd == -1)
    {
        state = Forbidden;
        ResponseWithError(NONE);
        return ;
    }

    struct dirent *entry;

    lseek (fd, 393, SEEK_SET);

    while ((entry = readdir(dir)) != NULL)
    {
        std::string value = entry->d_name;
        
        if (value == "." || value == "..")
            continue;
            
        if (entry->d_type == DT_DIR)
            value = "/" + value;

        std::string name = "    <li><a href=\"" + value + "\">" + value + "</a></li>\n";
        write (fd, name.c_str(), name.size());
    }
    write (fd, "  </ul>\n  <hr>\n</body>\n</html>\n", 32);
    close(fd);
    closedir (dir);
    fd = open("./error_pages/listening_page.html", O_RDWR);
    BuildGetResponse();
}

void    Response::SearchForIndex(void)
{
    if (FromLocation == true)
    {
        if (location.getIndex().empty() && CheckAutoIndex() == ON)
            GetListingPage();
        else if (location.getIndex().empty())
        {
            SetState(Forbidden);
            ResponseWithError(NONE);
        }
        else
            CheckIndexAccess(location.getIndex());
    }
    else
    {
        if (ServerBlock.getIndex().empty() && CheckAutoIndex() == ON)
            GetListingPage();
        else if (ServerBlock.getIndex().empty())
        {
            SetState(Forbidden);
            ResponseWithError(NONE);
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

std::string Response::MIME_Type(void) const
{
    size_t  pos = path.rfind('.');

    if (pos == std::string::npos)
        return ("application/octet-stream");

    std::string extenstion = path.substr(pos + 1);

    if (extenstion == html || extenstion == htm)
        return ("text/html");
    else if (extenstion == css)
        return ("text/css");
    else if (extenstion == js)
        return ("application/javascript");
    else if (extenstion == json || extenstion == pdf || extenstion == zip
        || extenstion == xml)
        return ("application/" + extenstion);
    else if (extenstion == txt)
        return ("text/plain");
    else if (extenstion == jpg || extenstion == jpeg)
        return ("image/jpeg");
    else if (extenstion == png || extenstion == gif)
        return ("image/" + extenstion);
    else if (extenstion == svg)
        return ("image/svg+xml");
    else if (extenstion == ico)
        return ("image/x-icon");
    else if (extenstion == ogg)
        return ("audio/ogg");
    else if (extenstion == mp3)
        return ("audio/mpeg");
    else if (extenstion == woff || extenstion == woff2 || extenstion == ttf)
        return ("font/" + extenstion);
    else if (extenstion == eot)
        return ("application/vnd.ms-fontobject");
    else if (extenstion == mp4 || extenstion == webm)
        return ("video/" + extenstion);
    return ("application/octet-stream");
}

void    Response::BuildGetResponse(void)
{
    std::ostringstream  oss;

    oss << getState();

    std::string header = "HTTP/1.1 " + oss.str() + " " + getStrState() + "\r\n";
    header += "Content-Type: " + MIME_Type() + "\r\n";
    
    size_t      size(0);
    std::string body;

    while (1337)
    {
        char    buff[BUFFER_SIZE];
        int bytes = read(fd, buff, BUFFER_SIZE - 1);
        std::cout << bytes << "  bytes\n";
        if (bytes == 0 || bytes == -1)
            break ;
        buff[bytes] = 0;
        size += bytes;
        body += buff;      
    }
        
    std::ostringstream  os;
    os << body.size();

    header += "Content-Length: " + os.str() + "\r\n";
    header += "Connection: " + Request.getHeaderValue("connection") + "\r\n\r\n";
    
    res = header + body;
    std::cout << res;
}

void    Response::CheckIfReadable(void)
{
    if (access(path.c_str(), R_OK) != 0)
    {
        std::cout << "Warning: The file " << path << " is not readable\n";
        SetState(Forbidden);
        if (state_path == ERROR)
            ResponseWithError(DEFAULT);
        else
            ResponseWithError(NONE);
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

    if (stat(path.c_str(), &info) == -1)
    {
        std::cout << "Warning: the file does not exist\n";
        SetState(Not_Found);
        ResponseWithError(NONE);
        return ;
    }
        
    if (S_ISDIR(info.st_mode))
        SearchForIndex();

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

std::string Response::DefaultForMatchError(void)
{
    int error = getState();
    
    if (error == Bad_Request)
        return "./error_pages/Bad_request.html";
    else if (error == Forbidden)
        return "./error_pages/Forbidden.html";
    else if (error == Not_Found)
        return "./error_pages/Not_Found.html";
    else if (error == Method_Not_Allowed)
        return "./error_pages/Method_Not_Allowed.html";
    else if (error == Content_Too_Large)
        return "./error_pages/Content_Too_Large.html";
    else if (error == URI_Too_Long)
        return "./error_pages/URI_Too_Long.html";
    else if (error == Request_Header_Fields_Too_Large)
        return "./error_pages/Request_Header_Fields_Too_Large.html";
    else if (error == Internal_Server_Error)
        return "./error_pages/Internal_Server_Error.html";
    else if (error == Not_Implemented)
        return "./error_pages/Not_Implemented.html";
    else
        return "./error_pages/HTTP_Version_Not_Supported.html";
}

void    Response::ResponseWithError(int serve)
{
    if (serve == DEFAULT)
    {
        SetPath(DefaultForMatchError());
        fd = open(path.c_str(), O_RDONLY);
        BuildGetResponse();
        return ;
    }

    struct stat info;

    SetStatePath(ERROR);
    
    size_t  size(0);

    for (size_t i(0); i < ServerBlock.getErrorPage(getState()).size(); i++)
    {
        SetPath(ServerBlock.getErrorPage(getState()));
        
        if (GetFullPath(path) == false)
            return ;

        if (stat(path.c_str(), &info) == 0)
            break ;
        size++;
    }
      
    if (size + 1 >= ServerBlock.getErrorPage(getState()).size())
    {
        std::cout << "Warning: the file does not exist\n";
        SetState(Not_Found);
        ResponseWithError(DEFAULT);
        return ;
    }
        
    CheckIfReadable();
}

void    Response::StartForResponse(ParseRequest request, Server BlockServer)
{
    SetRequest(request);
    SetBlockServer(BlockServer);
    SetState(request.getErrorNumber());
    SetPath(request.getUri());
    SetStatePath(NORMAL);

    if (getState() != OK)
        ResponseWithError(NONE);
    else
        ResponseWithOk();
}

int     Response::getState() const {return state;}

void    Response::SetRequest(ParseRequest Request){this->Request = Request;}

void    Response::SetBlockServer(Server BlockServer){this->ServerBlock = BlockServer;}

void    Response::SetLocation(Location location){this->location = location; this->FromLocation = true;}

void    Response::SetState(int state){this->state = state;}

void    Response::SetStatePath(int state_path){this->state_path = state_path;}

void    Response::SetPath(std::string path){this->path = path;}
