

#include <iostream>
#include <sstream>
#include "inc/Lexer.hpp"
#include <utility>
#include "inc/Server.hpp"
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include "inc/Server.hpp"

enum RequestParseStats
{
    METHOD,
    URL,
    HTTPVERSION,
    HEADERS_KEY,
    HEADER_VALUE,
    ADD_HEADER,
    CONTENTLENGTHBODY,
    READCHUNKSIZE,
    READCHUNK,
    FINISH,
    ERROR,
    NONE,
    CLOSE
};

class ParseRequest{
public:
    ParseRequest();
    ParseRequest(Server *server);
    ~ParseRequest();

    int                                                 errorNumber;
    size_t                                              pos;
    int                                                 CurrntParsState;
    std::string                                         Current_key;
    std::string                                         Current_value;
    std::string                                         Current_header_line;
    std::string                                         Method;
    std::string                                         Url;
    std::string                                         HttpProtocolVersion;
    std::pair<std::string, std::string>                 QuerieStrings;
    Server                                              *S;
    bool                                                chunkedEncoding;
    size_t                                              contentLength;
    std::vector<std::pair<std::string, std::string> >   Headers;
    bool                                                hasValidHost;
    size_t                                              ChunkSize;
    std::string                                         BufferBody;
    std::map<std::string, int>                          NonRepeatablesHeaders;
    std::string                                         Host;
    std::string                                         Port;
    std::string                                         QueryString;
    std::istringstream                                  incomigBytes;
    
    
    // parse input
    void        startParse(int fd);
    void        parseMethod(std::string &str);
    void        parseUrl(std::string &str);
    void        parseHttpVersion(std::string &str);
    void        parseHeaders(std::string &str);
    void        CheckingForBody();
    void        parseContentlengthBody(std::string &str);
    int         HexaStringToDecimalNum(std::string s);
    void        parseChunkedBody(std::string &str);
    void        trimBuff(std::string &str);
    void        toLowerCase(std::string &key);
    bool        isAllSpaces(std::string &str);
    bool        validKey(std::string &key);
    bool        checkIsThereaHost();
    bool        isNumber(std::string toCheck);
    void        ResetParserf();

    // checkers
    bool        isFinish();
    bool        isSupportedMethod(std::string &RequestMethod);
    int         isKnownMethod();
    bool        isValidUrl();
    bool        isHexa(char characetre);
    bool        Unresreved(char c);
    bool        Reserved(char c);
    bool        PercentEncoded(size_t &i);
    bool        isValidVersion();

    // getters
    std::string getMethod();
    std::string getUri();
    std::string getVersion();
    int         getParseState();
    std::string getHeaderValue(std::string key);

    // setters
    void        setMethod(std::string m);
    void        setUri(std::string u);
    void        setVersion(std::string v);
    void        SwitchState(int Next_State);
    void        setErrorNumber(int Number);
    void        setQueryString(std::string qurieInUrl);
    void        Reset();
    void        ResetBuffPos();
};














// construct
ParseRequest::ParseRequest() : errorNumber(0), pos(0), 
                            CurrntParsState(NONE), Method(""), Url(""),
                            HttpProtocolVersion(""), QuerieStrings("", ""), 
                            chunkedEncoding(false), contentLength(0)
{
    BufferBody.clear();
    ChunkSize = 0;
    NonRepeatablesHeaders["host"] = 0;
    NonRepeatablesHeaders["content-length"] = 0;
    NonRepeatablesHeaders["content-type"] = 0;
    NonRepeatablesHeaders["content-encoding"] = 0;
    NonRepeatablesHeaders["transfer-encoding"] = 0;
    NonRepeatablesHeaders["authorization"] = 0;
    NonRepeatablesHeaders["user-agent	"] = 0;
    NonRepeatablesHeaders["connection"] = 0;
    NonRepeatablesHeaders["date"] = 0;
    NonRepeatablesHeaders["upgrade"] = 0;
    NonRepeatablesHeaders["expect"] = 0;
    NonRepeatablesHeaders["range"] = 0;
}

