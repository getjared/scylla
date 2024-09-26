CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lcurl -ljson-c

TARGET = scylla
SRC = scylla.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean