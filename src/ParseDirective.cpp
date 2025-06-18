#include "../inc/ParseDirective.hpp"
#include <clocale>
#include <exception>
#include <utility>

DirectiveParser::DirectiveParser(Lexer& lexer) : lexer(lexer) {}

DirectiveParser::~DirectiveParser() {}

void	validateMandatoryDirectives(const Config* config)
{
	const std::vector<Server*>& servers = config->getServers();
	
	for (size_t i = 0; i < servers.size(); i++)
	{
		const Server* server = servers[i];
		const std::vector<Location*>& locations = server->getLocations();
		bool hasRootLocation = false;
		for (size_t j = 0; j < locations.size(); j++)
		{
			if (locations[j]->getPath() == "/")
			{
				hasRootLocation = true;
				break;
			}
		}

		if (!hasRootLocation && server->getRoot().empty())
			throw std::runtime_error(
					"Server block must have either a `root` directive or a `location /` block");

		if (server->getAllowedMethods().empty())
			throw std::runtime_error("Server must specify `allowed_methods`");

		for (size_t j = 0; j < locations.size(); j++)
		{
			const Location* loc = locations[j];
			if (loc->getAllowedMethods().empty())
				throw std::runtime_error("Location `" +
						loc->getPath() + "` must specify `allowed_methods`");
		}
	}
}

Config* DirectiveParser::parseConfig()
{
	Config* config = new Config();
	currentToken = lexer.getNextToken();

	try
	{
		while (currentToken.type != TOKEN_EOF)
		{
			if (currentToken.type == TOKEN_SERVER)
			{
				Server* server = parseServerBlock();
				if (server->getListen().empty())
					server->setListen(std::make_pair("0.0.0.0", "8000"));
				config->addServer(server);
			}
			else
				throw ParseException("Unexpected token outside server block", currentToken.line);
		}
	}
	catch (...)
	{
		delete config;
		throw;
	}

	try
	{
		validateMandatoryDirectives(config);
	}
	catch (...)
	{
		delete config;
		throw;
	}
	return (config);
}

Server* DirectiveParser::parseServerBlock()
{
	expect(TOKEN_SERVER);
	expect(TOKEN_OPEN_BRACE);

	Server* server = new Server();

	try
	{
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
	}
	catch (...)
	{
		delete server;
		throw;
	}

	expect(TOKEN_CLOSE_BRACE);
	return (server);
}

Location* DirectiveParser::parseLocationBlock(Server* server)
{
	expect(TOKEN_LOCATION);

	if (currentToken.type != TOKEN_STRING && currentToken.type != TOKEN_DIRECTIVE)
		throw ParseException("Expected location path", currentToken.line);

	std::string path = currentToken.value;
	advance();
	expect(TOKEN_OPEN_BRACE);

	Location* location = new Location(path);
	location->inheritFrom(server);

	try
	{
		while (currentToken.type != TOKEN_CLOSE_BRACE)
		{
			if (currentToken.type == TOKEN_DIRECTIVE)
				parseLocationDirective(location);
			else
				throw ParseException("Unexpected token in location block", currentToken.line);
		}
	}
	catch (...)
	{
		delete location;
		throw;
	}

	expect(TOKEN_CLOSE_BRACE);
	return location;
}

void DirectiveParser::parseAllowMethodsServ(Server *server, const std::vector<std::string>& values)
{
	if (values.empty())
		throw ParseException("allowed_methods directive requires at least one value", currentToken.line);

	for (size_t i = 0; i < values.size(); ++i)
		server->addAllowedMethod(values[i]);
}

void DirectiveParser::parseServerDirective(Server* server)
{
	std::string directive = currentToken.value;
	advance();
	std::vector<std::string> values = gatherDirectiveValues();

	if (directive == "allowed_methods")
		parseAllowMethodsServ(server, values);
	else if (directive == "listen")
		parseListen(server, values);
	else if (directive == "server_name")
		parseServerName(server, values);
	else if (directive == "root")
		parseRoot(server, NULL, values);
	else if (directive == "error_page")
		parseErrorPage(server, values);
	else if (directive == "return")
		parseReturn(server, values);
	else if (directive == "client_max_body_size")
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

	if (directive == "allowed_methods")
		parseAllowMethods(location, values);
	else if (directive == "root")
		parseRoot(NULL, location, values);
	else if (directive == "client_max_body_size")
		parseClientBodyLimit(NULL, location, values);
	else if (directive == "autoindex")
		parseAutoindex(NULL, location, values);
	else if (directive == "index")
		parseIndex(NULL, location, values);
	else if (directive == "cgi_info")
		parseCgiInfo(location, values);
	else if (directive == "return")
		parseReturnLoc(location, values);
	else if (directive == "upload_store")
		parseUploadStore(location, values);
	else
		throw ParseException("Unknown location directive: " + directive, currentToken.line);
}

static bool	isValidIPv4(const std::string& host)
{
	std::vector<std::string>	octets;
	std::stringstream			ss(host);
	std::string					octet;

	while (std::getline(ss, octet, '.'))
		octets.push_back(octet);

	if (octets.size() != 4)
		return false;

	for (size_t i = 0; i < 4; ++i)
	{
		char* end;
		long num = strtol(octets[i].c_str(), &end, 10);
		if (*end != '\0' || num < 0 || num > 255)
			return false;
	}
	return true;
}

static bool	UniqueListen(Server *server, std::pair<std::string, std::string> pair)
{
	for (size_t i = 0; i < server->getListen().size(); i++)
		if (pair == server->getListen()[i])
			return (false);
	return (true);
}

