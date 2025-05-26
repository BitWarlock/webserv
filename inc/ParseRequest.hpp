#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
#include <sstream>
#include "Lexer.hpp"
#include <utility>


enum RequestParseStats{
    METHOD,
    URL,
    HTTPVERSION,
    HEADERS,
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

        Server *S;
        int     ServerSocketFd;
        int     pos;
        std::string QueryeString;
        std::istringstream incomigBytes;

        //parse input
        void        startParse(std::string buff);
        void        parseMethod(std::string& str);
        void        parseUrl(std::string& str);
        void        parseHttpVersion(std::string& str);
        void        parseHeaders(std::string& str);
        void        parseBody(std::string& str);
        void        trimBuff(std::string& str);
        void        toLowerCase(std::string& key);
        bool        isAllSpaces(std::string& str);
    
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

    private:

        int                                 errorNumber;
        int                                 CurrntParsState;
        std::string                         Method;
        std::string                         Url;
        std::string                         HttpProtocolVersion;
        std::pair<std::string, std::string>  QuerieStrings;
        std::vector<std::pair<std::string, std::string>>  Headers;


};


#endif