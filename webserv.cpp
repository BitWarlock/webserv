#include "inc/WebServ.hpp"

bool	hasConfExt(const std::string &config_file)
{
	if (config_file.length() < 5)
		return (false);
	return (".conf" == config_file.substr(config_file.length() - 5));
}

bool	isRegularFile(const char *config_file)
{
	struct stat st;

	if (!hasConfExt(config_file))
	{
		std::cerr << RED"Error: Configuration file must have a .conf extension.\n" << RESET;
		return (false);
	}

	if (stat(config_file, &st) != 0)
	{
		std::cerr << RED"Error: opening " << config_file
				<< ": " << std::strerror(errno) << ".\n" RESET;
		return (false);
	}

	if (!S_ISREG(st.st_mode))
	{
		std::cerr << RED"Error: Not a regular file.\n" << RESET;
		return (false);
	}

	return (true);
}

std::string readFile(const char *config_file)
{
	std::ifstream input_file(config_file);
	if (!input_file)
	{
		std::cerr << "Error: opening " << config_file
			<< ": " << std::strerror(errno) << ".\n";
		return "";
	}

	std::ostringstream ss;
	ss << input_file.rdbuf();

	return ss.str();
}

std::string	readConfigFile(int argc, char *argv[])
{
	const char		*config_file;
	config_file = (argc == 1) ? "configs/default.conf" : argv[1];

	if (!isRegularFile(config_file))
		return ("");

	std::string	config_content = readFile(config_file);
	if (config_content.empty())
	{
		std::cerr << RED"Error: Empty file.\n" << RESET;
		return ("");
	}
	return (config_content);
}

int main(int argc, char *argv[])
{
	std::string config_content;

	if (argc > 2)
	{
		std::cerr << RED"Error:\n"
			<< "Usage: ./webserv [configuration file]\n" << RESET;
		return 1;
	}

	// 1. Read config file
	config_content = readConfigFile(argc, argv);
	if (config_content.empty())
		return 1; 
	try
	{
		// 2. Initialize lexer and parser
		Lexer lexer(config_content);
		DirectiveParser parser(lexer);

		// 3. Parse configuration
		Config* config = parser.parseConfig();

		// 4. SetupServer
		SetupServers	server(*config);
	}
	
	catch (const std::exception& e) {
		std::cerr << RED "Configuration error: " << e.what() << RESET "\n";
		return 1;
	}
	return 0;
}
