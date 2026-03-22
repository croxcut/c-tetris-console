# Pixler build :D*

CC      = gcc
TARGET  = pixler.exe
SRC     = pixler.c
HDR     = pixler.h
OBJ     = pixler.o

CFLAGS  = -O2 -Wall -std=c11 -mconsole
LDFLAGS = -lkernel32 -luser32

.PHONY: all clean run clean-saves

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

run: $(TARGET)
	./$(TARGET)

clean:
	del /Q $(OBJ) $(TARGET) 2>nul || true

clean-saves:
	del /Q pixler_save.dat pixler_scores.csv 2>nul || true
