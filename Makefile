TARGET=morse
CFLAGS=-g -Wall
OBJS=morse.o encoding.o
CC=gcc

$(TARGET):morse.c encoding.c encoding.h
	$(CC) -c encoding.c morse.c
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS)
clean:
	rm -f $(TARGET) $(OBJS)
