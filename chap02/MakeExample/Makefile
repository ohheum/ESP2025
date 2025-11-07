CC=gcc

CFLAGS = -I. -Wall

PROG = hellomake

OBJS = hellomake.o hellofunc.o

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) -f $(PROG) $(OBJS)
	
install:
	cp $(PROG) $(HOME)/bin
