main:main.o
	gcc main.c -o main -lwiringPi
clean:
	rm -f main main.o
