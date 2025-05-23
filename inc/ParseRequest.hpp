#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
#include <sstream>
#include "Lexer.hpp"


enum RequestParseStats{
    REQUEST_LINE,
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
        std::istringstream incomigBytes;

        //parse input
        void        startParse(std::string buff);
    
        //checkers
        bool        isFinish();
        bool        isSupportedMethod(std::string& RequestMethod);

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
        void        Reset();

    private:

        int                                 CurrntParsState;
        std::string                         Method;
        std::string                         Url;
        std::string                         HttpProtocolVersion;
        std::map<std::string, std::string>  QuerieStrings;
        std::map<std::string, std::string>  Headers;

};


#endif