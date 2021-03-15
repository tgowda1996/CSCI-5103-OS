Project Members:
Tushar Gowda  -  gowda019
Pierce Gruidl -  gruid018

### Async vs Sync 
We have defined a set of macros to control different variables - I/O size, loop size for other threads, file name.
Case 1: Large I/O and large calculation. Loop config = (1000,1000,100), File_name = "spam.csv", READ_WRITE_CONFIGURATIONS=3000
  In this case the Async I/O performed better. 
  Async I/O program ran in 2866ms and Sync I/O ran in 7396ms.


//TODO
