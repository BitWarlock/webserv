#include "../inc/Lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string &input) : input(input), pos(0), line(0)
{}

Token	Lexer::getNextToken()
{
	while (currentChar() != '\0')
	{
		char	chr = currentChar();

		if (std::isspace(chr)) { skipWhiteSpaces(); continue; }
		if (chr == '#') { skipComment(); continue; }
		if (chr == '{')
		{
			advance();
			return (Token(TOKEN_OPEN_BRACE, "{", line));
		}
		if (chr == '}')
		{
			advance();
			return (Token(TOKEN_CLOSE_BRACE, "}", line));
		}
		if (chr == ';')
		{
			advance();
			return (Token(TOKEN_SEMICOLON, ";", line));
		}
		if (chr == '"') { return readString(); };
		if (std::isdigit(chr)) { return readNumber(); };
	}

	return (Token(TOKEN_EOF, "", line));
}

char	Lexer::currentChar()
{
	return (pos < input.length() ? input[pos] : '\0');
}

void	Lexer::advance() { pos++; }

void	Lexer::skipWhiteSpaces()
{
	while (currentChar() != '\0' && std::isspace(currentChar()))
		if (currentChar() == '\n') line++, advance();
}

void	Lexer::skipComment()
{
	while (currentChar() != '\0' && currentChar() != '\n')
		advance();
	if (currentChar() == '\n')
		line++, advance();
}

Token	Lexer::readString()
{
	std::string	value;

	advance();
	while (currentChar() != '\0' && currentChar() != '"')
	{
		if (currentChar() == '\n') line++;
		value += currentChar();
		advance();
	}

	if (currentChar() == '"')
		advance();
	else
		return Token(TOKEN_ERROR, "Unterminated string", line);
	return (Token(TOKEN_STRING, value, line));
}

Token	Lexer::readNumber()
{
	std::string	value;

	while (currentChar() != '\0' && std::isdigit(currentChar()))
		value += currentChar(), advance();
	return (Token(TOKEN_NUMBER, value, line));
}
