OBJECTS = main.o io.o filesystem/filesystem.o filesystem/ext.o filesystem/fat.o
SOURCE = main.c io.c filesystem/filesystem.c filesystem/ext.c filesystem/fat.c filesystem/filesystem.h filesystem/ext.h filesystem/fat.h
CFLAGS = -Wall -Wextra -g -pthread -lpthread -lm
LDLIBS =

CC = gcc -D_GNU_SOURCE
OUT = ragnarok

all : $(OUT)
$(OUT) :  $(OBJECTS)
	$(CC)  $(OBJECTS) -o $(OUT) $(CFLAGS)
clean :
	$(RM) $(OBJECTS) main
