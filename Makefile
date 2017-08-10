CFLAGS = -Wall -Wextra -Werror -Wreturn-type -ggdb3 $(shell python3.5-config --cflags)
LIBS = $(shell python3.5-config --libs)

python: pythonmux.c python.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f python

.PHONY: clean
