#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>

class Lexer;
struct ServerConfig;

class Parser
{
	public:
		Parser(Lexer &lexer);

		bool	parse(std::vector<ServerConfig> &servers);

	private:
		Lexer		&lexer;
};

#endif // !PARSER_HPP
