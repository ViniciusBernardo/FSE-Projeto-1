all:
	gcc -Wall -o bin/bin main.c -lwiringPi -lbcm2835 -lpthread

run:
	./bin/bin

.PHONY: clean
clean:
	rm -rf bin/*
