CC = gcc
CFLAGS = -O2 -g -Wall -Wvla -Werror -std=gnu11
SANFLAGS = -fsanitize=address,leak
COVFLAGS = -fprofile-arcs -ftest-coverage

TARGET = ymirdb
COVTARGET = ymirdb_cov
SRC = ymirdb.c

all: $(TARGET)

cov: $(COVTARGET)
	./run_test ./$(COVTARGET)
	gcov $(COVTARGET)-$(SRC)

test: $(TARGET)
	./run_test ./$(TARGET)

$(COVTARGET): $(SRC)
	$(CC) $(CFLAGS) $(COVFLAGS) $^ -o $@

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SANFLAGS) $^ -o $@

clean:
	rm -f *.o *.gc* $(TARGET) $(COVTARGET)
