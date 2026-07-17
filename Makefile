NAME = graphical_test

all:
	cmake -B build -G Ninja
	cmake --build build
	cp build/$(NAME) ./$(NAME)

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
