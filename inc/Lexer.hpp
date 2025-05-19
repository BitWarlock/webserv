#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>

enum TokenType
{
    TOKEN_SERVER,
    TOKEN_LOCATION,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_SEMICOLON,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_DIRECTIVE,
    TOKEN_EOF,
    TOKEN_ERROR
};

struct Token
{
	Token(TokenType	type, std::string value, int line) : type(type), value(value), line(line)
	{}

	TokenType	type;
	std::string	value;
	int			line;
};

class Lexer
{
	public:
		Lexer(const std::string &input);

		Token	getNextToken();
	
	private:
		std::string	input;
		size_t		pos;
		int			line;

		char	currentChar();
		
		Token	readString();
		Token	readWord();
		Token	readNumber();

		void	skipWhiteSpaces();
		void	advance();
		void	skipComment();
};

#endif // !LEXER_HPP
