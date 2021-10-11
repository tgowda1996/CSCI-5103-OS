GCC=/usr/bin/gcc

simplefs: shell.o cache.o fs.o disk.o
	$(GCC) shell.o cache.o fs.o disk.o -o simplefs

shell.o: shell.c
	$(GCC) -Wall shell.c -c -o shell.o -g

fs.o: fs.c fs.h
	$(GCC) -Wall fs.c -c -o fs.o -g

disk.o: disk.c disk.h
	$(GCC) -Wall disk.c -c -o disk.o -g

cache.o: cache.c cache.h
	$(GCC) -Wall cache.c -c -o cache.o -g

clean:
	rm simplefs disk.o fs.o shell.o cache.o
