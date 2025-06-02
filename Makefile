snake: snake.o circular_uint32.o
	gcc snake.o circular_uint32.o -o snake -lncurses

snake.o: snake.c
	gcc -c snake.c -o snake.o -Wall -Werror

circular_uint32.o: circular_uint32.c
	gcc -c circular_uint32.c -o circular_uint32.o -Wall -Werror

clean:
	rm -f snake.o circular_uint32.o snake
