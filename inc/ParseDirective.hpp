#ifndef DIRECTIVE_PARSER_HPP
#define DIRECTIVE_PARSER_HPP

#include "Config.hpp"
#include "Lexer.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <sstream>

class DirectiveParser
{
	public:
		DirectiveParser(Lexer& lexer);
		~DirectiveParser();

		Config* parseConfig();

	private:
		Lexer&					lexer;
		Token					currentToken;

		// Block parsing
		Server*		parseServerBlock();
		Location*	parseLocationBlock(Server* server);

		// Directive parsing
		void parseServerDirective(Server* server);
		void parseLocationDirective(Location* location);

		// Specialized parsers
		void parseListen(Server* server, const std::vector<std::string>& values);
		void parseTimeout(Server* server, const std::vector<std::string>& values);
		void parseServerName(Server* server, const std::vector<std::string>& values);
		void parseRoot(Server* server, Location* location, std::vector<std::string>& values);
		void parseErrorPage(Server* server, const std::vector<std::string>& values);
		void parseReturn(Server* server, const std::vector<std::string>& values);
		void parseReturnLoc(Location *location, const std::vector<std::string>& values);
		void parseClientBodyLimit(Server* server, Location* location, const std::vector<std::string>& values);
		void parseAutoindex(Server* server, Location* location, const std::vector<std::string>& values);
		void parseIndex(Server* server, Location* location, const std::vector<std::string>& values);
		void parseAllowMethods(Location* location, const std::vector<std::string>& values);
		void parseAllowMethodsServ(Server *server, const std::vector<std::string>& values);
		void parseCgiInfo(Location* location, const std::vector<std::string>& values);
		void parseReturn(Location* location, const std::vector<std::string>& values);
		void parseUploadStore(Location* location, const std::vector<std::string>& values);

		// Helpers
		void						expect(TokenType expected);
		void						advance();
		std::vector<std::string>	gatherDirectiveValues();
		bool						isLocationDirective(const std::string& directive) const;
		size_t						parseSize(const std::string& sizeStr, std::string size);
};

static std::string intToString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

class ParseException : public std::runtime_error
{
	public:
		ParseException(const std::string& msg, int line) 
			: std::runtime_error(msg + " at line " + intToString(line)) {}
};

#endif