// construct with params
ParseRequest::ParseRequest(Server *server) : errorNumber(0), pos(0), 
                            CurrntParsState(NONE), Method(""), Url(""),
                            HttpProtocolVersion(""), QuerieStrings("", ""), 
                            S(server), chunkedEncoding(false), contentLength(0)
{
    BufferBody.clear();
    ChunkSize = 0;
    NonRepeatablesHeaders["host"] = 0;
    NonRepeatablesHeaders["content-length"] = 0;
    NonRepeatablesHeaders["content-type"] = 0;
    NonRepeatablesHeaders["content-encoding"] = 0;
    NonRepeatablesHeaders["transfer-encoding"] = 0;
    NonRepeatablesHeaders["authorization"] = 0;
    NonRepeatablesHeaders["user-agent	"] = 0;
    NonRepeatablesHeaders["connection"] = 0;
    NonRepeatablesHeaders["date"] = 0;
    NonRepeatablesHeaders["upgrade"] = 0;
    NonRepeatablesHeaders["expect"] = 0;
    NonRepeatablesHeaders["range"] = 0;
}

// distructor
ParseRequest::~ParseRequest() {}

// getters
std::string ParseRequest::getMethod() { return (Method); }
std::string ParseRequest::getVersion() { return (HttpProtocolVersion); }
std::string ParseRequest::getUri() { return (Url); }
int ParseRequest::getParseState() { return (CurrntParsState); }

// setters
void ParseRequest::setMethod(std::string m) { Method = m; }
void ParseRequest::SwitchState(int Next_State) { CurrntParsState = Next_State; }
void ParseRequest::setUri(std::string u) { Url = u; }
void ParseRequest::setVersion(std::string v) { HttpProtocolVersion = v; }
void ParseRequest::Reset() { SwitchState(NONE); }
void ParseRequest::setErrorNumber(int Number)
{
    errorNumber = Number;
    SwitchState(ERROR);
}
void ParseRequest::ResetBuffPos() { pos = 0; }

// checkers
bool ParseRequest::isFinish() { return (getParseState() == FINISH); }
bool ParseRequest::isSupportedMethod(std::string &RequestMethod)
{
    const std::vector<std::string> &sMethods = S->getAllowedMethods();
    std::string m = RequestMethod;
    for (size_t i = 0; i < sMethods.size(); i++)
    {
        if (sMethods[i] == m)
            return (true);
    }
    return (false);
}
int ParseRequest::isKnownMethod()
{
    if (Method == "GET" || Method == "POST" || \
        Method == "DELETE" || Method == "HEAD" || \
        Method == "PUT" || Method == "OPTIONS" || \
        Method == "TRACE" || Method == "CONNECT"){
        return (405);
    }
    return (501);
}
bool ParseRequest::isHexa(char characetre) { return (isxdigit(characetre)); }
bool ParseRequest::Unresreved(char c)
{
    std::string s = "-._~";
    return (std::isalnum(c) || (s.find(c) != std::string::npos));
}
bool ParseRequest::Reserved(char c)
{
    std::string ResevedCharacters = ":/?#[]@!$&'()*+,;=";
    return (ResevedCharacters.find(c) != std::string::npos);
}
bool ParseRequest::PercentEncoded(size_t &i)
{
    if (i + 2 <= Url.size() && isHexa(Url[i + 1]) && isHexa(Url[i + 2]))
        return (i += 2, true);
    return (false);
}
bool ParseRequest::isValidUrl()
{
    if (Url.size() >= 8192)
        return (SwitchState(ERROR), setErrorNumber(414), false);
    if (Url.empty() || Url[0] != '/')
        return (SwitchState(ERROR), setErrorNumber(400), false);
    for (size_t i(0); i < Url.size(); i++)
    {
        if ((Url[i] == '%' && !PercentEncoded(i)) || \
            (!Unresreved(Url[i]) && !Reserved(Url[i])))
            return (SwitchState(ERROR), setErrorNumber(400), false);
    }
    return (true);
}

// parsers
void ParseRequest::toLowerCase(std::string &key)
{
    for (size_t i(0); i < key.size(); i++)
    {
        key[i] = std::tolower((unsigned char)key[i]);
    }
}

