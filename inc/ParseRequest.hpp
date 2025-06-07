#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
#include <sstream>
#include "Lexer.hpp"
#include <utility>
#include "Server.hpp"

enum RequestParseStats
{
    METHOD,
    URL,
    HTTPVERSION,
    HEADERS,
    VLIDATEHEADRES,
    CONTENTLENGTHBODY,
    READCHUNKSIZE,
    READCHUNK,
    FINISH,
    ERROR,
    NONE
};

class ParseRequest{
public:
    ParseRequest();
    ParseRequest(Server *server, int fd);
    ~ParseRequest();

    int errorNumber;
    size_t pos;
    int CurrntParsState;
    std::string Method;
    std::string Url;
    std::string HttpProtocolVersion;
    std::pair<std::string, std::string> QuerieStrings;
    Server *S;
    int ServerSocketFd;
    bool chunkedEncoding;
    size_t contentLength;
    std::vector<std::pair<std::string, std::string> > Headers;
    bool hasValidHost;
    size_t ChunkSize;
    std::string BufferBody;
    std::map<std::string, int> NonRepeatablesHeaders;
    std::string Host;
    std::string Port;
    std::string QueryString;
    std::istringstream incomigBytes;
    
    
    // parse input
    void startParse(std::string& buff);
    void parseMethod(std::string &str);
    void parseUrl(std::string &str);
    void parseHttpVersion(std::string &str);
    void parseHeaders(std::string &str);
    void CheckingForBody();
    void parseContentlengthBody(std::string &str);
    int HexaStringToDecimalNum(std::string s);
    void parseChunkedBody(std::string &str);
    void trimBuff(std::string &str);
    void toLowerCase(std::string &key);
    bool isAllSpaces(std::string &str);
    bool validKey(std::string &key);
    bool checkIsThereaHost();
    bool isNumber(std::string toCheck);

    // checkers
    bool isFinish();
    bool isSupportedMethod(std::string &RequestMethod);
    int isKnownMethod();
    bool isValidUrl();
    bool isHexa(char characetre);
    bool Unresreved(char c);
    bool Reserved(char c);
    bool PercentEncoded(size_t &i);
    bool isValidVersion();

    // getters
    std::string getMethod();
    std::string getUri();
    std::string getVersion();
    int getParseState();

    // setters
    void setMethod(std::string m);
    void setUri(std::string u);
    void setVersion(std::string v);
    void SwitchState(int Next_State);
    void setErrorNumber(int Number);
    void setQueryString(std::string qurieInUrl);
    void Reset();
    void ResetBuffPos();
};




#endif