void DirectiveParser::parseListen(Server* server, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("listen directive requires exactly one value", currentToken.line);

	const std::string& value = values[0];
	size_t colon = value.find(':');

	std::string host, port;

	if (colon == std::string::npos)
	{
		host = "0.0.0.0";
		port = value;
	}
	else
	{
		host = value.substr(0, colon);
		port = value.substr(colon + 1);
	}

	char* end;
	long portNumber = strtol(port.c_str(), &end, 10);
	if (*end != '\0' || portNumber < 1 || portNumber > 65535)
		throw ParseException("Invalid port number: " + port, currentToken.line);
	if (host != "localhost" && host != "0.0.0.0" && !isValidIPv4(host))
		throw ParseException("Listen must be a valid IPV4 address (e.g. 127.0.0.1) " + host, currentToken.line);
	if (host == "localhost")
		host = "127.0.0.1";

	if (UniqueListen(server, std::make_pair(host, port)))
		server->setListen(std::make_pair(host, port));
}

void DirectiveParser::parseClientBodyLimit(Server* server,
		Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("client_max_body_size requires exactly one value", currentToken.line);

	size_t limit = parseSize(values[0]);

	if (location)
		location->setClientBodyLimit(limit);
	else
		server->setClientBodyLimit(limit);
}

void DirectiveParser::parseServerName(Server* server, const std::vector<std::string>& values)
{
	for (size_t i = 0; i < values.size(); ++i)
		server->addServerName(values[i]);
}

void DirectiveParser::parseRoot(Server* server, Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("root directive requires exactly one value", currentToken.line);

	if (location)
		location->setRoot(values[0]);
	else if (server)
		server->setRoot(values[0]);
}

void DirectiveParser::parseReturnLoc(Location *location, const std::vector<std::string>& values)
{
	if (values.size() < 1 || values.size() > 2)
		throw ParseException("return directive must have 1 or 2 values", currentToken.line);

	int code = atoi(values[0].c_str());
	if (code < 100 || code > 599)
		throw ParseException("Invalid HTTP status code", currentToken.line);

	ReturnDirective	return_d;

	return_d.enabled = true;
	return_d.status_code = code;
	return_d.target = (values.size() == 1) ? "" : values[1];
	location->addReturnDirective(return_d);
}

void DirectiveParser::parseReturn(Server* server, const std::vector<std::string>& values)
{
	if (values.size() < 1 || values.size() > 2)
		throw ParseException("return directive must have 1 or 2 values", currentToken.line);

	int code = atoi(values[0].c_str());
	if (code < 100 || code > 599)
		throw ParseException("Invalid HTTP status code", currentToken.line);

	ReturnDirective	return_d;

	return_d.enabled = true;
	return_d.status_code = code;
	return_d.target = (values.size() == 1) ? "" : values[1];
	server->addReturnDirective(return_d);
}

void DirectiveParser::parseErrorPage(Server* server, const std::vector<std::string>& values)
{
	if (values.size() < 2)
		throw ParseException("error_page directive requires at least two values", currentToken.line);

	int code = atoi(values[0].c_str());
	if (code < 400 || code > 599)
		throw ParseException("Invalid error code (must be 4xx or 5xx)", currentToken.line);

	for (size_t i = 1; i < values.size(); ++i)
		server->addErrorPage(code, values[i]);
}

void DirectiveParser::parseAutoindex(Server* server, Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("autoindex directive requires exactly one value (on/off)", currentToken.line);

	bool autoindex = (values[0] == "on");
	if (location)
		location->setAutoindex(autoindex);
	else if (server)
		server->setAutoindex(autoindex);
}

void DirectiveParser::parseIndex(Server* server, Location* location, const std::vector<std::string>& values)
{
	if (values.empty())
		throw ParseException("index directive requires at least one value", currentToken.line);

	for (size_t i = 0; i < values.size(); ++i)
	{
		if (location)
			location->addIndex(values[i]);
		else if (server)
			server->addIndex(values[i]);
	}
}

void DirectiveParser::parseAllowMethods(Location* location, const std::vector<std::string>& values)
{
	if (values.empty())
		throw ParseException("allowed_methods directive requires at least one value", currentToken.line);

	for (size_t i = 0; i < values.size(); ++i)
		location->addAllowedMethod(values[i]);
}

void DirectiveParser::parseCgiInfo(Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 2)
		throw ParseException("cgi_info directive requires exactly two values (extension and interpreter)", currentToken.line);

	location->setCgiExtension(values[0], values[1]);
}

void DirectiveParser::parseUploadStore(Location* location, const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw ParseException("upload_store directive requires exactly one value (path)", currentToken.line);

	location->setUploadStore(values[0]);
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
	std::vector<std::string>	values;
	int							directiveLine = currentToken.line;

	while (currentToken.type != TOKEN_SEMICOLON
			&& currentToken.type != TOKEN_EOF)
	{
		if (currentToken.type == TOKEN_STRING
		 || currentToken.type == TOKEN_NUMBER || 
			currentToken.type == TOKEN_IP_PORT
		 || currentToken.type == TOKEN_DIRECTIVE)
			values.push_back(currentToken.value);
		else
			throw ParseException("Unexpected token in directive values", currentToken.line);

		if (currentToken.line != directiveLine && currentToken.type != TOKEN_SEMICOLON)
			throw ParseException("Expected semicolon at the end of line ", currentToken.line);
		advance();
	}

	expect(TOKEN_SEMICOLON);
	return values;
}

size_t DirectiveParser::parseSize(const std::string& sizeStr)
{
	char* end;
	long size = strtol(sizeStr.c_str(), &end, 10);

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