void ParseRequest::trimBuff(std::string &str)
{
    pos = str.find_first_not_of(" ");
    if (pos != std::string::npos)
        str.erase(0, pos);
    ResetBuffPos();
}

void ParseRequest::parseMethod(std::string &str)
{
    if (isspace(str[0]))
        return (SwitchState(ERROR), setErrorNumber(400));
    pos = str.find(' ');
    if (pos == std::string::npos)
    {
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (SwitchState(ERROR), setErrorNumber(400));
        Method += str;
        str.clear();
        return (ResetBuffPos());
    }
    Method += str.substr(0, pos);
    str.erase(0, pos + 1);
    ResetBuffPos();
    // if (!isSupportedMethod(Method))
    //     return (setErrorNumber(isKnownMethod()));
    SwitchState(URL);
}
void ParseRequest::setQueryString(std::string queryInUrl) { QueryString = queryInUrl; }
void ParseRequest::parseUrl(std::string &str)
{
    trimBuff(str);
    pos = str.find(' ');
    if (pos == std::string::npos)
    {
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (SwitchState(ERROR), setErrorNumber(400));
        Url.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }
    Url.append(str.substr(0, pos));
    str.erase(0, pos + 1);
    if (!isValidUrl())
        return;
    if ((pos = Url.find('?')) != std::string::npos)
    {
        setQueryString(Url.substr(pos + 1, Url.size()));
        Url.erase(pos);
    }
    ResetBuffPos();
    SwitchState(HTTPVERSION);
}
bool ParseRequest::isValidVersion()
{
    if (HttpProtocolVersion == "HTTP/1.1")
        return (true);
    if (HttpProtocolVersion == "HTTP/1.0" || HttpProtocolVersion == "HTTP/0.9" ||
        HttpProtocolVersion == "HTTP/2" || HttpProtocolVersion == "HTTP/3")
        return (SwitchState(ERROR), setErrorNumber(505), false);
    return (SwitchState(ERROR), setErrorNumber(400), false);
}

void ParseRequest::parseHttpVersion(std::string &str)
{
    trimBuff(str);
    pos = str.find("\r\n");
    if (pos == std::string::npos)
    {
        HttpProtocolVersion.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }
    if (isspace(str[pos - 1]))
        return (SwitchState(ERROR), setErrorNumber(400));
    HttpProtocolVersion.append(str.substr(0, pos));
    str.erase(0, pos + 2);
    if (!isValidVersion())
        return;
    ResetBuffPos();
    SwitchState(HEADERS_KEY);
}
bool ParseRequest::isAllSpaces(std::string &str)
{
    for (size_t i(0); i < str.size(); i++)
    {
        if (!isspace(str[i]))
            return (false);
    }
    return (true);
}
bool ParseRequest::validKey(std::string &key)
{
    std::string visbleChar = "!#$%&'*+-.^_|~`";
    for (size_t i(0); i < key.size(); i++)
    {
        if (!isalnum(key[i]) && visbleChar.find(key[i]) == std::string::npos)
            return false;
    }
    return true;
}

