.POSIX:    # Parse it an run in POSIX conforming mode
.SUFFIXES: # Delete the default suffixes (inference rules)
.PHONY: all debug clean

CC=gcc
CFLAGS=-g -Wall -Werror -I$(IDIR)
LDLIBS=-pthread -lcurses
OUTPUT=bridge
ROOTDIR=.
IDIR=$(ROOTDIR)/include
SDIR=$(ROOTDIR)/src
ODIR=$(ROOTDIR)/obj

_DEPS=tcp.h server.h tui.h util.h
DEPS=$(addprefix $(IDIR)/,$(_DEPS))

_OBJS=tcp.o server.o main.o tui.o
OBJS=$(addprefix $(ODIR)/,$(_OBJS))

all: $(OUTPUT)

debug: CFLAGS += -DDEBUG
debug: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(ODIR) $(OUTPUT)
