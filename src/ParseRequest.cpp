#include "../inc/ParseRequest.hpp"
#include "../inc/Server.hpp"


//_____________________________________________________________________________CONSTRUCTORS_____________________________________________________________________________

// construct
ParseRequest::ParseRequest() : errorNumber(200), pos(0),
    CurrntParsState(PARSER_NONE), Method(""), Url(""),
    HttpProtocolVersion(""),
    chunkedEncoding(false), contentLength(0), MatchLocation(NULL){

    BufferBody.clear();

    ChunkSize = 0;

    NonRepeatablesHeaders["host"] = 0;

    NonRepeatablesHeaders["content-length"] = 0;

    NonRepeatablesHeaders["content-type"] = 0;

    NonRepeatablesHeaders["content-encoding"] = 0;

    NonRepeatablesHeaders["transfer-encoding"] = 0;

    NonRepeatablesHeaders["authorization"] = 0;

    NonRepeatablesHeaders["user-agent\t"] = 0;

    NonRepeatablesHeaders["connection"] = 0;

    NonRepeatablesHeaders["date"] = 0;

    NonRepeatablesHeaders["upgrade"] = 0;

    NonRepeatablesHeaders["expect"] = 0;

    NonRepeatablesHeaders["range"] = 0;
}

// construct with params
ParseRequest::ParseRequest(Server *server) : errorNumber(200), pos(0),
    CurrntParsState(PARSER_NONE), Method(""), Url(""),
    HttpProtocolVersion(""),
    S(server), chunkedEncoding(false), contentLength(0), MatchLocation(NULL){

    BufferBody.clear();

    ChunkSize = 0;
    
    NonRepeatablesHeaders["host"] = 0;

    NonRepeatablesHeaders["content-length"] = 0;

    NonRepeatablesHeaders["content-type"] = 0;

    NonRepeatablesHeaders["content-encoding"] = 0;

    NonRepeatablesHeaders["transfer-encoding"] = 0;

    NonRepeatablesHeaders["authorization"] = 0;

    NonRepeatablesHeaders["user-agent    "] = 0;

    NonRepeatablesHeaders["connection"] = 0;

    NonRepeatablesHeaders["date"] = 0;

    NonRepeatablesHeaders["upgrade"] = 0;

    NonRepeatablesHeaders["expect"] = 0;

    NonRepeatablesHeaders["range"] = 0;
}


//_____________________________________________________________________________DISTRUCTOR_____________________________________________________________________________

ParseRequest::~ParseRequest() {}

//_____________________________________________________________________________PARSER_FUNCTIONS_____________________________________________________________________________

// tbale that hold apointers to parse functions That parse The REQUEST LINE AND HEADERS ;
const ParseRequest::ParseFuncPtr ParseRequest::FirstParseTable[] = {

    &ParseRequest::StartNewRequest,              //    NONE_ (so the parse will start)

    &ParseRequest::parseMethod,                 //    METHOD_

    &ParseRequest::parseUrl,                    //    URL_

    &ParseRequest::parseHttpVersion,             //    HTTPVERSION_

    &ParseRequest::parseHeaders,                //    HEADERS_KEY_

    &ParseRequest::parseHeaders,                //    HEDERS_VALUE_

    &ParseRequest::parseHeaders,                //    ADDING THE HEADER_
};



// tbale that hold apointers to parse functions that parse the REQUEST BODY ;
const ParseRequest::ParseBodyFuncPtr ParseRequest::SecondParseTable[] = {

    0,                                          //    NONE_ (so the parse will start)

    0,                                             //    METHOD_

    0,                                                //    URL_

    0,                                             //    HTTPVERSION_

    0,                                            //    HEADERS_KEY_

    0,                                            //    HEDERS_VALUE_

    0,                                            //    ADDING THE HEADER_

    &ParseRequest::parseContentlengthBody,        //    CONTENT_LENGHT_BODY_

    &ParseRequest::parseChunkedBody,            //    CHUNED_BODY_SIZE_

    &ParseRequest::parseChunkedBody,            //    READ_THE_CHUNK_

    &ParseRequest::ParseMultipartBodyBoundary,    //    READ_BOUNDARY_

    &ParseRequest::ParseMultiPartBufferBody,    //    READ_MULTYPART_BUFFER_BODY_

};

// start newrequest;
void ParseRequest::StartNewRequest(std::string& buff){
    (void)buff;

    SwitchState(METHOD);
}

// read and parse  the method ;
void ParseRequest::parseMethod(std::string &str){

    if (std::isspace(str[0]))
        return (setErrorNumber(400, "Bad Request – HTTP method cannot start with whitespace"));

    pos = str.find(SPACE);

    if (pos == std::string::npos)
    {
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (setErrorNumber(400, "Bad Request – Malformed request line or missing URI"));

        Method += str;
        str.clear();

        return (ResetBuffPos());
    }

    Method += str.substr(0, pos);

    str.erase(0, pos + 1);

    ResetBuffPos();

    SwitchState(URL);
}

