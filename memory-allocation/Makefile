CFLAGS = -c -Wall
CFLAGS+= -g
LDFLAGS = 

SOURCES= mem_alloc.c mem_test.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=malloc_aditya

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *~ 
