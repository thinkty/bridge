CC=gcc
CFLAGS=-g -Wall -I$(IDIR)
ROOTDIR=.
IDIR=$(ROOTDIR)/include
SDIR=$(ROOTDIR)/src
ODIR=$(ROOTDIR)/obj
LIBS=-pthread

_DEPS = helper.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = client.o server.o helper.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: server client

server: $(ODIR)/helper.o $(ODIR)/server.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: $(ODIR)/helper.o $(ODIR)/client.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o client server

