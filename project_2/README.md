Project Members:
Tushar Gowda  -  gowda019
Pierce Gruidl -  gruid018


## Test cases

##### Lock and condition variable tests
Command to be used: make test_public_methods
./test_public_methods <test_number>

1. Testing condVariable wait and signal:
In this test, we created 3 threads. One was made to wait, one to signal and the other to acquire the lock. Threads were created in the same order. Below is the output for the test.
```
Threads Created
Joining on - 1
Thread 1 waiting
Starting thread 2
Thread 2 working
got a signal. Thread 1 starts working
Thread 2 work after signal
Thread 3 working
Joining on - 2
Joining on - 3
```

We can see that test thread 1 waits for the signal and thread 2 starts executing after that. This tells that the lock was released by thread 1. Thread after working signals thread 1, which acquires the lock again. Since we follow Hoare semantics, its lock is given back to thread 2 and the statement after signal is executed. Thread 3 acquires the lock after this as expected.

2. Testing condVariable wait and broadcast:
In this test, we created 10 threads. 8 were made to wait, one to broadcast and the other to acquire the lock. Threads were created in the same order. Below is the output for the test.
```
Threads Created
Joining on - 1
Thread 1 waiting
Thread 2 waiting
Thread 3 waiting
Thread 4 waiting
Thread 5 waiting
Thread 6 waiting
Thread 7 waiting
Thread 8 waiting
Starting thread 9
Thread 9 working
got a signal. Thread 1 starts working
got a signal. Thread 2 starts working
got a signal. Thread 3 starts working
got a signal. Thread 4 starts working
got a signal. Thread 5 starts working
got a signal. Thread 6 starts working
got a signal. Thread 7 starts working
got a signal. Thread 8 starts working
Thread 9 work after signal
Thread 10 working
Joining on - 2
Joining on - 3
Joining on - 4
Joining on - 5
Joining on - 6
Joining on - 7
Joining on - 8
Joining on - 9
Joining on - 10
```
It can be seen that all the 8 threads wait on the condVar and are woken up in order as required by Hoare semantics. The lock is returned to thread 9 which does further work. After that thread 10 takes the lock and proceeds with its work. If Hoare semantics wasn't working thread 10 might have started working earlier when following a FIFO based round-robin ready queue.

3. Testing lock and unlock:
We run 5 threads, each trying to access the same critical section. Thread 1 starts working and is preempted. Control is given to thread 2 and then 3 and so on. In all these, we can see that the critical section for 2,3,4,5 doesn't start before thread 1 exits the critical section. This pattern is followed for other threads as well. The start of the critical section is denoted by a "start working" and the end is denoted by an "end working". Since there is no interleave between these statements from different threads, we can claim that our lock is working. Output:
Threads Created
```
Joining on - 1
Starting thread 1
Thread 1 starts working
Starting thread 2
Starting thread 3
Starting thread 4
Starting thread 5
Thread 1 finished working
Thread 2 starts working
Joining on - 2
Thread 2 finished working
Thread 3 starts working
Joining on - 3
Thread 3 finished working
Thread 4 starts working
Joining on - 4
Thread 4 finished working
Thread 5 starts working
Joining on - 5
Thread 5 finished working
```

4. Testing spinLock lock and unlock:
Spinlock executes the same type of function. The only difference being the usage of spinlocks instead of standard locks. The output produced is as follows:
```
Threads Created
Joining on - 1
Starting thread 1
Thread 1 starts working
Starting thread 2
Starting thread 3
Starting thread 4
Starting thread 5
Thread 1 finished working
Thread 2 starts working
Joining on - 2
Thread 2 finished working
Thread 3 starts working
Joining on - 3
Thread 3 finished working
Thread 4 starts working
Joining on - 4
Thread 4 finished working
Thread 5 starts working
Joining on - 5
Thread 5 finished working
```
##### Async Read and Write
1. Testing async read: In this test, we have 2 threads. The first thread does an async read, followed by an async write. We test 2 properties of the read functionality: 
	* Is it able to read successfully? - A file "to_read" is read and is written to the file "to_write". We can see that the contents of both files match.
	* Is it working asynchronously? - For this, we kept the time quanta high to make sure that a thread is not preempted between completion of I/O and printing of the "Done" msg. We can see the following output:
	```
	Threads Created
	Joining on - 1
	Starting thread : 1
	Starting thread : 2
	Done with read and write
	Joining on - 2
	Exiting
	```
	This output tells us that thread 2 was started before thread 1 finished reading and writing and tells us that our async read is working as expected.

2. Testing async write: In this test, we have 2 threads. The first thread does a async write and the second one is a worker thread doing some job(polling in this case). We test 2 properties of the write functionality: 
	* Is it able to write successfully? - We write the statement "test" to the file - "to_write_async_call". We can see that the file was successfully created and the required line is present.
	* Is it working asynchronously? - For this, we kept the time quanta high to make sure that a thread is not preempted between completion of I/O and printing of the "Done" msg. We can see the following output:
	```
	Threads Created
	Joining on - 1
	Starting thread : 1
	Starting thread : 2
	Done with write
	Joining on - 2
	Exiting
	```
	This output tells us that thread 2 was started before thread 1 finished writing and tells us that our async write is working as expected.

## Priority Inversion
In order to implement priority inversion, we have used the suggested algorithm of random boosting. The problem of priority inversion occurs when a low priority thread holds a lock that a high priority thread is waiting on. 
To solve this problem we do the following things
* use three main data structures. A map to store the number of times a thread was boosted, 2 unordered sets containing the tcbs of priority 1 and priority 2 respectively. TCBs from set 1 is boosted to priority ORANGE and tcbs from set 2 are boosted to priority RED. 
* Whenever a thread yields, we check:
	* If the thread is holding any lock, depending on its priority, we put it in the appropriate set. Before adding we even check the size of the higher priority ready lists and don’t put it in the set if there are no higher priority jobs present in the ready queues.
	* If the thread is not holding a lock, we check if it was ever boosted and decrease its priority.