// read and parse the url ;
void ParseRequest::parseUrl(std::string &str){

    trimBuff(str);

    pos = str.find(SPACE);

    if (pos == std::string::npos)
    {
        if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
            return (setErrorNumber(400, "Bad Request – Malformed URL or missing HTTP version"));

        Url.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }

    Url.append(str.substr(0, pos));

    str.erase(0, pos + 1);

    if (!isValidUrl()){
        return;
    }

    if ((pos = Url.find('?')) != std::string::npos)
    {
        setQueryString(Url.substr(pos + 1, Url.size()));
        Url.erase(pos);
    }

    ResetBuffPos();
    FindMatchLocation();
    if (!isSupportedMethod(Method)){

        int numberErr = isKnownMethod();

        if (numberErr == 501)
            return (setErrorNumber(numberErr, "Method Not Implemented – Method '" + Method + "' is not supported"));
        else if (numberErr == 405)
            return (setErrorNumber(numberErr, "Method Not Allowed – Method '" + Method + "' is not allowed for the requested URL"));
    }

    SwitchState(HTTPVERSION);
}

// read and parse the http request  version ;
void ParseRequest::parseHttpVersion(std::string &str){

    trimBuff(str);

    pos = str.find(CLRF);

    if (pos == std::string::npos)
    {
        HttpProtocolVersion.append(str);
        str.erase(0);
        return (ResetBuffPos());
    }

    if (std::isspace(str[pos - 1]))
        return (setErrorNumber(400, "Bad Request – Unexpected whitespace before CRLF in HTTP version"));

    HttpProtocolVersion.append(str.substr(0, pos));

    str.erase(0, pos + 2);

    if (!isValidVersion())
        return;

    ResetBuffPos();

    SwitchState(HEADERS_KEY);
}

// read and parse the request headers once at a time ;
void ParseRequest::parseHeaders(std::string &str){

    pos = str.find(CLRF);

    if (pos == std::string::npos)
        return;

    if (CurrntParsState == HEADERS_KEY){
        Current_header_line = str.substr(0, pos);

        str.erase(0, pos + 2);

        if (Current_header_line.size() >= 8192)
            return (setErrorNumber(431, "Request Header Fields Too Large – Header line exceeds 8192 bytes limit"));

        pos = Current_header_line.find(':');

        if (pos == std::string::npos || std::isspace(Current_header_line[pos - 1]))
            return (setErrorNumber(400, "Bad Request – Invalid header format (missing colon separator or whitespace before colon)"));

        Current_key = Current_header_line.substr(0, pos);

        toLowerCase(Current_key);

        SwitchState(HEADER_VALUE);
    }

    if (CurrntParsState ==  HEADER_VALUE){
        Current_value += Current_header_line.substr(pos + 1);

        trimBuff(Current_value);

        if (!validKey(Current_key) || Current_key.empty() || \
            isAllSpaces(Current_key) || Current_value.empty()){
            return (setErrorNumber(400, "Bad Request – Invalid header field (empty name, invalid characters, or missing value)"));
        }

        SwitchState(ADD_HEADER);
    }

    if (CurrntParsState == ADD_HEADER){
        std::map<std::string, int>::iterator it = NonRepeatablesHeaders.find(Current_key);

        if (it != NonRepeatablesHeaders.end())
        {
            if (it->second)
                return (setErrorNumber(400, "Bad Request – Duplicate header field (header cannot appear multiple times)"));
            it->second++;
        }

        Headers.push_back(std::make_pair(Current_key, Current_value));
        Current_key.clear();
        Current_value.clear();

        SwitchState(HEADERS_KEY);
    }

    ResetBuffPos();
}

// parse the content length body type ;
void ParseRequest::parseContentlengthBody(std::vector <char >& NONE){

    (void) NONE;
    if (contentLength == 0)
        return (SwitchState(FINISH));

    // BufferBody +=  str.substr(0);

    // str.erase(0, str.size());

    std::cout << GRN << contentLength << "     "<< RequestBufferbody.size() << RESET<<"\n\n\n\n";
    if (static_cast<size_t> (contentLength) > RequestBufferbody.size() ){
        return ;
    }

    if (static_cast<size_t> (contentLength) < RequestBufferbody.size())
        return (setErrorNumber(400, "Bad Request – Request body exceeds declared 'Content-Length' value"));

    if (static_cast<size_t> (contentLength) > BufferBody.size()){

        if (ContentEncodingType == GZIP || ContentEncodingType == DEFLATE){
            DecompressBody();
            BufferBody = getBufferDecompressedBody();
        }

        std::string contentType = getHeaderValue("Content-Type");

        if (getMethod() == "POST" && contentType.find("multipart/") != std::string::npos) {
            return SwitchState(READ_BOUNDARY);
        }
        
        return (SwitchState(FINISH));
    }
}

