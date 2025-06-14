#include "../inc/ParseRequest.hpp"
#include "../inc/Server.hpp"


const ParseRequest::ParseFuncPtr ParseRequest::ParseTable[] = {
	&ParseRequest::StartNewRequest,
	&ParseRequest::parseMethod,         
    &ParseRequest::parseUrl,               
    &ParseRequest::parseHttpVersion, 
	&ParseRequest::parseHeaders,
	&ParseRequest::parseHeaders,
	&ParseRequest::parseHeaders,
	&ParseRequest::parseContentlengthBody,
	&ParseRequest::parseChunkedBody,
	&ParseRequest::parseChunkedBody,
};



// construct
ParseRequest::ParseRequest() : errorNumber(0), pos(0), 
    CurrntParsState(NONE), Method(""), Url(""),
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
	NonRepeatablesHeaders["user-agent	"] = 0;
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
ParseRequest::ParseRequest(Server *server) : errorNumber(0), pos(0), 
    CurrntParsState(NONE), Method(""), Url(""),
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

// get the method ;
std::string ParseRequest::getMethod()           { return (Method); }
// get the http version ;
std::string ParseRequest::getVersion()          { return (HttpProtocolVersion); }
// get the url ;
std::string ParseRequest::getUri()              { return (Url); }
// getting the parse stat ;
int ParseRequest::getParseState()               { return (CurrntParsState); }

// setters


// set the method ;
void ParseRequest::setMethod(std::string m)     { Method = m; }
// switch parse stat ;
void ParseRequest::SwitchState(int Next_State)  { CurrntParsState = Next_State; }
// set the url ; 
void ParseRequest::setUri(std::string u)        { Url = u; }
// set the httpverstion ;
void ParseRequest::setVersion(std::string v)    { HttpProtocolVersion = v; }
// reset parsstat ;
void ParseRequest::Reset()                      { SwitchState(NONE); }
// setting the error number ;
void ParseRequest::setErrorNumber(int Number)   {errorNumber = Number;  SwitchState(ERROR);}
// reset the pos of parsing ;
void ParseRequest::ResetBuffPos()               { pos = 0; }

// checkers

// check if the request parsing is finish or not yet ; 
bool ParseRequest::isFinish()                   { return (getParseState() == FINISH); }

// checking the method if its a supporetd one by the server ;
bool ParseRequest::isSupportedMethod(std::string &RequestMethod)
{
	const std::vector<std::string> &sMethods = S->getAllowedMethods();
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
bool ParseRequest::isHexa(char characetre)      { return (isxdigit(characetre)); }

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
		return (SwitchState(ERROR), setErrorNumber(414), false);
	if (Url.empty() || Url[0] != '/')
		return (SwitchState(ERROR), setErrorNumber(400), false);
	for (size_t i(0); i < Url.size(); i++)
	{
		if ((Url[i] == '%' && !PercentEncoded(i)) || (!Unresreved(Url[i]) && !Reserved(Url[i])))
			return (SwitchState(ERROR), setErrorNumber(400), false);
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
	if (isspace(str[0]))
		return (SwitchState(ERROR), setErrorNumber(400));
	pos = str.find(SPACE);
	if (pos == std::string::npos)
	{
		if ((str.find('\r') != std::string::npos) || str.find('\n') != std::string::npos)
			return (SwitchState(ERROR), setErrorNumber(400));
		Method += str;
		str.clear();
		return (ResetBuffPos());
	}
	Method += str.substr(0, pos);
	str.erase(0, pos + 1);
	ResetBuffPos();
	// if (!isSupportedMethod(Method))
	//     return (setErrorNumber(isKnownMethod()));
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
			return (SwitchState(ERROR), setErrorNumber(400));
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
	SwitchState(HTTPVERSION);
}

// check request verison and define the error number if its not valid ;
bool ParseRequest::isValidVersion(){
	if (HttpProtocolVersion == "HTTP/1.1")
		return (true);
	if (HttpProtocolVersion == "HTTP/1.0" || HttpProtocolVersion == "HTTP/0.9" ||
			HttpProtocolVersion == "HTTP/2" || HttpProtocolVersion == "HTTP/3")
		return (SwitchState(ERROR), setErrorNumber(505), false);
	return (SwitchState(ERROR), setErrorNumber(400), false);
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
	if (isspace(str[pos - 1]))
		return (SwitchState(ERROR), setErrorNumber(400));
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
		if (!isspace(str[i]))
			return (false);
	}
	return (true);
}