bool ParseRequest::checkIsThereaHost()
{
    std::string hoststring;
    for (size_t i(0); i < Headers.size(); i++)
    {
        if (Headers[i].first == "host" && !Headers[i].second.empty())
        {
            size_t p = Headers[i].second.find(':');
            if (p != std::string::npos)
            {
                Host = Headers[i].second.substr(0, p);
                Port = Headers[i].second.substr(p + 1);
            }
            else
                Host = Headers[i].second;
            hasValidHost = true;
            return (true);
        }
    }
    return (SwitchState(ERROR), setErrorNumber(400), false);
}
void ParseRequest::parseHeaders(std::string &str)
{
        pos = str.find("\r\n");
        if (pos == std::string::npos)
            return;
            if (CurrntParsState == HEADERS_KEY){
                Current_header_line = str.substr(0, pos);
                std::cout << "this is the str >> " << Current_header_line ;
            str.erase(0, pos + 2);
            if (Current_header_line.size() >= 8192)
                return (SwitchState(ERROR), setErrorNumber(431));
            pos = Current_header_line.find(':');
            if (pos == std::string::npos || isspace(Current_header_line[pos - 1]))
                return (SwitchState(ERROR), setErrorNumber(400));    
            Current_key = Current_header_line.substr(0, pos);
            toLowerCase(Current_key);
            SwitchState(HEADER_VALUE);
        }
        if (CurrntParsState ==  HEADER_VALUE){
            Current_value += Current_header_line.substr(pos + 1);
            trimBuff(Current_value);
            if (!validKey(Current_key) || Current_key.empty() || \
                isAllSpaces(Current_key) || Current_value.empty()){
                    return (SwitchState(ERROR), setErrorNumber(400));
                }
            SwitchState(ADD_HEADER);
        }
        if (CurrntParsState == ADD_HEADER){
            std::map<std::string, int>::iterator it = NonRepeatablesHeaders.find(Current_key);
            if (it != NonRepeatablesHeaders.end())
            {
                if (it->second)
                    return (SwitchState(ERROR), setErrorNumber(400));
                it->second++;
            }
            Headers.push_back(std::make_pair(Current_key, Current_value));
            Current_key = "";
            Current_value = "";
            SwitchState(HEADERS_KEY);
        }
        ResetBuffPos();
        if (str[0] == '\r' && str[1] == '\n')
        {
            str.erase(0, 2);
            if (!checkIsThereaHost())
                return (SwitchState(ERROR), setErrorNumber(400));
            CheckingForBody();
            return;
        }
}

bool ParseRequest::isNumber(std::string toCheck)
{
    for (size_t i(0); i < toCheck.size(); i++)
    {
        if (!isdigit(toCheck[i]))
            return (false);
    }
    return (true);
}

void ParseRequest::CheckingForBody()
{
    bool TransferEncodingPresent = false;
    bool contentLengthPresent = false;
    if (Method == "GET" || Method == "DELETE"){

        return (SwitchState(FINISH));
    }
    std::vector<std::pair<std::string, std::string> >::iterator Headersit;
    for (Headersit = Headers.begin(); Headersit != Headers.end(); Headersit++)
    {
        if (Headersit->first == "transfer-encoding")
        {
            TransferEncodingPresent = true;
            std::string tmp = Headersit->second;
            toLowerCase(tmp);
            if (tmp == "chunked")
                chunkedEncoding = true;
            else
                return (SwitchState(ERROR), setErrorNumber(501));
        }
        if (Headersit->first == "content-length")
        {
            contentLengthPresent = true;
            if (!isNumber(Headersit->second))
                return (SwitchState(ERROR), setErrorNumber(400));
            contentLength = std::atoi(Headersit->second.c_str());
            // if (contentLength < 0)
            //     return (SwitchState(ERROR), setErrorNumber(400));
            //
        }
    }
    if (TransferEncodingPresent && contentLengthPresent)
        return (SwitchState(ERROR), setErrorNumber(400));
    if (!chunkedEncoding && !contentLengthPresent)
        return (SwitchState(FINISH));
    if (chunkedEncoding)
        return SwitchState(READCHUNKSIZE);
    SwitchState(CONTENTLENGTHBODY);
}
void ParseRequest::parseContentlengthBody(std::string &str)
{
    if (contentLength == 0)
        return (SwitchState(FINISH));
    pos = str.find("\r\n");
    if (pos != std::string::npos){
        BufferBody =  str.substr(0,pos);
        str.clear();
        std::cout << "something\n";
        if (contentLength != BufferBody.size())
            return (SwitchState(ERROR), setErrorNumber(400));
        return (SwitchState(FINISH));
    }
}

int ParseRequest::HexaStringToDecimalNum(std::string s)
{
    std::istringstream stremstr(s);
    int ret = 0;
    stremstr >> std::hex >> ret;
    return ret;
}

