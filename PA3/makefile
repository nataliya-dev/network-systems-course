CFLAGS  = -g -Wall -pthread
CC = gcc
OBJFILES =  src/utils.o src/parser.o src/connection.o src/webproxy.o
TARGET = webproxy

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
