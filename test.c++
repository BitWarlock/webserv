

#include <iostream>
#include <sstream>
#include "inc/Lexer.hpp"
#include <utility>
#include "inc/Server.hpp"

enum RequestParseStats{
    METHOD,
    URL,
    HTTPVERSION,
    HEADERS,
    VLIDATEHEADRES,
    BODYS,
    FINISH,
    ERROR,
    NONE
};


class ParseRequest {
    public:
        ParseRequest();
        ParseRequest(Server *server, int fd);
        ~ParseRequest();

        int     ServerSocketFd;
        int     pos;
        bool    hasValidHost;
        size_t  contentLenght;
        bool    chunkedEncoding;
        std::string BufferBody;
        std::map<std::string, int> NonRepeatablesHeaders;
        std::string Host;
        std::string Port;
        Server *S;
        std::string QueryeString;
        std::istringstream incomigBytes;

        //parse input
        void        startParse(std::string buff);
        void        parseMethod(std::string& str);
        void        parseUrl(std::string& str);
        void        parseHttpVersion(std::string& str);
        void        parseHeaders(std::string& str);
        void        CheckingForBody();
        void        parseBody(std::string& str);
        void        trimBuff(std::string& str);
        void        toLowerCase(std::string& key);
        bool        isAllSpaces(std::string& str);
        bool        validKey(std::string& key);
        bool        checkIsThereaHost();
        bool        isNumber(std::string toCheck);

        //checkers
        bool        isFinish();
        bool        itHasBody(std::map<std::string, std::string> Hdrs);
        bool        isSupportedMethod(std::string& RequestMethod);
        int         isKnownMethod();
        bool        isValidUrl();
        bool        isHexa(char characetre);
        bool        Unresreved(char c);
        bool        Reserved(char c);
        bool        PercentEncoded(int& i);
        bool        isValidVersion();

        //getters
        std::string getMethod();
        std::string getUri();
        std::string getVersion();
        int         getParseState();

        //setters
        void        setMethod(std::string m);
        void        setUri(std::string u);
        void        setVersion(std::string v);
        void        SwitchState(int Next_State);
        void        setErrorNumber(int Number);
        void        setQueryString(std::string qurieInUrl);
        void        Reset();
        void        ResetBuffPos();



        int                                 errorNumber;
        int                                 CurrntParsState;
        std::string                         Method;
        std::string                         Url;
        std::string                         HttpProtocolVersion;
        std::pair<std::string, std::string>  QuerieStrings;
        std::vector<std::pair<std::string, std::string>>  Headers;


};


































