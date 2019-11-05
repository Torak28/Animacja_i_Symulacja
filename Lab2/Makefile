SHELL := /bin/bash
TARGETS = $(patsubst %.c, %, $(wildcard main*.c))

all: $(TARGETS)

%: %.c
	g++ gl3w.c $< -o $@ -lGL -lglfw -ldl -I./include

clean:
	rm -f $(TARGETS)
