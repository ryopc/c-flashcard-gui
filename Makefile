CC ?= gcc
CFLAGS ?= -O2 -Wall
PKG_CONFIG ?= pkg-config
PREFIX ?= /usr/local

GTK_CFLAGS = $(shell $(PKG_CONFIG) --cflags gtk+-3.0)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk+-3.0)

all: flashcard

flashcard: main.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) main.c -o flashcard $(GTK_LIBS)

install: flashcard
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 flashcard $(DESTDIR)$(PREFIX)/bin/flashcard
	install -d $(DESTDIR)$(PREFIX)/share/applications
	install -m 644 com.ryopc.Flashcard.desktop $(DESTDIR)$(PREFIX)/share/applications/com.ryopc.Flashcard.desktop
	install -d $(DESTDIR)$(PREFIX)/share/metainfo
	install -m 644 com.ryopc.Flashcard.appdata.xml $(DESTDIR)$(PREFIX)/share/metainfo/com.ryopc.Flashcard.appdata.xml

clean:
	rm -f flashcard

.PHONY: all install clean
