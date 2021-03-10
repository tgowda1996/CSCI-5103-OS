CC = g++
CFLAGS = -g -D_XOPEN_SOURCE=600

demo: ucontext-demo.cpp
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f demo
