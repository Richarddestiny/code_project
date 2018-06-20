CC = gcc
CFLAGS = -O0 -Wall -g

all:threadpool a

threadpool:threadpool.c
	$(CC) -o $@ $^ -lpthread $(CFLAGS)

a:a.c
	$(CC) -o $@ $^ -lpthread $(CFLAGS)

clean:
	$(RM) .*.sw? threadpool *.o
.PHONY:clean
