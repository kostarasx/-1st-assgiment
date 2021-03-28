CC= gcc

CFLAGS	:= -Wall -Wextra -O3 -g -pthread


# !DELETE 
#LFLAGS =
SRC		:= src
INCLUDE	:= inc
LIB		:= lib

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= main.exe
SOURCEDIRS	:= $(SRC)
else
EXECUTABLE	:= main
SOURCEDIRS	:= $(shell find $(SRC) -type d)

endif


SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))
OBJECTS		:= $(SOURCES:.c=.o)


all: $(EXECUTABLE)


.PHONY: clean
clean:
	-$(RM) $(EXECUTABLE)
	-$(RM) $(OBJECTS)


run: all
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS)  $^ -o $@ -lm