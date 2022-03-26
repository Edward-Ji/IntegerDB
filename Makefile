TARGET = ymirdb

CC = gcc

CFLAGS = -c -Wall -Wvla -Werror -g -std=gnu11 -Werror=format-security
LDFLAGS = -fsanitize=address,leak -static-libasan
SRC = ymirdb.c
OBJ = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $<

test:
	./run_test

clean:
	rm -f *.o *.obj $(TARGET)
