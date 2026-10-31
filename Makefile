# Makefile fuer CFGEDIT.PRG
# Cross-Compiler: m68k-atari-mint-gcc

CC      = m68k-atari-mint-gcc
CFLAGS  = -O2 -Wall -Wextra
LDFLAGS = -s

TARGET  = CFGEDIT.PRG
SRC     = src/cfgedit.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