// parsing the chuncked body type ;
void ParseRequest::parseChunkedBody(std::vector<char >& str){

    if (CurrntParsState == READCHUNKSIZE)
    {
        std::vector<char>::iterator it = std::search(str.begin(), str.end(), CLRF, CLRF + 2);
        if (it != str.end())
            pos = it - str.begin();

        if (pos != std::string::npos)
        {
            std::string StringChunkSize(str.begin(), str.begin() + pos);

            for (size_t i(0); i < StringChunkSize.size(); i++)
            {
                if (!isHexa(StringChunkSize[i]))
                    return (setErrorNumber(400, "Bad Request – Invalid chunk size format (must be hexadecimal)"));
            }

            ChunkSize = HexaStringToDecimalNum(StringChunkSize);

            if (ChunkSize > S->getClientBodyLimit())
                return (setErrorNumber(413, "Payload Too Large – Chunk size exceeds maximum allowed limit"));
    
            str.erase(str.begin(), str.begin() + pos + 2);

            ResetBuffPos();

            if (ChunkSize == 0){
                if (ContentEncodingType == GZIP || ContentEncodingType == DEFLATE){
                    DecompressBody();
                    BufferBody = getBufferDecompressedBody();
                }

                if (getMethod() == "POST" && getHeaderValue("content-type").find("multipart/") != std::string::npos)
                    return SwitchState(READ_BOUNDARY);

                return (SwitchState(FINISH));
            }

            SwitchState(READCHUNK);
        }
    }

    if (CurrntParsState == READCHUNK)
    {
        if (str.size() >= ChunkSize + 2)
        {
            BufferBody.insert(BufferBody.end(), str.begin(), str.begin() + ChunkSize);
            str.erase(str.begin(), str.begin() + ChunkSize + 2);
            ChunkSize = 0;

            ResetBuffPos();

            SwitchState(READCHUNKSIZE);
        }
    }
}

void    ParseRequest::ParseMultipartBodyBoundary(std::vector<char >& None){
    (void ) None;
    std::cout << ";;;;;;;;;;;;;;;;;;;;;;;;;\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    std::string ContentTypeValue = getHeaderValue("content-type");

    pos = ContentTypeValue.find("multipart/");
    if (ContentTypeValue.empty() || pos == std::string::npos)
        return SwitchState(FINISH);

    if (pos != std::string::npos){

        if (ContentTypeValue.find(";") != std::string::npos){
            pos = ContentTypeValue.find("boundary=");

            if (pos != std::string::npos){

                if (pos + 9 < ContentTypeValue.size()){
                    MultipartBoundary = ContentTypeValue.substr(pos+9);

                    return SwitchState(READ_MULTIPART_BODY);
                }
            }
        }

        return setErrorNumber(400, "Bad Request – Invalid or missing boundary parameter in multipart 'Content-Type' header");
    }
}





void        ParseRequest::ParseMultiPartBufferBody(std::vector <char >& None){
    (void) None;

    std::string Delimiter = "--" + MultipartBoundary;

    std::string Part;

    size_t StartPart = 0;

    size_t NextPos = 0;
     
    ResetBuffPos();
    std::cout << "HERE HERE HERE HERE \n\n\n";
    while (true){
        pos = getBufferBody_string().find(Delimiter, pos);

        if (pos == std::string::npos)
            return ;

        StartPart = pos + Delimiter.size();
        if (StartPart + 2 <= getBufferBody_string().size() && !getBufferBody_string().compare(StartPart, 2, "--"))
            break ;

        if (StartPart + 2 <= getBufferBody_string().size() && !getBufferBody_string().compare(StartPart, 2, CLRF))
            StartPart += 2;
        else if (StartPart + 1 <= getBufferBody_string().size() && !getBufferBody_string().compare(StartPart, 1, "\n"))
            StartPart += 1;

        NextPos = getBufferBody_string().find(Delimiter, StartPart);

        if (NextPos == std::string::npos)
            break ;

        if (NextPos >= 2 && !getBufferBody_string().compare(NextPos - 2, 2, CLRF))
            NextPos -= 2;
        else if (NextPos >= 1 && !getBufferBody_string().compare(NextPos - 1, 1, "\n"))
            NextPos -= 1;

        Part = getBufferBody_string().substr(StartPart, NextPos - StartPart);

        MultipartBufferBody.push_back( std::vector<char >(Part.begin(), Part.end()));

        pos = NextPos;

        if (getParseState() == FINISH || getParseState() == ERROR)
            return ;
    }

    SwitchState(FINISH);
}