ParseRequest::ParseRequest(): errorNumber(0), pos(0), CurrntParsState(NONE), Method(""), Url(""),
                HttpProtocolVersion(""), QuerieStrings("",""), chunkedEncoding(false), contentLenght(0){
    BufferBody.clear();
    NonRepeatablesHeaders["host"] = 0;
    NonRepeatablesHeaders["content-lenght"] = 0;
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
ParseRequest::ParseRequest(Server *server, int fd): errorNumber(0), pos(0), CurrntParsState(NONE), Method(""), Url(""),
                HttpProtocolVersion(""), QuerieStrings("",""), S(server), ServerSocketFd(fd), chunkedEncoding(false), contentLenght(0){
    BufferBody.clear();
    NonRepeatablesHeaders["host"] = 0;
    NonRepeatablesHeaders["content-lenght"] = 0;
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
//distructor 
ParseRequest::~ParseRequest(){}


//getters
std::string ParseRequest::getMethod(){return (Method);}
std::string ParseRequest::getVersion(){return (HttpProtocolVersion);}
std::string ParseRequest::getUri(){return(Url);}
int         ParseRequest::getParseState(){return (CurrntParsState);}

//setters
void        ParseRequest::setMethod(std::string m){Method = m;}
void        ParseRequest::SwitchState(int Next_State){CurrntParsState = Next_State;}
void        ParseRequest::setUri(std::string u){Url = u;}
void        ParseRequest::setVersion(std::string v){ HttpProtocolVersion = v;}
void        ParseRequest::Reset(){SwitchState(NONE);}
void        ParseRequest::setErrorNumber(int Number){errorNumber = Number; SwitchState(ERROR);}
void        ParseRequest::ResetBuffPos(){ pos = 0;}


//checkers
bool        ParseRequest::isFinish(){return (getParseState() == FINISH);}
bool        ParseRequest::isSupportedMethod(std::string& RequestMethod){
    const std::vector<std::string>& sMethods = S->getAllowedMethods();
   std::string m = getMethod();
   for (int i = 0; i < sMethods.size();i++){
    if (sMethods[i] == m)
    return (true);
   }
   return (false);
}
int         ParseRequest::isKnownMethod(){
    if (Method == "GET" || Method == "POST" || Method == "DELETE"
        || Method == "HEAD" || Method == "PUT" || Method == "OPTIONS"
        || Method == "TRACE" || Method == "CONNECT"){
            return (501);
        }
    return (405);
}
bool        ParseRequest::isHexa(char characetre){ return (isxdigit(characetre));}
bool        ParseRequest::Unresreved(char c){
    std::string s = "-._~";
    return (std::isalnum(c) || (s.find(c) != std::string::npos));
}
bool        ParseRequest::Reserved(char c){
    std::string ResevedCharacters = ":/?#[]@!$&'()*+,;=";
    return (ResevedCharacters.find(c) != std::string::npos);
}
bool        ParseRequest::PercentEncoded(int& i){
    if (i+2 <= Url.size() && isHexa(Url[i+1]) && isHexa(Url[i+2]))
        return (i += 2, true);
    return (false);
}
bool        ParseRequest::isValidUrl(){
    if (Url.size() >= 8192)
        return (SwitchState(ERROR), setErrorNumber(414), false);
    if (Url.empty() || Url[0] != '/')
        return (SwitchState(ERROR), setErrorNumber(400), false);
    for (int i(0);i < Url.size();i++){
        if ((Url[i] == '%' && !PercentEncoded(i)) || (!Unresreved(Url[i]) && !Reserved(Url[i])))
            return (SwitchState(ERROR), setErrorNumber(400), false);
    }
    return (true);
}

//parsers
void        ParseRequest::toLowerCase(std::string& key){
    for (int i(0); i < key.size()-1;i++){
        std::tolower((unsigned char)key[i]);
    }
}

void        ParseRequest::trimBuff(std::string& str){
    int pos = str.find_first_not_of(" ");
    if (pos)
        str.erase(0, pos - 1);
}

void ParseRequest::parseMethod(std::string& str){
    if (isspace(str[0]))
        return (SwitchState(ERROR), setErrorNumber(400));
    pos = str.find(' ');
    if (pos == std::string::npos){
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (SwitchState(ERROR), setErrorNumber(400));
        Method += str;
        str.clear();
        return (ResetBuffPos());
    }
    Method += str.substr(0, pos);
    str.erase(0, pos+1);
    ResetBuffPos();
    // if (!isSupportedMethod(Method))
    //     return (setErrorNumber(isKnownMethod()));
    SwitchState(URL);
}
void    ParseRequest::setQueryString(std::string queryInUrl){QueryeString = queryInUrl;}
void ParseRequest::parseUrl(std::string& str){
    trimBuff(str);
    pos = str.find(' ');
    if (pos == std::string::npos){
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (SwitchState(ERROR), setErrorNumber(400));
        Url.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }
    Url.append(str.substr(0, pos));
    str.erase(0, pos+1);
    if (!isValidUrl())
        return ;
    if ((pos = Url.find('?')) != std::string::npos){
        setQueryString(Url.substr(pos+1, Url.size()));
        Url.erase(pos);
    }
    ResetBuffPos();
    SwitchState(HTTPVERSION);
}
bool        ParseRequest::isValidVersion(){
    if (HttpProtocolVersion == "HTTP/1.1")
        return (true);
    if (HttpProtocolVersion == "HTTP/1.0" || HttpProtocolVersion == "HTTP/0.9" ||
    HttpProtocolVersion == "HTTP/2" || HttpProtocolVersion == "HTTP/3")
        return (SwitchState(ERROR), setErrorNumber(505), false);
    return (SwitchState(ERROR), setErrorNumber(400), false);
}

void ParseRequest::parseHttpVersion(std::string& str){
    trimBuff(str);
    pos = str.find("\r\n");
    if (pos == std::string::npos){
        HttpProtocolVersion.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }
    if (isspace(str[pos-1]))
        return (SwitchState(ERROR), setErrorNumber(400));
    HttpProtocolVersion.append(str.substr(0, pos));
    str.erase(0, pos+2);
    if (!isValidVersion())
        return;
    ResetBuffPos();
    SwitchState(HEADERS);
}
bool        ParseRequest::isAllSpaces(std::string& str){
    for (int i(0); i < str.size()-1;i++){
        if (!isspace(str[i]))
            return (false);
    }
    return (true);
}
bool        ParseRequest::validKey(std::string& key){
    std::string visbleChar = "!#$%&'*+-.^_|~`"; 
    for (int i(0); i < key.size()-1; i++){
        if (!isalnum(key[i]) && !visbleChar.find(key[i]))
            return false;
    }
    return true;
}
bool    ParseRequest::checkIsThereaHost(){
    std::string hoststring(NULL);
    for (int i(0); i < Headers.size();i++){
        if (Headers[i].first == "host" && !Headers[i].second.empty()){
            int p = Headers[i].second.find(':');
            if (p != std::string::npos){
                Host = Headers[i].second.substr(0, p);
                Port = Headers[i].second.substr(p+1);
            }
            else
                Host = Headers[i].second;
            hasValidHost = true;
            return (true);
        }
    }
    return (SwitchState(ERROR), setErrorNumber(400), false);
}
void ParseRequest::parseHeaders(std::string& str){
    pos = str.find("\r\n");
    if (pos == std::string::npos)
        return ;
    std::string line = str.substr(0, pos);
    str.erase(0, pos+2);
    if (line.size() >= 8192)
        return (SwitchState(ERROR), setErrorNumber(431));
    pos = line.find(':');
    if (pos == std::string::npos || isspace(line[pos-1]))
        return (SwitchState(ERROR), setErrorNumber(400));
    std::string key = line.substr(0,pos);
    std::string value = line.substr(pos+1);
    trimBuff(value);
    if (!validKey(key) || key.empty() || isAllSpaces(key) || value.empty())
        return (SwitchState(ERROR), setErrorNumber(400));
    toLowerCase(key);
    std::map<std::string, int>::iterator it = NonRepeatablesHeaders.find(key);
    if (it != NonRepeatablesHeaders.end()){
        if (it->second)
            return (SwitchState(ERROR), setErrorNumber(400));
        it->second++;
    }
    Headers.push_back(std::make_pair(key,value));
    ResetBuffPos();
    if (str[0] == '\r' && str[1] =='\n'){
        str.erase(0,2);
        if (!(checkIsThereaHost()))
            return( SwitchState(ERROR), setErrorNumber(400));
        CheckingForBody(); 
        return ;
    }
    if (getParseState() == HEADERS)
        parseHeaders(str);
}


bool        ParseRequest::isNumber(std::string toCheck){
    for (int i(0); i < toCheck.size();i++){
        if (!isdigit(toCheck[i]))
            return (false);
    }
    return (true);
}
void ParseRequest::CheckingForBody(){
    bool TransferEncodingPresent = false;
    bool ContentlenghtPresent = false;
    if (Method == "GET"|| Method == "DELETE")
        return (SwitchState(FINISH)); 
    std::vector<std::pair<std::string, std::string>>::iterator  Headersit;
    for (Headersit = Headers.begin(); Headersit != Headers.end(); Headersit++){
        if (Headersit->first == "transfer-encoding"){
            TransferEncodingPresent = true;
            std::string tmp = Headersit->second;
            toLowerCase(tmp);
            if (tmp == "chunked")
                chunkedEncoding = true;
            else
                return (SwitchState(ERROR), setErrorNumber(501));
        }
        if (Headersit->first == "content-length"){
            ContentlenghtPresent = true;
            if (!isNumber(Headersit->second))
                return (SwitchState(ERROR), setErrorNumber(400));
            contentLenght = std::stoi(Headersit->second);
            if (contentLenght < 0)
                return (SwitchState(ERROR), setErrorNumber(400));
            if ((size_t)contentLenght > S->getClientBodyLimit())
                return (SwitchState(ERROR), setErrorNumber(413));
        }
        SwitchState(BODYS);
    }
    if (TransferEncodingPresent && ContentlenghtPresent)
    return (SwitchState(ERROR), setErrorNumber(400));
    if (!chunkedEncoding && !ContentlenghtPresent)
        return (SwitchState(FINISH));
}
void       ParseRequest::parseBody(std::string& str){
    if (contentLenght ){
        size_t CurrentNumBytes = 0;
        while (CurrentNumBytes < contentLenght){
            CurrentNumBytes += str.size();
            BufferBody.append(str);
            str.erase(0, CurrentNumBytes);
            if (CurrentNumBytes > contentLenght)
                return (SwitchState(ERROR), setErrorNumber(400));
        }
    }
    else if (chunkedEncoding){

    }
}

void    ParseRequest::startParse(std::string buff){
    if (CurrntParsState == NONE && !buff.empty())
        SwitchState(METHOD);
    if(CurrntParsState == METHOD && !buff.empty())
        parseMethod(buff);
    if (CurrntParsState == URL && !buff.empty())
        parseUrl(buff);
    if (CurrntParsState == HTTPVERSION && !buff.empty())
        parseHttpVersion(buff);
    if (CurrntParsState == HEADERS && !buff.empty())
        parseHeaders(buff);
    if (CurrntParsState == BODYS && !buff.empty())
        parseBody(buff);
    if (CurrntParsState == ERROR)
        return ;

}




























Location::Location(const std::string& path) 
	: path(path), autoindex(false), client_body_limit(0) {return_d.enabled = false;}

Location::~Location() {}

	// Setters
void Location::addAllowedMethod(const std::string& method) {allowed_methods.push_back(method);}

void Location::setRoot(const std::string& root) {this->root = root;}

void Location::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Location::addIndex(const std::string& index) {this->index.push_back(index);}

void Location::setClientBodyLimit(size_t limit) {client_body_limit = limit;}

void Location::setCgiExtension(const std::string& ext,
		const std::string& interpreter) {cgi_info[ext] = interpreter;}

void Location::setReturn(const std::string& return_url) {this->return_url = return_url;}

void Location::setUploadStore(const std::string& path) {upload_store = path;}

void Location::addReturnDirective(const ReturnDirective rd) {return_d = rd;}

void Location::inheritFrom(const Server* server)
{
	if (root.empty())
		root = server->getRoot();
	if (client_body_limit == 0)
		client_body_limit = server->getClientBodyLimit();
	if (index.empty())
		index = server->getIndex();
}

// Getters
const ReturnDirective	Location::getReturnDirective() const {return return_d;}

const std::string& Location::getPath() const {return path;}

const std::vector<std::string>& Location::getAllowedMethods() const {return allowed_methods;}

const std::string& Location::getRoot() const {return root;}

bool Location::getAutoindex() const {return autoindex;}

const std::vector<std::string>& Location::getIndex() const {return index;}

size_t Location::getClientBodyLimit() const {return client_body_limit;}

const std::string& Location::getReturn() const {return return_url;}

const std::string& Location::getUploadStore() const {return upload_store;}

const std::string& Location::getCgiInfo(const std::string& ext) const
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
void Server::addAllowedMethod(const std::string& method) {allowed_methods.push_back(method);}

void Server::setListen(std::pair<std::string, std::string> host_port) {listen.push_back(host_port);}

void Server::addServerName(const std::string& name) {server_names.push_back(name);}

void Server::addErrorPage(int code, const std::string& path) {error_pages[code] = path;}

void Server::addReturnDirective(const ReturnDirective rd) {return_d = rd;}

void Server::addLocation(Location* location) {locations.push_back(location);}

void Server::setRoot(const std::string& root) {this->root = root;}

void Server::setClientBodyLimit(size_t limit) {client_body_limit = limit;}

void Server::setAutoindex(bool autoindex) {this->autoindex = autoindex;}

void Server::addIndex(const std::string& index) {this->index.push_back(index);}

// Getters
const std::vector<std::string>& Server::getAllowedMethods() const {return allowed_methods;}

const ReturnDirective	Server::getReturnDirective() const {return return_d;}

const std::vector<std::pair<std::string, std::string> > Server::getListen() const {return listen;}

const std::vector<std::string>& Server::getServerNames() const {return server_names;}

const std::vector<Location*>& Server::getLocations() const {return locations;}

const std::string& Server::getRoot() const {return root;}

size_t Server::getClientBodyLimit() const {return client_body_limit;}

bool Server::getAutoindex() const {return autoindex;}

const std::vector<std::string>& Server::getIndex() const {return index;}

const std::string& Server::getErrorPage(int code) const
{
	static const std::string empty;
	std::map<int, std::string>::const_iterator it = error_pages.find(code);
	return it != error_pages.end() ? it->second : empty;
}


int main() {
    Server dummyServer;
    ParseRequest parser(&dummyServer, 1);

    std::string request =
        "GET /index.html?lang=en HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: curl/7.68.0\r\n"
        "Accept: */*\r\n"
        "ddpt: */ddddd*\r\n"
        "Acc: hola\r\n"
        "ept: nqeodcl*/*\r\n"
        "\r\n";

    parser.startParse(request);
    if (parser.getParseState() == BODYS) {
        std::cout << "Method: " << parser.getMethod() << std::endl;
        std::cout << "URL: " << parser.getUri() << std::endl;
        std::cout << "Version: " << parser.getVersion() << std::endl;

        for (size_t i = 0; i < parser.Headers.size(); ++i) {
            std::cout << "Header[" << i << "]: " << parser.Headers[i].first
                      << " => " << parser.Headers[i].second << std::endl;
        }
        std::cout << parser.errorNumber << "\n";
    } else {
        std::cerr << "Failed to parse. Error state: " << parser.getParseState() << std::endl;
        std::cerr << "error number is : " << parser.errorNumber << "\n";
    }   
    return 0;
}


