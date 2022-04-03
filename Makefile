CC = gcc
VALGRIND = valgrind -q --leak-check=full
GCOV = gcov -m
RUN_TEST = ./run_test

CFLAGS = -Wall -Wvla -Werror -std=gnu11
COVFLAGS = -g --coverage

TARGET = integerdb
COVTARGET = $(TARGET)_cov
SRC = integerdb.c

all: $(TARGET)

cov: $(COVTARGET)
	$(RUN_TEST) ./$(COVTARGET)
	$(GCOV) $(SRC)

test: $(TARGET)
	$(RUN_TEST) "$(VALGRIND) ./$(TARGET)"

$(COVTARGET): $(SRC)
	$(CC) $(CFLAGS) $(COVFLAGS) $^ -c
	$(CC) $(CFLAGS) $(COVFLAGS) $(^:.c=.o) -o $@

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o *.gc* $(TARGET) $(COVTARGET)
