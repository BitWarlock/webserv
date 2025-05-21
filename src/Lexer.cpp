#include "../inc/Lexer.hpp"

Lexer::Lexer(const std::string &input) : input(input), pos(0), line(0)
{}

Token Lexer::getNextToken()
{
	while (currentChar() != '\0')
	{
		char chr = currentChar();

		if (std::isspace(chr)) { skipWhiteSpaces(); continue; }
		if (chr == '#') { skipComment(); continue; }
		if (chr == '{')
		{
			advance();
			return Token(TOKEN_OPEN_BRACE, "{", line);
		}
		if (chr == '}')
		{
			advance();
			return Token(TOKEN_CLOSE_BRACE, "}", line);
		}
		if (chr == ';')
		{
			advance();
			return Token(TOKEN_SEMICOLON, ";", line);
		}
		if (chr == '"') { return readString(); }
		if (std::isdigit(chr))
		{
			Token num = readNumber();
			if (num.value.find('.') != std::string::npos ||
					num.value.find(':') != std::string::npos)
				num.type = TOKEN_IP_PORT;
			return num;
		}
		return readWord();
	}
	return Token(TOKEN_EOF, "", line);
}

char	Lexer::currentChar()
{
	return (pos < input.length() ? input[pos] : '\0');
}

void	Lexer::advance() { pos++; }

void Lexer::skipWhiteSpaces()
{
	while (currentChar() != '\0' && std::isspace(currentChar()))
	{
		if (currentChar() == '\n')
			line++;
		advance();
	}
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

	if (currentChar() != '"')
		return Token(TOKEN_ERROR, "Unterminated string", line);
	advance();
	return (Token(TOKEN_STRING, value, line));
}

Token	Lexer::readNumber()
{
	std::string	value;

	while (currentChar() != '\0' && 
			(std::isdigit(currentChar()) || currentChar() == '.'
						  || currentChar() == ':'))
		value += currentChar(), advance();

	if (value.find_first_of("0123456789") == std::string::npos)
		return (Token(TOKEN_ERROR, "Invalid number", line));
	return (Token(TOKEN_NUMBER, value, line));
}

bool isKnownDirective(const std::string& word)
{
	static const std::string directiveArray[] = {
		"listen", "server_name", "root", "error_page", "client_body_limit",
		"autoindex", "index", "allow_methods", "cgi_info", "return", "upload_store"
	};
	static const std::set<std::string> directives(
			directiveArray, directiveArray + sizeof(directiveArray)/sizeof(std::string)
			);
	return directives.find(word) != directives.end();
}

Token Lexer::readWord()
{
	std::string value;
	char chr = currentChar();

	while (chr != '\0' && !std::isspace(chr) &&
			chr != '{' && chr != '}' &&
			chr != ';' && chr != '#')
	{
		value += chr;
		advance();
		chr = currentChar();
	}

	if (value == "server")
		return Token(TOKEN_SERVER, value, line);
	if (value == "location")
		return Token(TOKEN_LOCATION, value, line);

	if (isKnownDirective(value))
		return Token(TOKEN_DIRECTIVE, value, line);

	return Token(TOKEN_DIRECTIVE, value, line);
}
