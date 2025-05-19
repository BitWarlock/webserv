#include "inc/WebServ.hpp"

bool	has_conf_extension(const std::string &config_file)
{
	if (config_file.length() < 5)
		return (false);
	return (".conf" == config_file.substr(config_file.length() - 5));
}

bool	regular_file(const char *config_file)
{
	struct stat st;

	if (!has_conf_extension(config_file))
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

bool	valid_config_file(int argc, char *argv[])
{
	const char		*config_file;
	config_file = (argc == 1) ? "configs/default.conf" : argv[1];

	if (!regular_file(config_file))
		return (false);
	std::ifstream	input_file(config_file);

	if (input_file.peek() == EOF)
		return (std::cerr << RED"Error: Empty file.\n" RESET,
		false);
	return (true);
}

int	main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << RED"Error:\n"
			<< "Usage: ./webserv [configuration file]\n" << RESET;
		return (1);
	}
	if (!valid_config_file(argc, argv))
		return (false);
	return (0);
}
