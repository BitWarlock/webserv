CC      = c++
NAME    = webserv
CFLAGS  = -Wall -Wextra -Werror -std=c++98 -MMD -MP -g

SRCS    = webserv.cpp src/Lexer.cpp src/ParseDirective.cpp src/Config.cpp src/Server.cpp src/Location.cpp src/SetupServers.cpp src/ParseRequest.cpp src/Response.cpp src/Cgi.cpp src/SessionManager.cpp
OBJS    = $(SRCS:%.cpp=objs/%.o)
DEPS    = $(OBJS:.o=.d)

all: $(NAME)
	@printf "\033[1;32m[✓] Build complete: %s\033[0m\n" $(NAME)

$(NAME): $(OBJS)
	@printf "\033[1;34m[✓] Linking object files...\033[0m\n"
	@$(CC) $(CFLAGS) $^ -o $@

objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	@printf "\033[1;36m[✓] Compiling %s...\033[0m\n" $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@printf "\033[1;33m[✓] Cleaning object files...\033[0m\n"
	@$(RM) -r objs

fclean: clean
	@printf "\033[1;31m[✓] Removing executable: %s\033[0m\n" $(NAME)
	@$(RM) $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: clean
