/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hael-ghd <hael-ghd@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 16:32:33 by hael-ghd          #+#    #+#             */
/*   Updated: 2025/06/23 21:41:00 by hael-ghd         ###   ########.fr       */
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
    std::cout << location.getPath() << "\n";
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
            SetState(Internal_Server_Error);
            ResponseWithError(DEFAULT);
            return (false);
        }
    }
    else
    {
        if (ServerBlock.getRoot().empty())
        {
            SetState(Internal_Server_Error);
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
            return (SetState(Forbidden), ResponseWithError(NONE));
    }

    fd = open(path.c_str(), O_RDONLY);

    if (fd == -1)
        return (SetState(Internal_Server_Error), ResponseWithError(NONE));
    
    BuildGetResponse();
}

void    Response::GetListingPage(void)
{
    DIR *dir = opendir(path.c_str());
    int file = open("./error_pages/template.html", O_RDONLY);
    fd = open("./error_pages/listening_page.html", O_RDWR | O_TRUNC);

    if (!dir || file == -1 || fd == -1)
    {
        state = Forbidden;
        ResponseWithError(NONE);
        return ;
    }
    struct dirent *entry;

    char buff[500];
    int bytes = read(file, buff, sizeof(buff));
    write (fd, buff, bytes);
    buff[bytes] = 0;
    close(file);
    
    std::ostringstream  os;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string value = entry->d_name;
        
        if (value == "." || value == "..")
            continue;
            
        if (entry->d_type == DT_DIR)
            value = "/" + value;

        os << "    <li><a href=\"" + value + "\">" + value + "</a></li>\n";
    }
    os << "  </ul>\n  <hr>\n</body>\n</html>";
    closedir (dir);
    write (fd, os.str().c_str(), os.str().size());
    close(fd);
    SetPath("./error_pages/listening_page.html");
    BuildGetResponse();
}

