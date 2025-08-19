
#ifndef PARSEREQUEST_HPP
#define PARSEREQUEST_HPP

#include <iostream>
#include <sstream>
#include "Lexer.hpp"
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <zlib.h>
// #include <brotli/decode.h>
#include "Server.hpp"
#include "Config.hpp"


#define READING_BUFFER_SIZE 5
#define HEADERS_ENDING "\r\n\r\n"
#define CLRF "\r\n"
#define SPACE ' '
#define RED "\033[1;31m"
#define YLW "\033[1;33m"
#define RESET "\033[0m"
#define GRN "\e[0;32m"

// ENUM_HOLDS_PARSING_MACHINE_STATES_____________________________

enum RequestParser
{
    PARSER_NONE,
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
    PARSE_COUCKIES,
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

        // CONSTRUCTORS_AND_DISTRUCTOR_________________________________

        ParseRequest();
        ParseRequest(Server *server);
        ~ParseRequest();

    private:

        // VARIABLES_____________________________________________________

    
        char                                                 ReadingBuffer[READING_BUFFER_SIZE + 1];
        std::string                                            Current_PrasingLine;
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
        size_t                                                lastBodyPos;
        int                                                 contentLength;
        int                                                 ContentEncodingType;
        std::vector<std::pair<std::string, std::string> >   Headers;
        bool                                                hasValidHost;
        size_t                                              ChunkSize;
        std::vector <char >                                 BufferBody;
        std::vector <char >                                    RequestBufferbody;
        std::string                                         MultipartBoundary;
        std::vector<std::vector <char > >                           MultipartBufferBody;
        std::vector <char >                                 DecompressedBufferBody;
        std::map<std::string, int>                          NonRepeatablesHeaders;
        std::string                                         Host;
        std::string                                         Port;
        std::string                                         QueryString;
        std::map<std::string, std::string>                    cookies;
        Location*                                           MatchLocation;
        
        // FUNCTOIONS_POINTER_PARSING_TABLES_____________________________________________________________

        typedef void                                        (ParseRequest::*ParseFuncPtr)(std::string& buffer);
        typedef void                                        (ParseRequest::*ParseBodyFuncPtr)(std::vector<char >& buffer);
        static  const ParseFuncPtr                          FirstParseTable[];
        static  const ParseBodyFuncPtr                      SecondParseTable[];


    public:
        // FINDING_THE_BLOCK_SERVER_FOR_REQUEST__________________________________________________________________

        Server*                                             findBlockServer(const Config& config, std::string buff, Server* server);
        
        
        // PARSING_FUNCTIONS________________________________________________

        void        startParse(int fd, const Config& config, Server* server);
        void        StartNewRequest(std::string& buff);
        void        parseMethod(std::string& str);
        void        parseUrl(std::string& str);
        void        parseHttpVersion(std::string& str);
        void        parseHeaders(std::string& str);
        void        CheckingForBody();
        void        parseContentlengthBody(std::vector <char > &str);
        void        parseChunkedBody(std::vector <char >& str);
        void        ParseMultipartBodyBoundary(std::vector <char > &None);
        void        ParseMultiPartBufferBody(std::vector <char > &None);
        void        parseCookies(std::vector <char >& None);
        void        ReadAndParseIntilHeadersFinish(std::string& buff, int fd, const Config& config, Server* server);
        void        RaedAndParseRequestBody(std::string& buff, int fd);
        size_t         findInVector(const std::vector<char>& haystack,
                                  const std::vector<char>& needle,
                                  size_t startPos);

        // PARSING_HELPERS_________________________________________________

        int         HexaStringToDecimalNum(std::string s);
        void        trimBuff(std::string &str);
        void        trimBuffTail(std::string &str);
        void        toLowerCase(std::string &key);
        bool        isAllSpaces(std::string &str);
        bool        validKey(std::string &key);
        bool        checkIsThereaHost();
        bool        isNumber(std::string toCheck);
        void        ResetParserf();
        void        DecompressBody();
        bool        isFinish(int RecvReturn);
        bool        isSupportedMethod(std::string &RequestMethod);
        int         isKnownMethod();
        bool        isValidUrl();
        bool        isHexa(char characetre);
        bool        Unresreved(char c);
        bool        Reserved(char c);
        bool        PercentEncoded(size_t &i);
        bool        isValidVersion();
        void        CheckContentEncoding();
        void        FindMatchLocation();

        // GETTER_FUNCTIONS__________________________________________________

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
        std::vector <char >&                                getBufferBody(void);
        size_t                                              getContentLength(void);
        std::vector <char >&                                getBufferDecompressedBody();
        const std::vector<std::string>&                     getMatchedLocationAllowedMethods();
        std::vector<std::vector <char > >&                    getMultipartBuferBody();
        int                                                 getMatchedLocationBodySizeMax();
        Server                                              getBlockServer();
        std::string                                         getCookie(const std::string& name) const;
        const std::map<std::string, std::string>&           getCookies() const;
        Location*                                           getMatchLocation();
        
        // SETTER_FUNCTIONS__________________________________________________

        void        setMethod(std::string m);
        void        setUri(std::string u);
        void        setVersion(std::string v);
        void        SwitchState(int Next_State);
        void        setErrorNumber(int Number, std::string ErrorMsg);
        void        setQueryString(std::string qurieInUrl);
        void        Reset();
        void        ResetBuffPos();
        void        setContentEncodingType(int Type);
};

#endif
