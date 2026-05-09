# Sistem Programlama 2025-2026 Bahar Donemi Projesi
# tarsau - sikistirma yapmayan basit arsivleyici

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  := tarsau
SRC     := tarsau.c

.PHONY: all clean test demo

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Hizli demo: ornek metin dosyalarini arsivleyip ac
demo: $(TARGET)
	@echo "==> Arsivleme"
	./$(TARGET) -b test_files/t1.txt test_files/t2.txt test_files/t3.txt -o demo.sau
	@echo
	@echo "==> Arsiv icerigi (ilk 200 bayt):"
	@head -c 200 demo.sau; echo
	@echo
	@echo "==> Acma"
	./$(TARGET) -a demo.sau cikti
	@echo
	@echo "==> cikti/ icerigi:"
	@ls -l cikti
	@echo
	@echo "==> Ilk dosyanin icerigi:"
	@cat cikti/t1.txt

# Otomatik testler
test: $(TARGET)
	bash run_tests.sh

clean:
	rm -f $(TARGET) demo.sau a.sau
	rm -rf cikti test_out test_out2
	rm -f test_files/binary.bin
