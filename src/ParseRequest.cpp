#include "../inc/ParseRequest.hpp"
#include "../inc/Server.hpp"


// construct
ParseRequest::ParseRequest(): errorNumber(0), pos(0), CurrntParsState(NONE), Method(""), Url(""),
                HttpProtocolVersion(""), QuerieStrings("",""), chunkedEncoding(false), contentLenght(0){
    BufferBody.clear();
    ChunkSize = 0;
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
    ChunkSize = 0;
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
bool        ParseRequest::validKey(std::string& key){
    std::string visbleChar = "!#$%&'*+-.^_|~`"; 
    for (int i(0); i < key.size()-1; i++){
        if (!isalnum(key[i]) && !visbleChar.find(key[i]))
            return false;
    }
    return true;
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
        if (!checkIsThereaHost)
            return( SwitchState(ERROR), setErrorNumber(400));
        CheckingForBody(); 
        return ;
    }
    if (getParseState() == HEADERS)
        parseHeaders(str);
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
    }
    if (TransferEncodingPresent && ContentlenghtPresent)
        return (SwitchState(ERROR), setErrorNumber(400));
    if (!chunkedEncoding && !ContentlenghtPresent)
        return (SwitchState(FINISH));
    if (chunkedEncoding)
        return SwitchState(CHUNKEDBODY);
    SwitchState(CONTENTLENGTHBODY);
}
void       ParseRequest::parseContentlengthBody(std::string& str){
    BufferBody.append(str);
    contentLenght -= str.size();
    str.clear();
    if (contentLenght == 0)
        return (SwitchState(FINISH));
    else if (contentLenght < 0)
        return (SwitchState(ERROR), setErrorNumber(400));
}

int ParseRequest::HexaStringToDecimalNum(std::string s){
    std::istringstream stremstr(s);
    int ret = 0;
    stremstr >> std::hex >> ret;
    return ret;
}

void    ParseRequest::parseChunkedBody(std::string& str){
    pos = str.find("\r\n");
    if (pos != std::string::npos){
        std::string StringChunkSize  = str.substr(0, pos - 1);
        for (int i(0); i < StringChunkSize.size();i++){
            if (!isHexa(StringChunkSize[i]))
                return (SwitchState(ERROR), setErrorNumber(400));
        }
        ChunkSize = HexaStringToDecimalNum(StringChunkSize);
        if (!ChunkSize)
            return (SwitchState(FINISH));
        if (ChunkSize > S->getClientBodyLimit())
            return (SwitchState(ERROR), setErrorNumber(413));
        str.erase(0, pos + 2);
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
    if (CurrntParsState == CONTENTLENGTHBODY && !buff.empty())
        parseContentlengthBody(buff);
    if (CurrntParsState == CHUNKEDBODY && !buff.empty())
        parseChunkedBody(buff);
    if (CurrntParsState == ERROR || CurrntParsState == FINISH)
        return ;
}


