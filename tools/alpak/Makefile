OUTPUT=alpak
CFLAGS=-Wall -g3
LDFLAGS=
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
HEADERS=$(wildcard *.h)

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $(OUTPUT) $(OBJECTS) $(LDFLAGS)

*.o: *.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<
	
.PHONY: clean
clean:
	$(RM) $(OUTPUT) $(OBJECTS)
