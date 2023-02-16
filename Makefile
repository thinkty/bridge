.POSIX:    # Parse it an run in POSIX conforming mode
.SUFFIXES: # Delete the default suffixes (inference rules)

CC=gcc
CFLAGS=-g -Wall -I$(IDIR)
LDLIBS=-pthread -lcurses
OUTPUT=bridge
ROOTDIR=.
IDIR=$(ROOTDIR)/include
SDIR=$(ROOTDIR)/src
ODIR=$(ROOTDIR)/obj

_DEPS=tcp.h
DEPS=$(addprefix $(IDIR)/,$(_DEPS))

_OBJS=tcp.o bridge.o
OBJS=$(addprefix $(ODIR)/,$(_OBJS))

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(OBJS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean 
clean:
	rm -rf $(ODIR) $(OUTPUT)

