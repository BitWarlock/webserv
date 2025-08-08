#include "../inc/ParseRequest.hpp"
#include "../inc/Server.hpp"

// tbale that hold apointers to parse functions;
const ParseRequest::ParseFuncPtr ParseRequest::ParseTable[] = {
	&ParseRequest::StartNewRequest,  			//	NONE_ (so the parse will start)
	&ParseRequest::parseMethod,         		//	METHOD_ 
    &ParseRequest::parseUrl,            		//	URL_
    &ParseRequest::parseHttpVersion, 			//	HTTPVERSION_
	&ParseRequest::parseHeaders,				//	HEADERS_KEY_
	&ParseRequest::parseHeaders,				//	HEDERS_VALUE_
	&ParseRequest::parseHeaders,				//	ADDING THE HEADER_
	&ParseRequest::parseContentlengthBody,		//	CONTENT_LENGHT_BODY_
	&ParseRequest::parseChunkedBody,			//	CHUNED_BODY_SIZE_
	&ParseRequest::parseChunkedBody,			//	READ_THE_CHUNK_
	&ParseRequest::ParseMultipartBodyBoundary,	//	READ_BOUNDARY_
	&ParseRequest::ParseMultiPartBufferBody,	//	READ_MULTYPART_BUFFER_BODY_
};



// construct
ParseRequest::ParseRequest() : errorNumber(200), pos(0), 
    CurrntParsState(PARSER_NONE), Method(""), Url(""),
	HttpProtocolVersion(""),
    chunkedEncoding(false), contentLength(0){
	BufferBody.clear();
	ChunkSize = 0;
	NonRepeatablesHeaders["host"] = 0;
	NonRepeatablesHeaders["content-length"] = 0;
	NonRepeatablesHeaders["content-type"] = 0;
	NonRepeatablesHeaders["content-encoding"] = 0;
	NonRepeatablesHeaders["transfer-encoding"] = 0;
	NonRepeatablesHeaders["authorization"] = 0;
	NonRepeatablesHeaders["user-agent"] = 0;
	NonRepeatablesHeaders["connection"] = 0;
	NonRepeatablesHeaders["date"] = 0;
	NonRepeatablesHeaders["upgrade"] = 0;
	NonRepeatablesHeaders["expect"] = 0;
	NonRepeatablesHeaders["range"] = 0;
}
// ParseRequest::ParseRequest(ParseRequest& other){

// }
// ParseRequest& ParseRequest::operator=(const ParseRequest& other){
	
// }
// construct with params
ParseRequest::ParseRequest(Server *server) : errorNumber(200), pos(0), 
    CurrntParsState(PARSER_NONE), Method(""), Url(""),
	HttpProtocolVersion(""),
    S(server), chunkedEncoding(false), contentLength(0){
	BufferBody.clear();
	ChunkSize = 0;
	NonRepeatablesHeaders["host"] = 0;
	NonRepeatablesHeaders["content-length"] = 0;
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
// distructor
ParseRequest::~ParseRequest() {}

// getters

std::string& 										ParseRequest::getMethod()						{ return (Method); }
std::string& 										ParseRequest::getVersion()      				{ return (HttpProtocolVersion); }
std::string& 										ParseRequest::getUri()          				{ return (Url); }
int 												ParseRequest::getParseState()   				{ return (CurrntParsState); }
int         										ParseRequest::getErrorNumber()					{ return (errorNumber);}

std::vector<std::pair<std::string, std::string> >&	ParseRequest::getHeaders()						{ return (Headers);}
std::string&										ParseRequest::getHost()							{ return (Host);}
std::string&										ParseRequest::getPort()							{ return (Port);}
int         										ParseRequest::getContentEncodingType(int Type)	{ (void) Type; return (ContentEncodingType);}
std::string&										ParseRequest::getQueryString()					{ return (QueryString);}
std::string&										ParseRequest::getBufferBody()					{ return (BufferBody);}
size_t												ParseRequest::getContentLength()				{ return (contentLength);}
std::string&                                        ParseRequest::getBufferDecompressedBody()		{ return (DecompressedBufferBody);}
std::vector<std::string >&							ParseRequest::getMultipartBuferBody()			{ return (MultipartBufferBody);}
Server												ParseRequest::getBlockServer() { return *S; }

// setters


void ParseRequest::setMethod(std::string m)     	{ Method = m; }
void ParseRequest::SwitchState(int Next_State)  	{ CurrntParsState = Next_State; }
void ParseRequest::setUri(std::string u)        	{ Url = u; }
void ParseRequest::setVersion(std::string v)    	{ HttpProtocolVersion = v; }
void ParseRequest::Reset()                      	{ SwitchState(PARSER_NONE); }
void ParseRequest::setErrorNumber(int Number, std::string ErrorMsg)   	{ 
	errorNumber = Number;
	SwitchState(ERROR);
	std::cerr << RED << "ERROR: " << errorNumber << "  " << ErrorMsg << RESET << std::endl;
}
void ParseRequest::ResetBuffPos()               	{ pos = 0; }
void ParseRequest::setContentEncodingType(int Type)	{ 
	ContentEncodingType = Type;

}

// checkers

// check if the request parsing is finish or not yet ; 
bool ParseRequest::isFinish()                   { return (getParseState() == FINISH); }

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
			return (setErrorNumber(400, "Bad Request – URL contains invalid or incorrectly encoded characters"), false);
	}
	return (true);
}

