#include "../inc/ParseDirective.hpp"

DirectiveParser::DirectiveParser(Lexer& lexer) : lexer(lexer) {}

DirectiveParser::~DirectiveParser()
{
	for (size_t i = 0; i < serversInProgress.size(); ++i)
		delete serversInProgress[i];
}

Config* DirectiveParser::parseConfig()
{
	Config* config = new Config();
	currentToken = lexer.getNextToken();

	while (currentToken.type != TOKEN_EOF)
	{
		if (currentToken.type == TOKEN_SERVER)
		{
			Server* server = parseServerBlock();
			config->addServer(server);
			serversInProgress.push_back(server);
		}
		else
			throw ParseException("Unexpected token outside server block", currentToken.line);
	}

	serversInProgress.clear();
	return (config);
}

Server* DirectiveParser::parseServerBlock()
{
	expect(TOKEN_SERVER);
	expect(TOKEN_OPEN_BRACE);

	Server* server = new Server();

	while (currentToken.type != TOKEN_CLOSE_BRACE)
	{
		if (currentToken.type == TOKEN_LOCATION)
		{
			Location* location = parseLocationBlock(server);
			server->addLocation(location);
		}
		else if (currentToken.type == TOKEN_DIRECTIVE)
			parseServerDirective(server);
		else
			throw ParseException("Unexpected token in server block", currentToken.line);
	}

	expect(TOKEN_CLOSE_BRACE);
	return (server);
}

Location* DirectiveParser::parseLocationBlock(Server* server)
{
	expect(TOKEN_LOCATION);

	if (currentToken.type != TOKEN_STRING)
		throw ParseException("Expected location path", currentToken.line);

	std::string path = currentToken.value;
	advance();
	expect(TOKEN_OPEN_BRACE);

	Location* location = new Location(path);
	location->inheritFrom(server);

	while (currentToken.type != TOKEN_CLOSE_BRACE)
	{
		if (currentToken.type == TOKEN_DIRECTIVE)
			parseLocationDirective(location);
		else
			throw ParseException("Unexpected token in location block", currentToken.line);
	}

	expect(TOKEN_CLOSE_BRACE);
	return location;
}

void DirectiveParser::parseServerDirective(Server* server)
{
	std::string directive = currentToken.value;
	advance();
	std::vector<std::string> values = gatherDirectiveValues();

	if (directive == "listen")
		parseListen(server, values);
	else if (directive == "server_name")
		parseServerName(server, values);
	else if (directive == "root")
		parseRoot(server, NULL, values);
	else if (directive == "error_page")
		parseErrorPage(server, values);
	else if (directive == "client_body_limit")
		parseClientBodyLimit(server, NULL, values);
	else if (directive == "autoindex")
		parseAutoindex(server, NULL, values);
	else if (directive == "index")
		parseIndex(server, NULL, values);
	else
		throw ParseException("Unknown server directive: " + directive, currentToken.line);
}

void DirectiveParser::parseLocationDirective(Location* location)
{
	std::string directive = currentToken.value;
	advance();
	std::vector<std::string> values = gatherDirectiveValues();

	if (directive == "allow_methods")
		parseAllowMethods(location, values);
	else if (directive == "root")
		parseRoot(NULL, location, values);
	else if (directive == "client_body_limit")
		parseClientBodyLimit(NULL, location, values);
	else if (directive == "autoindex")
		parseAutoindex(NULL, location, values);
	else if (directive == "index")
		parseIndex(NULL, location, values);
	else if (directive == "cgi_info")
		parseCgiInfo(location, values);
	else if (directive == "return")
		parseReturn(location, values);
	else if (directive == "upload_store")
		parseUploadStore(location, values);
	else
		throw ParseException("Unknown location directive: " + directive, currentToken.line);
}

void DirectiveParser::parseListen(Server* server, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("listen directive requires exactly one value", currentToken.line);

	const std::string& value = values[0];
	size_t colon = value.find(':');

	if (colon == std::string::npos)
	{
		server->setHost("0.0.0.0");
		server->setPort(value);
	}
	else
	{
		server->setHost(value.substr(0, colon));
		server->setPort(value.substr(colon + 1));
	}

	char* end;
	long port = std::strtol(server->getPort().c_str(), &end, 10);
	if (*end != '\0' || port < 1 || port > 65535)
		throw ParseException("Invalid port number: " + server->getPort(), currentToken.line);
}

void DirectiveParser::parseClientBodyLimit(Server* server,
		Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("client_body_limit requires exactly one value", currentToken.line);

	size_t limit = parseSize(values[0]);

	if (location)
		location->setClientBodyLimit(limit);
	else
		server->setClientBodyLimit(limit);
}

void DirectiveParser::expect(TokenType expected)
{
	if (currentToken.type != expected)
		throw ParseException("Unexpected token", currentToken.line);
	advance();
}

void DirectiveParser::advance()
{
	currentToken = lexer.getNextToken();
}

std::vector<std::string> DirectiveParser::gatherDirectiveValues()
{
	std::vector<std::string> values;

	while (currentToken.type != TOKEN_SEMICOLON
			&& currentToken.type != TOKEN_EOF)
	{
		if (currentToken.type == TOKEN_STRING
				|| currentToken.type == TOKEN_NUMBER || 
				currentToken.type == TOKEN_IP_PORT)
			values.push_back(currentToken.value);
		else
			throw ParseException("Unexpected token in directive values", currentToken.line);
		advance();
	}

	expect(TOKEN_SEMICOLON);
	return values;
}

size_t DirectiveParser::parseSize(const std::string& sizeStr)
{
	char* end;
	long size = std::strtol(sizeStr.c_str(), &end, 10);

	if (*end != '\0')
	{
		char unit = std::toupper(*end);
		switch (unit)
		{
			case 'K': size *= 1024; break;
			case 'M': size *= 1024 * 1024; break;
			case 'G': size *= 1024 * 1024 * 1024; break;
			default: throw ParseException("Invalid size unit: " + sizeStr, currentToken.line);
		}
	}

	if (size <= 0)
		throw ParseException("Size must be positive: " + sizeStr, currentToken.line);
	return static_cast<size_t>(size);
}
