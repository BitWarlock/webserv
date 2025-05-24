#include "ParseRequest.hpp"
#include "Server.hpp"

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
bool        ParseRequest::Unresreved(char c){return (std::isalnum(c));}
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
        if ((Url[i] == '%' && !PercentEncoded(i)) || !Unresreved(Url[i]) && !Reserved(Url[i]))
            return (SwitchState(ERROR), setErrorNumber(400), false);
    }
    return (true);
}

//parser
void        ParseRequest::trimBuff(std::string& str){
    int pos = str.find_first_not_of(" ");
    if (pos)
        str.erase(0, pos - 1);
}

void ParseRequest::parseMethod(std::string& str){
    trimBuff(str);
    pos = str.find(' ');
    if (pos == std::string::npos){
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
}

void ParseRequest::parseHttpVersion(std::string& str){
    
}
void ParseRequest::parseHeaders(std::string& str){

}
void ParseRequest::parseBody(std::string& str){

}

void    ParseRequest::startParse(std::string buff){
    buff.substr(buff.find_first_not_of(" "));
    if (CurrntParsState == NONE && !buff.empty()){
        SwitchState(METHOD);
    }
    if(CurrntParsState == METHOD && !buff.empty())
        parseMethod(buff);
    if (CurrntParsState == URL && !buff.empty())
        parseUrl(buff);
}