// parsers

// change a string to a lowecase for headers specialy ;
void ParseRequest::toLowerCase(std::string &key){
	for (size_t i(0); i < key.size(); i++)
	{
		key[i] = std::tolower((unsigned char)key[i]);
	}
}

// trimming a buffer if there is a spaces in the begining ;
void ParseRequest::trimBuff(std::string &str){
	pos = str.find_first_not_of(" ");
	if (pos != std::string::npos)
		str.erase(0, pos);
	ResetBuffPos();
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

// set the querie string from the url if there is ;
void ParseRequest::setQueryString(std::string queryInUrl)   { QueryString = queryInUrl; }

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
	if (!isValidUrl())
		return;
	if ((pos = Url.find('?')) != std::string::npos)
	{
		setQueryString(Url.substr(pos + 1, Url.size()));
		Url.erase(pos);
	}
	ResetBuffPos();
	if (!isSupportedMethod(Method)){
		int numberErr = isKnownMethod();
		if (numberErr == 501)
			return (setErrorNumber(numberErr, "Method Not Implemented – Method '" + Method + "' is not supported"));
		else if (numberErr == 405)
	    	return (setErrorNumber(numberErr, "Method Not Allowed – Method '" + Method + "' is not allowed for the requested URL"));
	}
	SwitchState(HTTPVERSION);
}

// check request verison and define the error number if its not valid ;
bool ParseRequest::isValidVersion(){
	if (HttpProtocolVersion == "HTTP/1.1" || HttpProtocolVersion == "HTTP/1.0")
	{
		return (true);
	}
	if (HttpProtocolVersion == "HTTP/0.9" || HttpProtocolVersion == "HTTP/2" \
		|| HttpProtocolVersion == "HTTP/3")
		return (setErrorNumber(505, "HTTP Version Not Supported – '" + HttpProtocolVersion + "' is not supported by the server"), false);
	return (setErrorNumber(400, "Bad Request – Malformed or invalid HTTP version: '" + HttpProtocolVersion + "'"), false);
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
	if (HttpProtocolVersion == "HTTP/1.0")
		return true;
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
	if ((pos = str.find(CLRF)) == 0)
	{
		str.erase(0, 2);
		if (!checkIsThereaHost())
			return (setErrorNumber(400, "Bad Request – Missing or empty 'Host' header (required in HTTP/1.1)"));
		CheckingForBody();
		return;
	}
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

// parse the content length body type ;
void ParseRequest::parseContentlengthBody(std::string &str){
	if (contentLength == 0)
		return (SwitchState(FINISH));
	BufferBody +=  str.substr(0);
	str.erase(0, str.size());
	if (static_cast<size_t> (contentLength) > BufferBody.size())
		return ;
	if (static_cast<size_t> (contentLength) < BufferBody.size())
		return (setErrorNumber(400, "Bad Request – Request body exceeds declared 'Content-Length' value"));
	if (static_cast<size_t> (contentLength) == BufferBody.size()){
        if (ContentEncodingType == GZIP || ContentEncodingType == DEFLATE)
			DecompressBody();
		}

        std::string contentType = getHeaderValue("Content-Type");
        if (getMethod() == "POST" && contentType.find("multipart/form-data") != std::string::npos) {
            return SwitchState(READ_BOUNDARY);
        }
        
        return (SwitchState(FINISH));
    }
}

// convert from hexadecimal to decimal ;
int ParseRequest::HexaStringToDecimalNum(std::string s){
	std::istringstream stremstr(s);
	int ret = 0;
	stremstr >> std::hex >> ret;
	return ret;
}