// parse cookies if there is
void    ParseRequest::parseCookies(std::string& None)
{
    (void) None;
    std::string cookie_header = getHeaderValue("cookie");

    if (cookie_header.empty())
        return;
    
    std::istringstream    iss(cookie_header);
    std::string            cookie;
    
    while (std::getline(iss, cookie, ';'))
    {
        size_t pos = cookie.find('=');
        if (pos != std::string::npos)
        {
            std::string name = cookie.substr(0, pos);
            std::string value = cookie.substr(pos + 1);
            
            trimBuff(name);

            while (name.length() > 0 && (name[name.length() - 1] == ' ' || name[name.length() - 1] == '\t'))
                name.erase(name.length() - 1, 1);
            
            while (value.length() > 0 && (value[0] == ' ' || value[0] == '\t'))
                value.erase(0, 1);

            while (value.length() > 0 && (value[value.length() - 1] == ' ' || value[value.length() - 1] == '\t'))
                value.erase(value.length() - 1, 1);
            
            cookies[name] = value;
        }
    }
}

// decompress the body only if the encoding type is gzip or deflate
void        ParseRequest::DecompressBody(){

    z_stream    Strm;

    std::memset(&Strm,0,sizeof(Strm));

    int            Bits = 15;

    if (ContentEncodingType == GZIP)
        Bits += 16;
    else if (ContentEncodingType == DEFLATE)
        Bits *= -1;

    Strm.zalloc = Z_NULL;
    Strm.zfree = Z_NULL;
    Strm.opaque = Z_NULL;
    Strm.next_in = (Bytef *)RequestBufferbody.data();
    Strm.avail_in = RequestBufferbody.size();
    if (inflateInit2(&Strm, Bits) != Z_OK)
        return (setErrorNumber(400, "Decompressing The Body Fails "));

    char outbuffer[32768];
    int ret = Z_OK;
    DecompressedBufferBody.clear();

    while (ret != Z_STREAM_END){
        Strm.next_out = (Bytef *) outbuffer;
        Strm.avail_out = sizeof(outbuffer);
        ret = inflate(&Strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END){
            inflateEnd(&Strm);
            return (setErrorNumber(400, "Decompressing the body Fails 2222"));
        }
        DecompressedBufferBody.insert(DecompressedBufferBody.end(), \
        outbuffer, outbuffer + (sizeof(outbuffer) - Strm.avail_out));
    }
    
    inflateEnd(&Strm);

    // SwitchState(FINISH);
}

// reset the variable to parse another request ;
void        ParseRequest::ResetParserf(){

    errorNumber = 200;

    ResetBuffPos();

    CurrntParsState = PARSER_NONE;

    Current_key.clear();

    Current_value.clear();

    Current_header_line.clear();

    Method.clear();

    Url.clear();

    HttpProtocolVersion.clear();

    chunkedEncoding = 0;

    contentLength = 0;

    Headers.clear();

    Port.clear();

    Host.clear();

    hasValidHost = 0;

    ChunkSize = 0;

    BufferBody.clear();

    QueryString.clear();
}




//_____________________________________________________________________________GETTER_FUNCTIONS_____________________________________________________________________________




std::string&                                         ParseRequest::getMethod()                        	{ return (Method); }

std::string&                                         ParseRequest::getVersion()                      	{ return (HttpProtocolVersion); }

std::string&                                         ParseRequest::getUri()                          	{ return (Url); }

int                                                 ParseRequest::getParseState()                   	{ return (CurrntParsState); }

int                                                 ParseRequest::getErrorNumber()                    	{ return (errorNumber);}


std::vector<std::pair<std::string, std::string> >&	ParseRequest::getHeaders()                        	{ return (Headers);}

std::string&                                        ParseRequest::getHost()                            	{ return (Host);}

std::string&                                        ParseRequest::getPort()                            	{ return (Port);}

int                                                 ParseRequest::getContentEncodingType(int Type)   	{ (void) Type; return (ContentEncodingType);}

std::string&                                        ParseRequest::getQueryString()                    	{ return (QueryString);}

std::vector <char >&                                ParseRequest::getBufferBody()                    	{ return (RequestBufferbody);}

std::string                                			ParseRequest::getBufferBody_string()                { 

	std::string ReqB(&RequestBufferbody[0], RequestBufferbody.size());	
	
	return (ReqB);

}

size_t                                              ParseRequest::getContentLength()                	{ return (contentLength);}

std::vector <char >&                                ParseRequest::getBufferDecompressedBody()        	{ return (DecompressedBufferBody);}

std::string	                                		ParseRequest::getBufferDecompressedBody_string()    { 

	std::string Decompresstring( &DecompressedBufferBody[0], DecompressedBufferBody.size());
	
	return (Decompresstring);

}

