#include "inc/WebServ.hpp"
#include <fstream>
#include <iterator>
#include <string>

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
		std::cerr << RED"Error: Not a regular file,\n" << RESET;
		return (false);
	}

	return (true);
}

std::string	readFile(const char *config_file)
{
	std::ifstream	input_file(config_file);

	if (!input_file)
	{
		std::cerr << RED"Error: opening " << config_file
				<< ": " << std::strerror(errno) << ".\n" RESET;
		return ("");
	}

	std::string	content(std::istream_iterator<char>(input_file),
				(std::istream_iterator<char>()));
	return (content);
}

std::string	readConfigFile(int argc, char *argv[])
{
	const char		*config_file;
	config_file = (argc == 1) ? "configs/default.conf" : argv[1];

	if (!isRegularFile(config_file))
		return ("");

	std::string	config_content = readFile(config_file);
	if (config_content.empty())
		return ("");
	return (config_content);
}

// bool	parseConfig(const std::string &config_content)
// {
//
// 	return (true);
// }

int	main(int argc, char *argv[])
{
	std::string	config_content;

	if (argc > 2)
	{
		std::cerr << RED"Error:\n"
			<< "Usage: ./webserv [configuration file]\n" << RESET;
		return (1);
	}
	config_content = readConfigFile(argc, argv);
	if (config_content.empty())
		return (1);
	// parseConfig(config_content);
	return (0);
}
