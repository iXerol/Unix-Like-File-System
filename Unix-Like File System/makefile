OBJS=strings.o main
main.o: main.c strings.o
	clang -o main main.c strings.o
strings.o: strings.c
	clang -o strings.o -c strings.c
clean:
	rm $(OBJS)