std::vector<std::vector <char > >&                    ParseRequest::getMultipartBuferBody()            	{ return (MultipartBufferBody);}

std::vector<std::string >                    ParseRequest::getMultipartBuferBody_string()		{

	std::vector<std::string > MultipartStr;
	
	for (std::vector<std::vector <char > >::iterator it = MultipartBufferBody.begin(); it != MultipartBufferBody.end();it++){
		MultipartStr.push_back(std::string(it->begin(), it->end()));
	}
	return (MultipartStr);

}

Server                                                ParseRequest::getBlockServer()                     { return *S; }

Location*                                            ParseRequest::getMatchLocation()                { return MatchLocation; }

std::string    ParseRequest::getCookie(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator it = cookies.find(name);

    return (it != cookies.end()) ? it->second : "";
}

const std::map<std::string, std::string>&    ParseRequest::getCookies() const                        { return cookies; }

// check if the url match a location block on the server to get its allowed methods
// if no location  matched i get the default allowed methods for the server
const std::vector<std::string>&     ParseRequest::getMatchedLocationAllowedMethods(){

    if (MatchLocation)
        return MatchLocation->getAllowedMethods();
    return S->getAllowedMethods();
}

// get the matched location body size limits
int     ParseRequest::getMatchedLocationBodySizeMax(){

    if (MatchLocation)
        return MatchLocation->getClientBodyLimit();
    return S->getClientBodyLimit();
}

// a header value fron the readeed request headers ;
std::string ParseRequest::getHeaderValue(std::string key){

    toLowerCase(key);

    std::vector<std::pair<std::string , std::string> >::iterator i;

    for (i = Headers.begin(); i != Headers.end();i++){
        if (i->first == key)
            return (i->second);
    }

    return ("");
}




//_____________________________________________________________________________SETTER_FUNCTIONS_____________________________________________________________________________




void ParseRequest::setMethod(std::string m)                 { Method = m; }

void ParseRequest::SwitchState(int Next_State)              { CurrntParsState = Next_State; }

void ParseRequest::setUri(std::string u)                    { Url = u; }

void ParseRequest::setVersion(std::string v)                { HttpProtocolVersion = v; }

void ParseRequest::Reset()                                  { SwitchState(PARSER_NONE); }

void ParseRequest::ResetBuffPos()                           { pos = 0; }

void ParseRequest::setContentEncodingType(int Type)            { ContentEncodingType = Type;}

void ParseRequest::setQueryString(std::string queryInUrl)   { QueryString = queryInUrl; }

void ParseRequest::setErrorNumber(int Number, std::string ErrorMsg){

    errorNumber = Number;
    SwitchState(ERROR);

    std::cerr << RED << "ERROR: " << errorNumber << "  " << ErrorMsg << RESET << std::endl;
}





//_____________________________________________________________________________PARSE_HELPER_FUNCTIONS_____________________________________________________________________________


void    ParseRequest::FindMatchLocation()
{
    const std::vector<Location*>&    locations = S->getLocations();
    std::string    urlpath = getUri();
    size_t    size(0);

    for (size_t i(0); i < locations.size(); i++)
    {
        const std::string& locPath = locations[i]->getPath();

        if (urlpath.compare(0, locPath.size(), locPath) == 0){
            if (locations[i]->getPath().size() > size){
                size = locations[i]->getPath().size();
                MatchLocation = locations[i];
            }
        }
    }
}

// check if the request parsing is finish or not yet ;
bool ParseRequest::isFinish(int RecvReturn) {

	(void ) RecvReturn;
    if (CurrntParsState == FINISH)
        std::cout << GRN << "OK " << YLW << "- HTTP request parsed and validated successfully" << RESET << std::endl;
    
    // else if (CurrntParsState != ERROR && CurrntParsState != FINISH){    
    //     if (!RecvReturn)
    //         SwitchState(CLOSE);
    //     else if (RecvReturn == -1)
    //         setErrorNumber(400, "Incomplete Request – Client closed connection before completing the request");
    // }
    
    return (CurrntParsState == FINISH || CurrntParsState == ERROR || CurrntParsState == CLOSE );
}

// checking the method if its a supporetd one by the server ;
bool ParseRequest::isSupportedMethod(std::string &RequestMethod)
{
    const std::vector<std::string> &sMethods = getMatchedLocationAllowedMethods();
    std::string m = RequestMethod;

    for (size_t i = 0; i < sMethods.size(); i++)
    {
        if (sMethods[i] == m)
            return (true);
    }

    return (false);
}

// check the request method if its known one or not ;
int ParseRequest::isKnownMethod()
{
    if (Method == "GET" || Method == "POST" || \
        Method == "DELETE" || Method == "HEAD" || \
        Method == "PUT" || Method == "OPTIONS" || \
        Method == "TRACE" || Method == "CONNECT"){
        return (405);
    }

    return (501);
}

