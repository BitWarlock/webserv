#include "ParseRequest.hpp"
#include "Server.hpp"


ParseRequest::ParseRequest(): CurrntParsState(NONE), Method(""), Uri(""),
                HttpProtocolVersion(""), QuerieStrings("",""){
}

ParseRequest::ParseRequest(Server *server, int fd): CurrntParsState(NONE), Method(""), Uri(""),
                HttpProtocolVersion(""), QuerieStrings("",""), S(server), ServerSocketFd(fd){
}

ParseRequest::ParseRequest(){}

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

void    ParseRequest::startParse(std::string buff){
    
}
