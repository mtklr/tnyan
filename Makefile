# tnyan Makefile

PROG = tnyan

CFLAGS += -Os #-O0 -g -Wall
LDLIBS = -lncurses

all: $(PROG)

$(PROG): $(PROG).o

clean:
	$(RM) $(PROG)
	$(RM) *.o

.PHONY: all clean
