.PHONY: clean uninstall install
.SUFFIXES: .c .cc .o

CC = gcc
CXX = g++
LD = g++

CFLAGS = -Wall --std=c99 -g
CXXFLAGS = -Wall --std=c++98 -g
LDLIBS = `fox-config --libs`

PREFIX=/usr/local

OBJ = fxdabout.o res/foxres.o

ICONS = res/*.gif
BIN = fxdabout

CXXFLAGS += `fox-config --cflags`

all: $(BIN)

$(OBJ): res/foxres.h res/foxres.cc

res/foxres.h: $(ICONS)
	reswrap -i -d -p res_ -o res/foxres.h $(ICONS)

res/foxres.cc: res/foxres.h $(ICONS)
	reswrap -e -d -p res_ -o res/foxres.cc $(ICONS)


fxdabout.o: fxdabout.cc

all: $(BIN)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)
	rm -f res/foxres.h res/foxres.cc

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	strip $(DESTDIR)$(PREFIX)/bin/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

$(BIN): $(OBJ)
	$(LD) -o$(BIN) $(LDFLAGS) $(OBJ) $(LDLIBS)


.c.o:
	$(CC) $(CFLAGS) -c -o$@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o$@ $<