// checking a character if its valid hexa decimal ;
bool ParseRequest::isHexa(char characetre)      { return (std::isxdigit(characetre)); }

// checking the unreserved charcter in the querie string in url ;
bool ParseRequest::Unresreved(char c){

    std::string s = "-._~";

    return (std::isalnum(c) || (s.find(c) != std::string::npos));
}

// checkin for  the reserved charcters in the querie string in the url ;
bool ParseRequest::Reserved(char c){

    std::string ResevedCharacters = ":/?#[]@!$&'()*+,;=";

    return (ResevedCharacters.find(c) != std::string::npos);
}

// checking if the percent incoding the quesries strings in url if its valif or not ==>
// cause every percent in it should be followed by a hexadecimal number ;
bool ParseRequest::PercentEncoded(size_t &i){

    if (i + 2 <= Url.size() && isHexa(Url[i + 1]) && isHexa(Url[i + 2]))
        return (i += 2, true);

    return (false);
}

// checking if the url is valid and define the error number if its not ;
bool ParseRequest::isValidUrl(){

    if (Url.size() >= 8192)
        return (setErrorNumber(414, "URI Too Long – Request-URI exceeds maximum length (8192 bytes)"), false);

    if (Url.empty() || Url[0] != '/')
        return (setErrorNumber(400, "Bad Request – URL is empty or does not start with '/'\n"), false);

    for (size_t i(0); i < Url.size(); i++)
    {
        if ((Url[i] == '%' && !PercentEncoded(i)) || (!Unresreved(Url[i]) && !Reserved(Url[i])))
        {
            return (setErrorNumber(400, "Bad Request – URL contains invalid or incorrectly encoded characters"), false);
        }
    }

    return (true);
}

// change a string to a lowecase for headers specialy ;
void ParseRequest::toLowerCase(std::string &key){

    for (size_t i(0); i < key.size(); i++)
    {
        key[i] = std::tolower((unsigned char)key[i]);
    }

}

// trimming a buffer if there is a spaces at the begining ;
void ParseRequest::trimBuff(std::string &str){

    pos = str.find_first_not_of(" ");

    if (pos != std::string::npos)
        str.erase(0, pos);

    ResetBuffPos();
}

// trimming a buffer if there is a spaces at the begining ;
void ParseRequest::trimBuffTail(std::string &str){

    pos = str.find_last_not_of(" ");

    if (pos != std::string::npos)
        str.erase(pos);

    ResetBuffPos();
}


// check request verison and define the error number if its not valid ;
bool ParseRequest::isValidVersion(){

    if (HttpProtocolVersion == "HTTP/1.1")
    {
        return (true);
    }

    if (HttpProtocolVersion == "HTTP/0.9" || HttpProtocolVersion == "HTTP/2" || HttpProtocolVersion == "HTTP/3"  || HttpProtocolVersion == "HTTP/1.0")
        return (setErrorNumber(505, "HTTP Version Not Supported – '" + HttpProtocolVersion + "' is not supported by the server"), false);

    return (setErrorNumber(400, "Bad Request – Malformed or invalid HTTP version: '" + HttpProtocolVersion + "'"), false);
}



// checking if a string is only spaces ;
bool ParseRequest::isAllSpaces(std::string &str){

    for (size_t i(0); i < str.size(); i++)
    {
        if (!std::isspace(str[i]))
            return (false);
    }

    return (true);
}

// checking for the header key if contain a non valid character ;
bool ParseRequest::validKey(std::string &key){

    std::string visbleChar = "!#$%&'*+-.^_|~`";

    for (size_t i(0); i < key.size(); i++)
    {
        if (!std::isalnum(key[i]) && visbleChar.find(key[i]) == std::string::npos)
            return false;
    }

    return true;
}

// checking for host (a mandatory header that should be in the request headers ) ;
bool ParseRequest::checkIsThereaHost(){

    std::string hoststring;

    for (size_t i(0); i < Headers.size(); i++)
    {
        if (Headers[i].first == "host" && !Headers[i].second.empty())
        {
            size_t p = Headers[i].second.find(':');

            if (p != std::string::npos)
            {
                Host = Headers[i].second.substr(0, p);
                Port = Headers[i].second.substr(p + 1);
            }
            else
                Host = Headers[i].second;

            hasValidHost = true;
            return (true);
        }
    }

    return (false);
}

// checking a string if its a number or not ;
bool ParseRequest::isNumber(std::string toCheck){

    for (size_t i(0); i < toCheck.size(); i++)
    {
        if (!isdigit(toCheck[i]))
            return (false);
    }

    return (true);
}

