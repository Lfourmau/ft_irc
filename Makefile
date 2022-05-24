NAME = server

SRCS = server.cpp

OBJS = $(SRCS:.cpp=.o)


CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address
CXX = clang++

all : $(NAME)


%.o : %.cpp $(INC)
	$(CXX) -c $(CXXFLAGS) $< -o $(<:.cpp=.o) -g

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o ${NAME} $(OBJS)



clean : 
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re