CC = g++
CFLAGS = -g -lrt --std=c++14
DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h async_io.h
OBJ = TCB.o uthread.o Lock.o CondVar.o SpinLock.o async_io.o
MAIN_OBJ = main.o
TEST_PROFILE_1 = async_vs_sync.o
TEST_PRIORITY_INVERSION = priority_inversion_test.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

uthread-sync-demo: $(OBJ) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

async_vs_sync: $(OBJ) $(TEST_PROFILE_1)
	$(CC) -o $@ $^ $(CFLAGS)

priority_inversion: $(OBJ) $(TEST_PRIORITY_INVERSION)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o uthread-sync-demo
