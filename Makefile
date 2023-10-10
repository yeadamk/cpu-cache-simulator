EXEC = cache

all: ${EXEC}

%: %.c
	gcc -Wall -g -o $@ $^

clear: clean
clean:
	rm -rf ${EXEC} *~
	@ls
