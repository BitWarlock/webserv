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

		// 4. SetupServers
		SetupServers	server(*config);

		// 4. Print parsed configuration (for testing)
		std::cout << GRN "Successfully parsed configuration:\n" << RESET;
		std::cout << "Found " << config->getServers().size() << " server blocks\n\n";

		// 5. Iterate through servers
		for (size_t i = 0; i < config->getServers().size(); ++i) {
			const Server* server = config->getServers()[i];
			std::cout << "Server #" << i + 1 << ":\n";
			for (size_t i = 0; i < server->getListen().size(); i++)
			{
				std::cout << "  Host: " << server->getListen()[i].first << "\n";
				std::cout << "  Port: " << server->getListen()[i].second << "\n";
			}
			if (server->getReturnDirective().enabled)
				std::cout << "  Return " << server->getReturnDirective().status_code <<
					": Target: " << server->getReturnDirective().target << "\n";

			// Print server names if they exist
			if (!server->getServerNames().empty()) {
				std::cout << "  Server Names:";
				for (size_t j = 0; j < server->getServerNames().size(); ++j) {
					std::cout << " " << server->getServerNames()[j];
				}
				std::cout << "\n";
			}

			// Print locations
			std::cout << "  Locations:\n";
			const std::vector<Location*>& locations = server->getLocations();
			for (size_t j = 0; j < locations.size(); ++j) {
				const Location* loc = locations[j];
				std::cout << "    Path: " << loc->getPath() << "\n";
				if (loc->getReturnDirective().enabled)
					std::cout << "    Return " << loc->getReturnDirective().status_code <<
						": Target: " << loc->getReturnDirective().target << "\n";
				std::cout << "      Methods:";
				for (size_t k = 0; k < loc->getAllowedMethods().size(); ++k) {
					std::cout << " " << loc->getAllowedMethods()[k];
				}
				std::cout << "\n";

				if (!loc->getRoot().empty()) {
					std::cout << "      Root: " << loc->getRoot() << "\n";
				}

				if (loc->getClientBodyLimit() != 0) {
					std::cout << "      Client Body Limit: " << loc->getClientBodyLimit() << "\n";
				}
			}
			std::cout << "\n";
		}

		// 6. Cleanup
		delete config;

	} catch (const std::exception& e) {
		std::cerr << RED "Configuration error: " << e.what() << RESET "\n";
		return 1;
	}
	return 0;
}