void ParseRequest::parseChunkedBody(std::string &str)
{
    if (CurrntParsState == READCHUNKSIZE)
    {
        pos = str.find("\r\n");
        if (pos != std::string::npos)
        {
            std::string StringChunkSize = str.substr(0, pos);
            for (size_t i(0); i < StringChunkSize.size(); i++)
            {
                if (!isHexa(StringChunkSize[i]))
                    return (SwitchState(ERROR), setErrorNumber(400));
            }
            ChunkSize = HexaStringToDecimalNum(StringChunkSize);
            // if (ChunkSize > S->getClientBodyLimit())
            //     return (SwitchState(ERROR), setErrorNumber(413));
            str.erase(0, pos + 2);
            ResetBuffPos();
            if (ChunkSize == 0)
                return (SwitchState(FINISH));
            SwitchState(READCHUNK);
        }
    }
    if (CurrntParsState == READCHUNK)
    {
            if (str.size() >= ChunkSize + 2)
            {
                BufferBody.append(str.substr(0, ChunkSize));
                str.erase(0, ChunkSize + 2);
                ChunkSize = 0;
                ResetBuffPos();
                SwitchState(READCHUNKSIZE);
            }
    }
}

std::string ParseRequest::getHeaderValue(std::string key){
    std::vector<std::pair<std::string , std::string> >::iterator i;
    for (i = Headers.begin(); i != Headers.end();i++){
        if (i->first == key)
            return (i->second);
    }
    return ("");
}


void        ParseRequest::ResetParserf(){
    errorNumber = 0;
    ResetBuffPos();
    CurrntParsState = NONE;
    Current_key.clear();
    Current_value.clear();
    Current_header_line.clear();
    Method.clear();
    Url.clear();
    HttpProtocolVersion.clear();
    chunkedEncoding = 0;
    contentLength = 0;
    Headers.clear();
    hasValidHost = 0;
    ChunkSize = 0;
    BufferBody.clear();
    QueryString.clear();
}

void ParseRequest::startParse(int fd)
{
    std::string buff;
    
    buff =
    "POST /api/data HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "31\r\n"
    "{\"name\": \"John Doe\", "
    "\"email\": \"john@example.com\"}\r\n"
    "0\r\n"
    "\r\n";
    while (CurrntParsState != CLOSE)
    {
        // std::cout << CurrntParsState << "\n";
        // char        str[500];
        // memset(str, 0, sizeof(str));
        // int bytes = recv(fd, str, sizeof(str) - 1, 0);
        // if (!bytes)
        //     return (SwitchState(CLOSE));
        // buff.append(str);
        if (CurrntParsState == NONE)
            SwitchState(METHOD);
        if (CurrntParsState == METHOD)
            parseMethod(buff);
        if (CurrntParsState == URL)
            parseUrl(buff);
        if (CurrntParsState == HTTPVERSION)
            parseHttpVersion(buff);
        if (CurrntParsState == HEADERS_KEY || CurrntParsState == HEADER_VALUE || CurrntParsState ==  ADD_HEADER)
            parseHeaders(buff);
        if (CurrntParsState == CONTENTLENGTHBODY)
            parseContentlengthBody(buff);
        if (CurrntParsState == READCHUNKSIZE || CurrntParsState == READCHUNK)
            parseChunkedBody(buff);
        if (CurrntParsState ==  FINISH || CurrntParsState == ERROR)
            break;
    }
}















Location::Location(const std::string &path)
    : path(path), autoindex(false), client_body_limit(0) { return_d.enabled = false; }

Location::~Location() {}

// Setters
void Location::addAllowedMethod(const std::string &method) { allowed_methods.push_back(method); }

void Location::setRoot(const std::string &root) { this->root = root; }

void Location::setAutoindex(bool autoindex) { this->autoindex = autoindex; }

void Location::addIndex(const std::string &index) { this->index.push_back(index); }

void Location::setClientBodyLimit(size_t limit) { client_body_limit = limit; }

void Location::setCgiExtension(const std::string &ext,
                               const std::string &interpreter) { cgi_info[ext] = interpreter; }

void Location::setReturn(const std::string &return_url) { this->return_url = return_url; }

void Location::setUploadStore(const std::string &path) { upload_store = path; }

void Location::addReturnDirective(const ReturnDirective rd) { return_d = rd; }

