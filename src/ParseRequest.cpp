#include "../inc/ParseRequest.hpp"
#include "../inc/Server.hpp"

// construct
ParseRequest::ParseRequest(): pos(0), CurrntParsState(NONE), Method(""), Url(""),
                HttpProtocolVersion(""), QuerieStrings("",""){}
// construct with params 
ParseRequest::ParseRequest(Server *server, int fd): pos(0), CurrntParsState(NONE), Method(""), Url(""),
                HttpProtocolVersion(""), QuerieStrings("",""), S(server), ServerSocketFd(fd){}
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
    trimBuff(str);
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
    if (!isSupportedMethod(Method))
        return (setErrorNumber(isKnownMethod()));
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

void ParseRequest::parseHeaders(std::string& str){
    trimBuff(str);
    pos = str.find("\r\n\r\n");
    pos = str.find("\r\n");
    if (pos == std::string::npos)
        return ;
    str.erase(pos+2);
    pos = str.find(':');
    if (pos == std::string::npos || isspace(str[pos-1]))
        return (SwitchState(ERROR), setErrorNumber(400));
    std::string key = str.substr(0,pos);
    std::string value = str.substr(pos+1, str.size());
    trimBuff(str);
    if (key.empty() || isAllSpaces(key) || value.empty())
        return (SwitchState(ERROR), setErrorNumber(400));
    toLowerCase(key);
    Headers.push_back(std::make_pair(key,value));
    ResetBuffPos();
    parseHeaders(str);
}
void ParseRequest::parseBody(std::string& str){

}

void    ParseRequest::startParse(std::string buff){
    buff.substr(buff.find_first_not_of(" "));
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
         
}