// checking for the header key if contain a non valid character ; 
bool ParseRequest::validKey(std::string &key){
	std::string visbleChar = "!#$%&'*+-.^_|~`";
	for (size_t i(0); i < key.size(); i++)
	{
		if (!isalnum(key[i]) && visbleChar.find(key[i]) == std::string::npos)
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
	return (SwitchState(ERROR), setErrorNumber(400), false);
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
			return (SwitchState(ERROR), setErrorNumber(431));
		pos = Current_header_line.find(':');
		if (pos == std::string::npos || isspace(Current_header_line[pos - 1]))
			return (SwitchState(ERROR), setErrorNumber(400));    
		Current_key = Current_header_line.substr(0, pos);
		toLowerCase(Current_key);
		SwitchState(HEADER_VALUE);
	}
	if (CurrntParsState ==  HEADER_VALUE){
		Current_value += Current_header_line.substr(pos + 1);
		trimBuff(Current_value);
		if (!validKey(Current_key) || Current_key.empty() || \
            isAllSpaces(Current_key) || Current_value.empty()){
			return (SwitchState(ERROR), setErrorNumber(400));
        }
        SwitchState(ADD_HEADER);
	}
	if (CurrntParsState == ADD_HEADER){
		std::map<std::string, int>::iterator it = NonRepeatablesHeaders.find(Current_key);
		if (it != NonRepeatablesHeaders.end())
		{
			if (it->second)
				return (SwitchState(ERROR), setErrorNumber(400));
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
			return (SwitchState(ERROR), setErrorNumber(400));
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
	if (Method == "GET" || Method == "DELETE"){

		return (SwitchState(FINISH));
	}
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
				return (SwitchState(ERROR), setErrorNumber(501));
		}
		if (Headersit->first == "content-length")
		{
			contentLengthPresent = true;
			if (!isNumber(Headersit->second))
				return (SwitchState(ERROR), setErrorNumber(400));
			contentLength = std::atoi(Headersit->second.c_str());
			// if (contentLength < 0)
			//     return (SwitchState(ERROR), setErrorNumber(400));
			//
		}
	}
	if (TransferEncodingPresent && contentLengthPresent)
		return (SwitchState(ERROR), setErrorNumber(400));
	if (!chunkedEncoding && !contentLengthPresent)
		return (SwitchState(FINISH));
	if (chunkedEncoding)
		return SwitchState(READCHUNKSIZE);
	SwitchState(CONTENTLENGTHBODY);
}

// parse the content length body type ;
void ParseRequest::parseContentlengthBody(std::string &str){
	if (contentLength == 0)
		return (SwitchState(FINISH));
	pos = str.find(CLRF);
	if (pos != std::string::npos){
		BufferBody =  str.substr(0,pos);
		str.clear();
		std::cout << "something\n";
		if (contentLength != BufferBody.size())
			return (SwitchState(ERROR), setErrorNumber(400));
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
					return (SwitchState(ERROR), setErrorNumber(400));
			}
			ChunkSize = HexaStringToDecimalNum(StringChunkSize);
			if (ChunkSize > S->getClientBodyLimit())
				return (SwitchState(ERROR), setErrorNumber(413));
			str.erase(0, pos + 2);
			ResetBuffPos();
			if (ChunkSize == 0)
				return (SwitchState(FINISH));
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
	std::vector<std::pair<std::string , std::string> >::iterator i;
	for (i = Headers.begin(); i != Headers.end();i++){
		if (i->first == key)
			return (i->second);
	}
	return ("");
}

// reset the variable to parse another request ;
void        ParseRequest::ResetParserf(){
	errorNumber = 0;
	ResetBuffPos();
	CurrntParsState = NONE;
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

// start reading and parsing ;
void ParseRequest::startParse(int fd){
	std::string buff;

	while (true)
	{
		char        str[1000];

		memset(str, 0, sizeof(str));
		ssize_t bytes = recv(fd, str, sizeof(str) - 1, 0);
		if (bytes == 0)
			return (SwitchState(CLOSE));
		else if (bytes < 0)
			return (SwitchState(NONE));
		buff.append(str);
		switch(CurrntParsState){
			case FINISH:
				return ;
			case ERROR:
				return ;
			default :
			if (CurrntParsState < PARSEARRAYSIZE)
				(this->*ParseRequest::ParseTable[CurrntParsState])(buff);
			break;
		}
	}
}
