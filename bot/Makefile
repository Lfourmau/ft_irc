NAME		=	ircbot

INC_DIR		=	inc
INC			=	bot.hpp
INCS		=	$(addprefix $(INC_DIR)/, $(INC))

SRC_DIR		=	src
SRC			=	main.cpp
SRCS		=	$(addprefix $(SRC_DIR)/, $(SRC))

OBJ_DIR		=	obj
OBJ			=	$(SRC:.cpp=.o)
OBJS		=	$(addprefix $(OBJ_DIR)/, $(OBJ))

CXX			=	clang++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

RM			=	rm
RFLAGS		=	-rf

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $< -I $(INC_DIR)/

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o ${NAME} $(OBJS)

clean : 
	$(RM) $(RFLAGS) $(OBJS)
	$(RM) $(RFLAGS) *.dSYM
	$(RM) $(RFLAGS) $(OBJ_DIR)

fclean : clean
	$(RM) $(RFLAGS) $(NAME)

re : fclean all

.PHONY : all clean fclean re run
