NAME = server

INC = 	server.hpp \
		channel.hpp \
		user.hpp

SRCS = 	main.cpp \
		channel.cpp \
		user.cpp \
		server.cpp \

OBJS = $(SRCS:.cpp=.o)


CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address
CXX = clang++

all : $(NAME)


%.o : %.cpp $(INC)
	$(CXX) -c $(CXXFLAGS) $< -o $(<:.cpp=.o) -g

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o ${NAME} $(OBJS)

run : $(NAME)
	./$(NAME) 

clean : 
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re