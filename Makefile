CC = gcc
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SOURCES = ./src/*.c ./src/*.h
TARGET = ./build/game
all:
	$(CC) $(SOURCES) -o $(TARGET) $(LIBS)
run:
	rm -f $(TARGET)
	$(CC) $(SOURCES) -o $(TARGET) $(LIBS)
	$(TARGET)
clean:
	rm -rf ./build/game
.PHONY: all clean