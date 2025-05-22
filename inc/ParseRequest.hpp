#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
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
        ~ParseRequest();

        std::string incomigBytes[4096];
        void        startParse(std::string buff);
        bool        isFinish();
        bool        isSupportedMethod();
        void        SwitchState();
        void        Reset();

        std::string getMethod();
        std::string getUri();
        std::string getVersion();
        int         getParseState();

        void        setMethod();
        void        setUri();
        void        setVersion();
        void        setParseState(); 

    private:
        int parsState;
        std::string Method;
        std::string Uri;
        std::string Version;
        std::string QuerieString;
        std::vector<std::string>  RequestLine;
        std::map<std::string, std::string> Headers;

};


#endif