// parsing the chuncked body type ;
void ParseRequest::parseChunkedBody(std::string &str){
	if (CurrntParsState == READCHUNKSIZE)
	{
		pos = str.find(CLRF);
		if (pos != std::string::npos)
		{
			std::string StringChunkSize = str.substr(0, pos);
			for (size_t i(0); i < StringChunkSize.size(); i++)
			{
				if (!isHexa(StringChunkSize[i]))
					return (setErrorNumber(400, "Bad Request – Invalid chunk size format (must be hexadecimal)"));
			}
			ChunkSize = HexaStringToDecimalNum(StringChunkSize);
			if (ChunkSize > S->getClientBodyLimit())
				return (setErrorNumber(413, "Payload Too Large – Chunk size exceeds maximum allowed limit"));
			str.erase(0, pos + 2);
			ResetBuffPos();
			if (ChunkSize == 0){
				if (ContentEncodingType == GZIP || ContentEncodingType == DEFLATE)
					DecompressBody();
				if (getMethod() == "POST")
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
			BufferBody.append(str.substr(0, ChunkSize));
			str.erase(0, ChunkSize + 2);
			ChunkSize = 0;
			ResetBuffPos();
			SwitchState(READCHUNKSIZE);
		}
	}
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

// start newrequest;
void ParseRequest::StartNewRequest(std::string& buff){
	(void)buff;
	SwitchState(METHOD);
}

void        ParseRequest::DecompressBody(){
	z_stream	Strm;
	std::memset(&Strm,0,sizeof(Strm));
	int			Bits = 15;
	if (ContentEncodingType == GZIP)
		Bits += 16;
	else if (ContentEncodingType == DEFLATE)
		Bits *= -1;
	Strm.zalloc = Z_NULL;
	Strm.zfree = Z_NULL;
	Strm.opaque = Z_NULL;
	Strm.next_in = (Bytef *)BufferBody.data();
	Strm.avail_in = BufferBody.size();

	// if (inflateInit2(&Strm, Bits) != Z_OK)
	// 	return (setErrorNumber(400));
	char outbuffer[32768];
	int ret = Z_OK;
	DecompressedBufferBody.clear();
	while (ret != Z_STREAM_END){
		Strm.next_out = (Bytef *) outbuffer;
		Strm.avail_out = sizeof(outbuffer);
		// ret = inflate(&Strm, Z_NO_FLUSH);
		// if (ret != Z_OK && ret != Z_STREAM_END){
		// 	inflateEnd(&Strm);
		// 	return (setErrorNumber(400));
		// }
		DecompressedBufferBody.append(outbuffer, sizeof(outbuffer) - Strm.avail_out);
	}
	// inflateEnd(&Strm);
	SwitchState(FINISH);
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

// check if the url match a location block on the server to get its allowed methods
// if no location  matched i get the default allowed methods for the server  
const std::vector<std::string>&     ParseRequest::getMatchedLocationAllowedMethods(){
	std::string urlpath = Url;
	const std::vector<Location*>& locations = S->getLocations();

	while (true){
		size_t i = 0;
		while (i < locations.size()){
			if (locations[i]->getPath() == urlpath)
				return locations[i]->getAllowedMethods();
			i++;
		}
		if (urlpath != "/" && urlpath.size() > 1){
			if (urlpath[urlpath.size() - 1] == '/')
				urlpath.erase(urlpath.find_last_of('/'));
			urlpath = urlpath.erase(urlpath.find_last_of('/')+1);
		}
		else if (urlpath == "/") {
			while (i < locations.size()){
				if (locations[i]->getPath() == "/")
					return locations[i]->getAllowedMethods();
			}
			break;
		}
	}
	return S->getAllowedMethods();
}

int     ParseRequest::getMatchedLocationBodySizeMax(){
	std::string urlpath = Url;
	const std::vector<Location*>& locations = S->getLocations();

	while (true){
		size_t i = 0;
		while (i < locations.size()){
			if (locations[i]->getPath() == urlpath)
				return locations[i]->getClientBodyLimit();
			i++;
		}
		if (urlpath != "/" && urlpath.size() > 1){
			if (urlpath[urlpath.size() - 1] == '/')
				urlpath.erase(urlpath.find_last_of('/'));
			urlpath = urlpath.erase(urlpath.find_last_of('/')+1);
		}
        else if (urlpath == "/") {
			while (i < locations.size()){
			if (locations[i]->getPath() == "/")
				return locations[i]->getClientBodyLimit();
			}
			break;
		}
	}
	return S->getClientBodyLimit();
}

void        ParseRequest::ParseMultiPartBufferBody(std::string& None){
	(void) None;
	std::string Delimiter = "--" + MultipartBoundary;
	std::string Part;
	size_t StartPart = 0;
	size_t NextPos = 0;
	ResetBuffPos();
	while (true){
		pos = BufferBody.find(Delimiter, pos);
		if (pos == std::string::npos)
			return ;
		StartPart = pos + Delimiter.size();
		if (StartPart + 2 <= BufferBody.size() && !BufferBody.compare(StartPart, 2, "--"))
			break ;
		if (StartPart + 2 <= BufferBody.size() && !BufferBody.compare(StartPart, 2, CLRF))
			StartPart += 2;
		else if (StartPart + 1 <= BufferBody.size() && !BufferBody.compare(StartPart, 1, "\n"))
			StartPart += 1;
		NextPos = BufferBody.find(Delimiter, StartPart);
		if (NextPos == std::string::npos)
			break ;
		if (NextPos >= 2 && !BufferBody.compare(NextPos - 2, 2, CLRF))
			NextPos -= 2;
		else if (NextPos >= 1 && !BufferBody.compare(NextPos - 1, 1, "\n"))
			NextPos -= 1;
		Part = BufferBody.substr(StartPart, NextPos - StartPart);
		MultipartBufferBody.push_back(Part);
		pos = NextPos;
		if (getParseState() == FINISH || getParseState() == ERROR)
			return ;
	}
	SwitchState(FINISH);
}

void	ParseRequest::ParseMultipartBodyBoundary(std::string& None){
	(void ) None;
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

Server*	ParseRequest::findBlockServer(const Config& config, std::string buff)
{
	size_t	pos_start = buff.find("Host: ");
	
	if (pos_start == std::string::npos)
		return NULL;
	
	pos_start += 6;

	size_t	pos_end = buff.find_first_of("\r\n", pos_start);
	if (pos_end == std::string::npos)
		return NULL;

	std::string host_port = buff.substr(pos_start, pos_end - pos_start);
	size_t pos = host_port.find(":");

	if (pos == std::string::npos)
		return NULL;

	std::string host = host_port.substr(0, pos);
	std::string port = host_port.substr(pos + 1, host_port.size());

	Server* match_block = const_cast<Server*> (config.getServer(host, port));

	if (match_block != NULL)
		return (match_block);

	return (const_cast<Server*> (config.getServer(host, port)));
}

// start reading and parsing ;
void ParseRequest::startParse(int fd, const Config& config){
	std::string buff;
	while(true)
	{
		char        str[1000];
		if (buff.empty()){
			std::memset(str, 0, sizeof(str));
			ssize_t bytes = recv(fd, str, 999, 0);
			if (bytes <= 0 && (CurrntParsState == ERROR || CurrntParsState == FINISH || CurrntParsState == PARSER_NONE)){
				if (CurrntParsState == FINISH)
					std::cout << GRN << "OK " << YLW << "- HTTP request parsed and validated successfully" << RESET << std::endl;
				return ;
			}
			if (bytes > 0)
			{
				str[bytes] = 0;
				buff.append(str, bytes);
			}
			if (CurrntParsState == PARSER_NONE)
				S = findBlockServer(config, buff);
			std::cout << buff;
		}
		switch(CurrntParsState){
			case FINISH:
			case ERROR:
				return;
			default :
				if (CurrntParsState < PARSEARRAYSIZE){
					(this->*ParseRequest::ParseTable[CurrntParsState])(buff);
						break;
			}
		}
		if (CurrntParsState == FINISH || CurrntParsState == ERROR)
			break ;
	}
	parseCookies();
}

void	ParseRequest::parseCookies()
{
	std::string cookie_header = getHeaderValue("Cookie");
	if (cookie_header.empty())
		return;
	
	std::istringstream	iss(cookie_header);
	std::string			cookie;
	
	while (std::getline(iss, cookie, ';'))
	{
		size_t pos = cookie.find('=');
		if (pos != std::string::npos)
		{
			std::string name = cookie.substr(0, pos);
			std::string value = cookie.substr(pos + 1);
			
			while (name.length() > 0 && (name[0] == ' ' || name[0] == '\t'))
				name.erase(0, 1);
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

std::string	ParseRequest::getCookie(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = cookies.find(name);
	return (it != cookies.end()) ? it->second : "";
}

const std::map<std::string, std::string>&	ParseRequest::getCookies() const
{
	return cookies;
}
