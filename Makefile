SOURCE=countChars.cpp
NAME=countChars

CC=g++

all: $(NAME)

$(NAME): $(SOURCE)
	$(CC) $(SOURCE) -o $(NAME)

clean: 
	rm -f $(NAME) *.o 
