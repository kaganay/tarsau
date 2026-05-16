# Sistem Programlama 2025-2026 Bahar Donemi Projesi
# tarsau - sikistirma yapmayan basit arsivleyici

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  := tarsau
SRC     := tarsau.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.sau
	rm -rf cikti
