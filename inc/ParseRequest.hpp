#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
#include <sstream>
#include "Lexer.hpp"
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <zlib.h>
//#include <brotli/decode.h>
#include "Server.hpp"
#define CLRF "\r\n"
#define SPACE ' '

enum RequestParser
{
    NONE,
    METHOD,
    URL,
    HTTPVERSION,
    HEADERS_KEY,
    HEADER_VALUE,
    ADD_HEADER,
    CONTENTLENGTHBODY,
    READCHUNKSIZE,
    READCHUNK,
	READ_BOUNDARY,
	READ_MULTIPART_BODY,
    PARSEARRAYSIZE,
    FINISH,
    ERROR,
    CLOSE,
    GZIP,
    DEFLATE,
    IDENTITY
};

class ParseRequest{
    public:
        ParseRequest();
        ParseRequest(Server *server);
        // ParseRequest(ParseRequest& other);
        // ParseRequest& operator=(const ParseRequest& other);
        ~ParseRequest();

    private:
        int                                                 errorNumber;
        size_t                                              pos;
        int                                                 CurrntParsState;
        std::string                                         Current_key;
        std::string                                         Current_value;
        std::string                                         Current_header_line;
        std::string                                         Method;
        std::string                                         Url;
        std::string                                         HttpProtocolVersion;
        Server                                              *S;
        bool                                                chunkedEncoding;
        int                                                 contentLength;
		int													lastChunksize;
        int                                                 ContentEncodingType;
        std::vector<std::pair<std::string, std::string> >   Headers;
        bool                                                hasValidHost;
        size_t                                              ChunkSize;
        std::string                                         BufferBody;
        std::string                                         MultipartBoundary;
        std::vector<std::string >                           MultipartBufferBody;
        std::string                                         DecompressedBufferBody;
        std::map<std::string, int>                          NonRepeatablesHeaders;
        std::string                                         Host;
        std::string                                         Port;
        std::string                                         QueryString;
        typedef void                                        (ParseRequest::*ParseFuncPtr)(std::string& buffer);
        static  const ParseFuncPtr                          ParseTable[];


    public:
        // parse input
        void        startParse(int fd, Server server);
        void        StartNewRequest(std::string& buff);
        void        parseMethod(std::string& str);
        void        parseUrl(std::string& str);
        void        parseHttpVersion(std::string& str);
        void        parseHeaders(std::string& str);
        void        CheckingForBody();
        void        parseContentlengthBody(std::string &str);
        void        parseChunkedBody(std::string &str);
        int         HexaStringToDecimalNum(std::string s);
        void        trimBuff(std::string &str);
        void        toLowerCase(std::string &key);
        bool        isAllSpaces(std::string &str);
        bool        validKey(std::string &key);
        bool        checkIsThereaHost();
        bool        isNumber(std::string toCheck);
        void        ResetParserf();
        void        DecompressBody();
        void		ParseMultipartBodyBoundary(std::string &None);
        void        ParseMultiPartBufferBody(std::string &None);

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
        void        CheckContentEncoding();

        // getters
        std::string&                                        getMethod();
        std::string&                                        getUri();
        std::string&                                        getVersion();
        int                                                 getParseState();
        std::string                                         getHeaderValue(std::string key);
        int                                                 getErrorNumber();
        std::vector<std::pair<std::string, std::string> >&  getHeaders();
        std::string&                                        getHost();
        std::string&                                        getPort();
        int                                                 getContentEncodingType(int Type);

        std::string&                                        getQueryString(void);
        std::string&                                        getBufferBody(void);
        size_t                                              getContentLength(void);
        std::string&                                        getBufferDecompressedBody();
        const std::vector<std::string>&                     getMatchedLocationAllowedMethods();
        std::vector<std::string >&							getMultipartBuferBody();
        int                                                 getMatchedLocationBodySizeMax();

        // setters
        void        setMethod(std::string m);
        void        setUri(std::string u);
        void        setVersion(std::string v);
        void        SwitchState(int Next_State);
        void        setErrorNumber(int Number);
        void        setQueryString(std::string qurieInUrl);
        void        Reset();
        void        ResetBuffPos();
        void        setContentEncodingType(int Type);
};

#endif