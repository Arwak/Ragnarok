OBJECTS = main.o io.o /filesystem/ext.o /filesystem/fat.o /filesystem/filesystem.o
SOURCE = main.c io.c /filesystem/ext.c /filesystem/fat.c /filesystem/filesystem.c /filesystem/ext.h /filesystem/fat.h /filesystem/filesystem.h
CFLAGS = -Wall -Wextra -g -pthread -lpthread
LDLIBS =

CC = gcc -D_GNU_SOURCE
OUT = ragnarok

all : $(OUT)
$(OUT) :  $(OBJECTS)
	$(CC)  $(OBJECTS) -o $(OUT) $(CFLAGS)
clean :
	$(RM) $(OBJECTS) main