.PHONY: all linux windows clean help font

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -fPIC
LDFLAGS = -lm

LINUX_CFLAGS = $(CFLAGS) $(shell pkg-config --cflags raylib)
LINUX_LDFLAGS = $(LDFLAGS) $(shell pkg-config --libs raylib)

WINDOWS_CC = x86_64-w64-mingw32-gcc
WINDOWS_CFLAGS = $(CFLAGS) -I$(RAYLIB_WINDOWS_PATH)/include
WINDOWS_LDFLAGS = -L$(RAYLIB_WINDOWS_PATH)/lib -lraylib -lgdi32 -lwinmm -lpthread

SRCS = src/main.c src/flashcard.c
OBJS_LINUX = obj/linux/main.o obj/linux/flashcard.o
OBJS_WINDOWS = obj/windows/main.o obj/windows/flashcard.o

TARGET_LINUX = bin/flashcard
TARGET_WINDOWS = bin/flashcard.exe

FONT_URL = https://github.com/notofonts/noto-cjk/raw/main/Sans/OTF/Japanese/NotoSansCJKjp-Regular.otf
FONT_FILE = data/NotoSansCJKjp-Regular.otf

all: linux

font:
	@echo "Downloading Japanese font (Noto Sans CJK JP)..."
	@mkdir -p data
	@if [ ! -f "$(FONT_FILE)" ]; then \
		curl -L -o "$(FONT_FILE)" "$(FONT_URL)" && \
		echo "✓ Font downloaded: $(FONT_FILE)"; \
	else \
		echo "✓ Font already exists: $(FONT_FILE)"; \
	fi

linux: $(TARGET_LINUX)

windows: $(TARGET_WINDOWS)

$(TARGET_LINUX): $(OBJS_LINUX) | bin
	$(CC) -o $@ $^ $(LINUX_LDFLAGS)
	@echo "✓ Linux build complete: $@"

$(TARGET_WINDOWS): $(OBJS_WINDOWS) | bin
	$(WINDOWS_CC) -o $@ $^ $(WINDOWS_LDFLAGS)
	@echo "✓ Windows build complete: $@"

obj/linux/%.o: src/%.c | obj/linux
	$(CC) $(LINUX_CFLAGS) -c $< -o $@

obj/windows/%.o: src/%.c | obj/windows
	$(WINDOWS_CC) $(WINDOWS_CFLAGS) -c $< -o $@

bin obj/linux obj/windows:
	@mkdir -p $@

clean:
	@rm -rf obj bin
	@echo "✓ Cleaned"

help:
	@echo "Flashcard Builder (Raylib Edition)"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build for Linux (default)"
	@echo "  make windows      - Cross-compile for Windows (requires x86_64-w64-mingw32-gcc)"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make help         - Show this message"
	@echo ""
	@echo "For Windows cross-compilation, set RAYLIB_WINDOWS_PATH:"
	@echo "  RAYLIB_WINDOWS_PATH=/path/to/raylib make windows"