* Every time a yield operation is performed, either during pre-emption or a manual yield, we boost 5% or 1 whichever is higher from the medium priority threads and 10% or 1 (whichever is higher) from the low priority threads.

To test this logic we use a scenario where there are 6 threads in total. 1 low priority thread holding a lock, 1 high priority thread waiting on the lock, 2 hp worker threads, and 2 med priority worker threads. We create the ap thread first, let it get the lock, yield, and then in the main thread create the other threads. 
Ideally, this is the perfect scenario for priority inversion as according to our ready queue scheduling strategy the hp thread will be the last one to finish and thus have its priority inverted. 
Our code is able to handle this situation as demonstrated in the output below:
------------------------
Usage command:
make priority_inversion
./priority_inversion 
------------------------
Output

```
Acquiring lock for 1
Threads Created
Joining on - 1
Acquiring lock for 2
Starting thread : 3
Starting thread : 4
Out of lock
Exiting thread 2
Exiting thread 4
Exiting thread 3
Starting thread : 5
Starting thread : 6
Exiting thread 5
Exiting thread 6
Exiting thread 1
Joining on - 2
Joining on - 3
Joining on - 4
Joining on - 5
Joining on - 6
Exiting
```

As we can see from the output thread 1 acquired lock and thread 2 tries to acquire the lock. Hp threads 3 and 4 start executing. Meanwhile, the priority of thread 1 is boosted. It releases the lock and thread 2, which has a small critical section, is the first hp thread to exit. Thread 1 on the other hand is the last one to exit as we were able to drop its priority successfully.



## Async vs Sync

---------------
Usage Commands
make async_vs_sync
./async_vs_sync <type> (1 for sync and 2 for async)
Other parameters need to be tuned by changing the macros.
---------------

We have defined a set of macros to control different variables - I/O size, loop size for other threads, file name.

Case 1: Large I/O and large calculation. Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_OPERATIONS=3000.

	In this case, the Async I/O performed better. 
	Async I/O program ran in 2866ms and Sync I/O ran in 7396ms  
	We can see that async I/O provides better performance. This is because it lets the other threads proceed in their work when the I/O thread is busy. 

Case 2: Constant calculation, Varying readwrite.

	* Loop config = (1000,1000,1), File_name = "spam.csv", READ_WRITE_OPERATIONS=1
	Sync = 741ms, Async = 735ms.
	
	* Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_OPERATIONS=100
	Sync = 1062ms, Async = 1065ms.

	* Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_OPERATIONS=1000
	Sync = 4811ms, Async = 1823ms.

	* Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_OPERATIONS=3000
	Sync = 7396ms, Async = 2866ms.

	We can see that as we increase the amount of I/O, both sync and async take more time. But after a particular threshold  of I/O operations, the ratio between the time taken by sync and async is similar. This happens because the worker threads finish working and the only thread left is the i/o thread and bot sync and async take similar amounts of time when run on a single thread. At least when seen in a millisecond time scale.

Case 3: Varying calculation, Fixed readwrite.
	* Loop config = (1000,100,1), File_name = "spam.csv", READ_WRITE_OPERATIONS=500
	Sync = 1850ms, Async = 646ms.

	* Loop config = (100,100,1), File_name = "spam.csv", READ_WRITE_OPERATIONS=500
	Sync = 1898ms, Async = 571ms.

	* Loop config = (100,10,1), File_name = "spam.csv", READ_WRITE_OPERATIONS=500
	Sync = 1898ms, Async = 571ms.

	* Loop config = (1000,500,1000), File_name = "spam.csv", READ_WRITE_OPERATIONS=500
	Sync = 5343ms, Async = 3936ms.

	* Loop config = (1000,1000,1000), File_name = "spam.csv", READ_WRITE_OPERATIONS=500
	Sync = 8695ms, Async = 7264ms.

	We can see that as we increase the amount of work the absolute difference starts to stagnate. This is because the i/o thread finishes executing and in both cases the rest of the threads take a similar amount of time.


## Standard lock vs spin lock
* Command to run the test: 
make spin_vs_normal 
./spin_vs_normal
This will run the profiling script which changes the values for multiple parameters. The one with 50 threads was chosen. quanta value needs to be set manually. Currently set to 10000.

* During the initial testing when the critical section was small, spinlock performed better. This is because, for small critical sections, the overhead of context switching in a standard lock is more than the wait time in a spinlock.
* Comparing lock performance with varying critical sections:
Below are the experiment results obtained with 50 threads with varying loads. X-axis is the time taken and Y-axis is the load.

![Alt text](spin_vs_normal_50threads_init100.png?raw=true "spin vs normal - varying workload")

As we can see the spinlocks work better than the normal lock for smaller critical sections but it starts performing poorly as the critical section size increases. This is because as the size of the critical section increases the threads that are waiting for the lock will spin more times as opposed to the case of a smaller critical section where the thread might spin only once.

* In a multicore system, the threshold where standard locks start becoming better than spin locks will decrease. But the overall trend will be similar.

* We could see that the thread library quanta has an effect on the performance of spinlocks. As we increase the thread library quanta, spinlocks outperform standard locks at critical section sizes where it was worse than the standard lock at smaller quantas.

![Alt text](spin_vs_normal_50threads_init10000.png?raw=true "spin vs normal - varying workload- 10000 quanta")
