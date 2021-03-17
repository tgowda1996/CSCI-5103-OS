Project Members:
Tushar Gowda  -  gowda019
Pierce Gruidl -  gruid018

### Async vs Sync 
We have defined a set of macros to control different variables - I/O size, loop size for other threads, file name.
Case 1: Large I/O and large calculation. Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_CONFIGURATIONS=3000
  In this case the Async I/O performed better. 
  Async I/O program ran in 2866ms and Sync I/O ran in 7396ms.


### Standard lock vs spin lock
* During the initial testing when the critical section was small, spin lock performed better. This is because, for small critical sections, the overhead of context switching in a standard lock is more than the wait time in a spin lock.
* Comparing lock performace with varying critical sections:
Below is the experiment results obtained with 50 threads with varying loads. X axis is the time taken and y axis is the load.

![Alt text](spin_vs_normal_50threads_init100.png?raw=true "spin vs normal - varying workload")

As we can see the spin locks work better than the normal lock for smaller critical sections but it starts performing real bad as the critical section size increases. This is because as the size of critical section increases the threads that are waiting for the lock will spin more number of times as opposed to the case of smaller critical section where the thread might spin only once.

* In a multicore system, the threashold where standard locks start becoming better than spin locks will decrease. But the overall trend will be similar.

* We could see that the thread library quanta has an affect on the performance of spin locks. As we increase the thread library quanta, spinlocks outperform standard locks at critical section sizes where it was worse than the standard lock at smaller quantas.

![Alt text](spin_vs_normal_50threads_init10000.png?raw=true "spin vs normal - varying workload- 10000 quanta")