// checking if if the request has a body and get the body type ;
void ParseRequest::CheckingForBody(){

    bool TransferEncodingPresent = false;

    bool contentLengthPresent = false;

    if (Method == "GET" || Method == "DELETE")
        return (SwitchState(FINISH));

    std::vector<std::pair<std::string, std::string> >::iterator Headersit;

    for (Headersit = Headers.begin(); Headersit != Headers.end(); Headersit++)
    {
        if (Headersit->first == "transfer-encoding")
        {
            TransferEncodingPresent = true;

            std::string tmp = Headersit->second;

            toLowerCase(tmp);

            if (tmp == "chunked")
                chunkedEncoding = true;
            else
                return (setErrorNumber(501, "Not Implemented – Unsupported transfer-encoding (only 'chunked' is supported)"));
        }

        if (Headersit->first == "content-length")
        {
            contentLengthPresent = true;

            if (!isNumber(Headersit->second))
                return (setErrorNumber(400, "Bad Request – Invalid 'Content-Length' header (must be a non-negative integer)"));

            contentLength = std::atoi(Headersit->second.c_str());

            if (contentLength < 0)
                return (setErrorNumber(400, "Bad Request – Invalid 'Content-Length' header (cannot be negative)"));

            if (contentLength > getMatchedLocationBodySizeMax())
                return setErrorNumber(413, "Payload Too Large – Request body exceeds maximum allowed size");
        }
    }

    if (TransferEncodingPresent && contentLengthPresent)
        return (setErrorNumber(400, "Bad Request – Cannot have both 'Transfer-Encoding' and 'Content-Length' headers"));

    if (!chunkedEncoding && !contentLengthPresent)
        return (setErrorNumber(411, "Length Required – Request must include 'Content-Length' or 'Transfer-Encoding: chunked'"));

    if (chunkedEncoding)
        return SwitchState(READCHUNKSIZE);

    SwitchState(CONTENTLENGTHBODY);

    CheckContentEncoding();
}

// convert from hexadecimal to decimal ;
int ParseRequest::HexaStringToDecimalNum(std::string s){

    std::istringstream stremstr(s);
    int ret = 0;
    stremstr >> std::hex >> ret;

    return ret;
}

// check if the body coded with valid coding type ;
void         ParseRequest::CheckContentEncoding(){
    
    std::string Value =  getHeaderValue("content-encoding");
    
    if (Value.empty())
        return ;
    
    trimBuff(Value);

    if ((pos = Value.find_first_of(" \t")) != std::string::npos)
        Value.erase(pos);

    ResetBuffPos();

    std::string &valueRefrence = Value;

    toLowerCase(valueRefrence);

    if (Value == "identity")
        return ;
    else if (Value == "deflate")
        return (setContentEncodingType((DEFLATE)));
    else if (Value == "gzip")
        return (setContentEncodingType((GZIP)));
    else
        return (setErrorNumber(415, "Unsupported Media Type – Unsupported 'Content-Encoding' type (only 'identity', 'gzip', and 'deflate' are supported)"));

}

Server*    ParseRequest::findBlockServer(const Config& config, std::string buff, Server* server)
{
    size_t    pos_start = buff.find("Host: ");
    
    (void) server;
    if (pos_start == std::string::npos)
        return S;
    
    pos_start += 6;

    size_t    pos_end = buff.find_first_of("\r\n", pos_start);

    if (pos_end == std::string::npos)
        return S;

    std::string host_port = buff.substr(pos_start, pos_end - pos_start);
    size_t pos = host_port.find(":");

    if (pos == std::string::npos)
        return S;

    std::string host = host_port.substr(0, pos);
    std::string port = host_port.substr(pos + 1, host_port.size());
    Server* match_block = const_cast<Server*> (config.getServerName(host, port));

    if (match_block != NULL)
        return (match_block);

    match_block = const_cast<Server*> (config.getServer(host, port));

    if (match_block != NULL)
        return (match_block);

    return S;
}



//_____________________________________________________________________________START_READING_AND_PARSE_____________________________________________________________________________

void    ParseRequest::ReadAndParseRequestBody(std::string&    buff, int fd){

    RequestBufferbody.assign( buff.begin(), buff.end());
    std::cout << YLW << buff << RESET;
    size_t  ByetsReadsFromBody = buff.size();
    buff.clear();
    ssize_t ReadedBytes = 0;
    while (true){

        std::memset(ReadingBuffer, 0, READING_BUFFER_SIZE);
        ReadedBytes = recv(fd, ReadingBuffer, READING_BUFFER_SIZE, 0);
        std::cout <<RED<<"\n\n\n\n\n\n\n\n\n\n"<< RequestBufferbody.size() << "\n\n\n\n\n\n\n\n" << RESET ;
        if (ReadedBytes > 0){
            ByetsReadsFromBody += ReadedBytes;
            RequestBufferbody.insert(RequestBufferbody.end(), &ReadingBuffer[0], ReadingBuffer + ReadedBytes);

        }
        std::cout << YLW << ByetsReadsFromBody << RESET << "         \n\n\n" ;
        (this->*ParseRequest::SecondParseTable[CurrntParsState])(RequestBufferbody);
	
        if (ReadedBytes <= 0){
            if (isFinish(ReadedBytes))
                return ;
        }
    }
}

