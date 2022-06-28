NAME = ircserv

INC = 	server.hpp \
		channel.hpp \
		user.hpp	\
		rpl.hpp

SRCS =  channel.cpp \
		user.cpp \
		server.cpp \
		string_maker.cpp \

OBJS = $(SRCS:.cpp=.o)


CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
CXX = clang++

all : $(NAME)


%.o : %.cpp $(INC)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(NAME) : $(OBJS) main.cpp
	$(CXX) $(CXXFLAGS) -o ${NAME} main.cpp $(OBJS)

run : $(NAME)
	./$(NAME) 

clean : 
	rm -f $(OBJS)
	$(RM) -r *.dSYM

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re