void Location::inheritFrom(const Server *server)
{
    if (root.empty())
        root = server->getRoot();
    if (client_body_limit == 0)
        client_body_limit = server->getClientBodyLimit();
    if (index.empty())
        index = server->getIndex();
}

// Getters
const ReturnDirective Location::getReturnDirective() const { return return_d; }

const std::string &Location::getPath() const { return path; }

const std::vector<std::string> &Location::getAllowedMethods() const { return allowed_methods; }

const std::string &Location::getRoot() const { return root; }

bool Location::getAutoindex() const { return autoindex; }

const std::vector<std::string> &Location::getIndex() const { return index; }

size_t Location::getClientBodyLimit() const { return client_body_limit; }

const std::string &Location::getReturn() const { return return_url; }

const std::string &Location::getUploadStore() const { return upload_store; }

const std::string &Location::getCgiInfo(const std::string &ext) const
{
    static const std::string empty;
    std::map<std::string, std::string>::const_iterator it = cgi_info.find(ext);
    return it != cgi_info.end() ? it->second : empty;
}

Server::Server()
    : client_body_limit(0), autoindex(false)
{
    listen.push_back(std::make_pair("0.0.0.0", "8000"));
    return_d.enabled = false;
}

Server::~Server()
{
    for (size_t i = 0; i < locations.size(); ++i)
        delete locations[i];
}

// Setters
void Server::addAllowedMethod(const std::string &method) { allowed_methods.push_back(method); }

void Server::setListen(std::pair<std::string, std::string> host_port) { listen.push_back(host_port); }

void Server::addServerName(const std::string &name) { server_names.push_back(name); }

void Server::addErrorPage(int code, const std::string &path) { error_pages[code] = path; }

void Server::addReturnDirective(const ReturnDirective rd) { return_d = rd; }

void Server::addLocation(Location *location) { locations.push_back(location); }

void Server::setRoot(const std::string &root) { this->root = root; }

void Server::setClientBodyLimit(size_t limit) { client_body_limit = limit; }

void Server::setAutoindex(bool autoindex) { this->autoindex = autoindex; }

void Server::addIndex(const std::string &index) { this->index.push_back(index); }

// Getters
const std::vector<std::string> &Server::getAllowedMethods() const { return allowed_methods; }

const ReturnDirective Server::getReturnDirective() const { return return_d; }

const std::vector<std::pair<std::string, std::string>>& Server::getListen() const { return listen; }

const std::vector<std::string> &Server::getServerNames() const { return server_names; }

const std::vector<Location *> &Server::getLocations() const { return locations; }

const std::string &Server::getRoot() const { return root; }

size_t Server::getClientBodyLimit() const { return client_body_limit; }

bool Server::getAutoindex() const { return autoindex; }

const std::vector<std::string> &Server::getIndex() const { return index; }

const std::string &Server::getErrorPage(int code) const
{
    static const std::string empty;
    std::map<int, std::string>::const_iterator it = error_pages.find(code);
    return it != error_pages.end() ? it->second : empty;
}

int main()
{
    Server dummyServer;
    ParseRequest parser(&dummyServer);

    std::string request =
    "POST /api/data HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "31\r\n"
    "{\"name\": \"John Doe\", "
    "\"email\": \"john@example.com\"}\r\n"
    "0\r\n"
    "\r\n";
    
    parser.startParse(1);
    std::cout << "Method: " << parser.getMethod() << std::endl;
    std::cout << "URL: " << parser.getUri() << std::endl;
    std::cout << "Version: " << parser.getVersion() << std::endl;
    for (size_t i = 0; i < parser.Headers.size(); ++i)
    {
        std::cout << "Header[" << i << "]: " << parser.Headers[i].first
                  << " => " << parser.Headers[i].second << std::endl;
    }
    std::cout << "Body :   "  << parser.BufferBody << "\n";
    std::cout << parser.errorNumber << "\n";
    std::cerr << "Failed to parse. Error state: " << parser.getParseState() << std::endl;
    std::cerr << "error number is : " << parser.errorNumber << "\n";
    return 0;
}