void    Response::SearchForIndex(void)
{
    if (FromLocation == true)
    {
        if (location.getIndex().empty() && CheckAutoIndex() == ON)
            GetListingPage();

        else if (location.getIndex().empty())
            return (SetState(Forbidden), ResponseWithError(NONE));

        else
            CheckIndexAccess(location.getIndex());
    }
    else
    {
        if (ServerBlock.getIndex().empty() && CheckAutoIndex() == ON)
            GetListingPage();

        else if (ServerBlock.getIndex().empty())
            return (SetState(Forbidden), ResponseWithError(NONE));

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

    fd = open(path.c_str(), O_RDONLY);

    struct stat info;

    stat(path.c_str(), &info);

    std::ostringstream  os;
    os << info.st_size;

    std::string header = "HTTP/1.1 " + oss.str() + " " + getStrState() + "\r\n";
    header += "Content-Type: " + MIME_Type() + "\r\n";
    header += "Content-Length: " + os.str() + "\r\n";
    header += "Connection: close\r\n\r\n";
    
    send(fd_client, header.c_str(), strlen(header.c_str()), MSG_NOSIGNAL);

    char buffer[4096];
    ssize_t n(0);

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
        send(fd_client, buffer, n, MSG_NOSIGNAL);

    close(fd);
}

bool    Response::ReturnDirective(void)
{
    std::ostringstream   statuscode;
    std::string         redirecturl;

    CheckLocations(path);

    if (ServerBlock.getReturnDirective().enabled == false && 
        location.getReturnDirective().enabled == false)
        return (true);
    else if (FromLocation == true)
    {
        statuscode << location.getReturnDirective().status_code;
        redirecturl = location.getReturnDirective().target;
    }
    else
    {
        statuscode << ServerBlock.getReturnDirective().status_code;
        redirecturl = ServerBlock.getReturnDirective().target;
    }

    std::string response = "HTTP/1.1 " + statuscode.str() + " Redirect\r\n";
    response += "location: " + redirecturl + " \r\n";
    response += "Content-Length: 0\r\n";
    response += "Connection: close\r\n\r\n";
    send(fd_client, response.c_str(), strlen(response.c_str()), MSG_NOSIGNAL);
    return (false);
}

void    Response::ChildProccess(std::string interpreter)
{
    std::vector<std::string>    env_storage;
    env_storage.clear();

    env_storage.push_back("REQUEST_METHOD=" + Request.getMethod());
    env_storage.push_back("SCRIPT_NAME=" + Request.getUri());
    env_storage.push_back("SCRIPT_FILENAME=" + path);
    env_storage.push_back("QUERY_STRING=" + Request.getQueryString());
    env_storage.push_back("CONTENT_TYPE=" + Request.getHeaderValue("content-type"));
    env_storage.push_back("SERVER_PROTOCOL=HTTP/1.1");

    for (size_t i = 0; i < env_storage.size(); ++i)
        env[i] = (char*) env_storage[i].c_str();
    env[env_storage.size()] = NULL;

    char    *argv[3];
    argv[0] = (char*) interpreter.c_str();
    argv[1] = (char*) path.c_str();
    argv[2] = NULL;

    if (execve (argv[0], argv, env) == -1)
        std::cerr << "execve: failed\n";
    exit(1);
}

bool    Response::CheckForCGI(void)
{
    size_t  pos = path.rfind('.');
    if (pos == std::string::npos)
        return true;

    std::string extenstion = path.substr(pos + 1);
    std::string interpreter = location.getCgiInfo(extenstion);

    if (interpreter.empty() || access(interpreter.c_str(), F_OK | X_OK) == -1)
        return (SetState(Internal_Server_Error), ResponseWithError(NONE), false);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
        return (SetState(Internal_Server_Error), ResponseWithError(NONE), false);
    int pid = fork();
    if (pid == -1)
        return (SetState(Internal_Server_Error), ResponseWithError(NONE), false);
    else if (pid == 0)
    {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], 1);
        close(pipe_fd[1]);
        ChildProccess(interpreter);
    }
    wait(NULL);
    close(pipe_fd[1]);

    std::string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: text/html\r\n";
    header += "Connection: close\r\n\r\n";
    
    send(fd_client, header.c_str(), strlen(header.c_str()), MSG_NOSIGNAL);

    char buffer[4096];
    ssize_t n(0);

	while ((n = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
        write(pipe_fd[0], buffer, strlen(buffer));
    return (false);
}

void    Response::GetPageResponse(void)
{
    struct stat info;
    
    // if (ReturnDirective() == false)
    //     return ;

    if (GetFullPath(path) == false)
        return ;

    if (stat(path.c_str(), &info) == -1)
        return (SetState(Not_Found), ResponseWithError(NONE));
        
    if (S_ISDIR(info.st_mode))
        return (SearchForIndex());

    if (access(path.c_str(), R_OK) != 0)
        return (SetState(Forbidden), ResponseWithError(NONE));

    if (FromLocation == true)
        if (CheckForCGI() == false)
            return ;

    BuildGetResponse();
}

void    Response::BuildDeleteResponse(void)
{
    if (unlink(path.c_str()) == -1)
        return (SetState(Internal_Server_Error), ResponseWithError(NONE));

    std::string headers = "HTTP/1.1 204 Content\r\n";
    headers += "content-Length: 0\r\n";
    headers += "Connection: close\r\n\r\n";

    send(fd_client, headers.c_str(), strlen(headers.c_str()), MSG_NOSIGNAL);
}

void    Response::DeleteContentResponse(void)
{
    struct stat info;
    
    // if (ReturnDirective() == false)
    //     return ;

    if (GetFullPath(path) == false)
        return ;

    if (stat(path.c_str(), &info) == -1)
        return (SetState(Not_Found), ResponseWithError(NONE));

    if (S_ISDIR(info.st_mode) || access(path.c_str(), W_OK) != 0)
        return (SetState(Forbidden), ResponseWithError(NONE));

    BuildDeleteResponse();
}

void    Response::PostContentResponse(void)
{
    struct stat info;
    
    if (ReturnDirective() == false)
        return ;

    if (GetFullPath(path) == false)
        return ;

    if (FromLocation == true)
        if (CheckForCGI() == false)
            return ;

    size_t  max_size = ServerBlock.getClientBodyLimit();

    if (FromLocation == true)
        max_size = location.getClientBodyLimit();

    std::string lengthstr = Request.getHeaderValue("content-length");
    if (lengthstr.empty())
    {
        std::string chunks = Request.getHeaderValue("transfer-encoding");
        if (chunks.empty())
            return (SetState(Length_Required), ResponseWithError(NONE));
    }

    if (Request.getContentLength() > max_size)
        return (SetState(Content_Too_Large), ResponseWithError(NONE));

    if (stat(path.c_str(), &info) == -1)
    {
        std::string dir = path.substr(0, path.rfind("/"));
        if (access(dir.c_str(), W_OK) == -1)
            return (SetState(Forbidden), ResponseWithError(NONE));
    }
    
    if (S_ISDIR(info.st_mode))
        return (SetState(Conflict), ResponseWithError(NONE));
    
    if (access(path.c_str(), W_OK) != 0)
        return (SetState(Forbidden), ResponseWithError(NONE));
    
    int fd_file = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0664);

    if (fd_file == -1)
        return (SetState(Forbidden), ResponseWithError(NONE));

    if (write(fd_file, Request.getBufferBody().c_str(), Request.getBufferBody().size()) == -1)
        return (close(fd_file), SetState(Internal_Server_Error), ResponseWithError(NONE));
    close(fd_file);

    BuildGetResponse();
}

void    Response::ResponseWithOk(void)
{
    std::string Method = Request.getMethod();

    if (Method == GET)
        GetPageResponse();
    // else if (Method == POST)
    //     AploadContentResponse();
    else
        DeleteContentResponse();
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
        BuildGetResponse();
        return ;
    }

    struct stat info;
    std::string error_page = ServerBlock.getErrorPage(getState());
    
    if (!error_page.empty())
    {
        SetStatePath(RES_ERROR);
        SetPath(error_page);
        if (GetFullPath(path) == false)
            return ;

        if (stat(path.c_str(), &info) != 0)
            return (SetState(Not_Found), ResponseWithError(DEFAULT));

        else if (S_ISDIR(info.st_mode) || access(path.c_str(), R_OK) != 0)
            return (SetState(Internal_Server_Error), ResponseWithError(DEFAULT));
    }
    else
        return (ResponseWithError(DEFAULT));
    
    BuildGetResponse();
}

void    Response::StartForResponse(ParseRequest request, Server BlockServer, int fd_client)
{
    SetRequest(request);
    SetBlockServer(BlockServer);
    SetState(request.getErrorNumber());
    SetPath(request.getUri());
    SetStatePath(NORMAL);
    this->fd_client = fd_client;

    if (getState() == Method_Not_Allowed)
        ResponseWithError(NONE);

    else if (getState() != OK)
    {
        if (ReturnDirective() == false)
            return ;
        ResponseWithError(NONE);
    }

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
