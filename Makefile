CXX = g++-11
CPPFLAGS = -std=c++98 -Wall -Werror -Wextra -g3 -fsanitize=address

NAME = ircserv

all: $(NAME)

$(NAME): server.cpp
	$(CXX) $(CPPFLAGS) server.cpp -o $(NAME)

clean:
	$(RM) $(NAME)
	$(RM) -r *.dSYM

fclean: clean

re: fclean all