void    ParseRequest::ReadAndParseIntilHeadersFinish(std::string& buff, int fd, const Config& config, Server* server){

    int FindBlockServerCheck = 0;
    while (true){

        std::memset(ReadingBuffer, 0, READING_BUFFER_SIZE);
        ssize_t ReadedBytes = recv(fd, ReadingBuffer, READING_BUFFER_SIZE, 0);
        
        if (ReadedBytes > 0){
            ReadingBuffer[ReadedBytes] = 0;
            buff.append(ReadingBuffer, ReadedBytes);
        }
        
        if (buff.find(HEADERS_ENDING) != std::string::npos){
            if (!FindBlockServerCheck){
                S = findBlockServer(config, buff, server);
                FindBlockServerCheck++;
            }
            while (!buff.empty() && (pos = buff.find(CLRF)) != std::string::npos){
                if (Current_PrasingLine.empty()){
                    Current_PrasingLine = buff.substr(0, pos + 2);
					std::cout << RED << "current Prase Line   : " << Current_PrasingLine << RESET;
                    buff.erase(0, pos + 2);
                    if (Current_PrasingLine.find(CLRF) == 0){
                        // buff.erase(0, 2);
						if (!checkIsThereaHost())
            				return (setErrorNumber(400, "Bad Request – Missing or empty 'Host' header (required in HTTP/1.1)"));
        				CheckingForBody();
						return ;
					}
                }
                (this->*ParseRequest::FirstParseTable[CurrntParsState])(Current_PrasingLine);
            }
        }
        if (ReadedBytes <= 0){
            if (isFinish(ReadedBytes) && buff.empty())
                return ;
        }

    }
}

void        ParseRequest::startParse(int fd, const Config& config, Server* server){
        std::string buff;

        if (PARSER_NONE == CurrntParsState)
            S = server;
        
        ReadAndParseIntilHeadersFinish(buff, fd, config, server);
        parseCookies(buff);
        if (Method == "POST" && CurrntParsState > ADD_HEADER && CurrntParsState < PARSEARRAYSIZE){
            std::cout << "hhhhhhhhhhh\n\n\n\n";
            ReadAndParseRequestBody(buff, fd);
        }
        return ;
}




// void ParseRequest::startParse(int fd, const Config& config, Server*server){
//         std::string buff;

//         if (PARSER_NONE == CurrntParsState)
//                 S = server;

//         while(true)
//         {
//                 char        str[6];

//                 while ((buff.find("\r\n\r\n") == std::string::npos) ||
//                                 CurrntParsState > ADD_HEADER ){
//                         std::memset(str, 0, sizeof(str));
//                         ssize_t bytes = recv(fd, str, 5, 0);
//                         if (bytes > 0)
//                         {
//                                 str[bytes] = 0;
//                                 buff.append(str, bytes);
//                                 if ((buff.find("\r\n\r\n") != std::string::npos))
//                                     break;
//                         }
//                         if (bytes == 0){
//                                 if (CurrntParsState == ERROR || CurrntParsState == FINISH || CurrntParsState == PARSER_NONE){
//                                         if (CurrntParsState == FINISH){
//                                                 std::cout << GRN << "OK " << YLW << "- HTTP request parsed and validated successfully" << RESET << std::endl;
//                                                 return ;
//                                         }
//                                 }
//                                 setErrorNumber(400, "Bad Request – Client closed connection before completing the request");
//                                 return;
//                         }
//                         if (bytes < 0 &&
//                                         !(CurrntParsState == READ_BOUNDARY || CurrntParsState == READ_MULTIPART_BODY)){
//                                         break;
//                         }

//                         if (CurrntParsState == PARSER_NONE)
//                                 S = findBlockServer(config, buff, server);
//                 }
//                 std::cout << buff << "\n";
//                 std::cout << CurrntParsState << "      \n";
//                 switch(CurrntParsState){
//                         case FINISH:
//                         case ERROR:
//                                 return;
//                         default :
//                                 if (CurrntParsState < PARSEARRAYSIZE){
//                                         (this->*ParseRequest::ParseTable[CurrntParsState])(buff);
//                                         break;
//                                 }
//                 }
//                 if (CurrntParsState == FINISH || CurrntParsState == ERROR)
//                         break ;
//         }
// }

