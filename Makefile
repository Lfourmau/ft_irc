CXX = g++-11
CPPFLAGS = -std=c++98 -I . -Wall -Werror -Wextra -g3 #-fsanitize=address

NAME = ircserv

SRCS = server_lowlevel.cpp channel.cpp user.cpp server.cpp

HEADERS = $(OBJS:.cpp=.hpp)

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@ 

$(NAME): $(OBJS) main.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) $(OBJS) main.cpp -o $(NAME)

clean:
	$(RM) $(NAME)
	$(RM) $(OBJS)
	$(RM) -r *.dSYM

fclean: clean

re: fclean all
