CFLAGS = -Wall -Wextra -Werror -Wreturn-type -ggdb3

python: pythonmux.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f python

.PHONY: clean
