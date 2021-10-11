## CSCI-5103-Project-4

### Team members
- Anthony Dierssen-Morice (diers040)
- Tushar Gowda (gowda019)

### fs_format
In order to test the `fs_format` function, the following commands were run:
```
[19:30:13]$ ./simplefs mydisk 50
opened emulated disk image mydisk with 50 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> mount
mount failed!
 simplefs> format
disk formatted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> exit
Not currently mounted
closing emulated disk.
9 disk block reads
7 disk block writes
[19:31:08]$ ./simplefs mydisk 50
opened emulated disk image mydisk with 50 blocks
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> exit
Not currently mounted
closing emulated disk.
7 disk block reads
0 disk block writes
```
The above test shows that a newly created, unformatted disk does not contain any
file system, and, thus, cannot be mounted. After formatting the disk, however, we can
run `debug` to see that a file system does now exist--indicating a successful
format. To show that the disk image persists, we then exit and relaunch the same
disk image to find that `debug` does, in fact, confirm that the disk is still
formatted.

### fs_create
In order to test the `fs_create` function, the following commands were run:
```
[19:39:52]$ ./simplefs mydisk 50
opened emulated disk image mydisk with 50 blocks
 simplefs> format
disk formatted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 0 bytes
    direct blocks:
 simplefs> create
created inode 1
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 0 bytes
    direct blocks:
inode 1:
    size: 0 bytes
    direct blocks:
 simplefs> exit   
closing emulated disk.
30 disk block reads
9 disk block writes
```
In the above test we format a newly created disk, mount the disk, and then
create a new file. The subsequent call to `debug` shows that the file has been
created successfully. For good measure, a second file is created and `debug` is
called again to verify that everything is as expected.

### fs_mount and fs_unmount
In order to test the `fs_mount` and `fs_unmount` functions, the following commands were run:
```
[19:47:10]$ ./simplefs mydisk 50
opened emulated disk image mydisk with 50 blocks
 simplefs> format
disk formatted.
 simplefs> create
Must be mounted to perform create
create failed!
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 0 bytes
    direct blocks:
 simplefs> format
Unmount before formatting
format failed!
 simplefs> unmount
disk unmounted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 0 bytes
    direct blocks:
 simplefs> format
disk formatted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> exit
Not currently mounted
closing emulated disk.
29 disk block reads
15 disk block writes
```
In the above test we first format a newly created disk. With the formatted disk,
we attempt to create a new file. This operation fails because the disk is not
mounted. As such, we mount the disk before proceeding to successfully create a
file, thereby indicating that the disk is actually mounted. Next, we attempt to
format the disk. This action fails because the disk is mounted. As such, we
unmount the disk before attempting to format the disk again. The format then
succeeds indicating the unmount was successful.

### fs_debug
observe the following output obtained from the solution executable:
```
[19:53:57]$ ./simplefs-solution image.200 200
opened emulated disk image image.200 with 200 blocks
 simplefs> debug
superblock:
    200 blocks
    21 inode blocks
    2688 inodes
inode 1:
    size: 1523 bytes
    direct blocks: 152 
inode 2:
    size: 105421 bytes
    direct blocks: 49 50 51 52 53 
    indirect block: 54
    indirect data blocks: 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 
inode 9:
    size: 409305 bytes
    direct blocks: 22 23 24 25 26 
    indirect block: 28
    indirect data blocks: 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 76 77 78 79 80 82 83 84 85 86 
87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 1
19 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 1
48 149 150 151 
 simplefs> exit
Not currently mounted
closing emulated disk.
2691 disk block reads
0 disk block writes
```
This output matches the output from our own implementation of `fs_debug` with
the exception of the number of disk block reads:
```
[19:55:06]$ ./simplefs image.200 200         
opened emulated disk image image.200 with 200 blocks
 simplefs> debug
superblock:
    200 blocks
    21 inode blocks
    2688 inodes
inode 1:
    size: 1523 bytes
    direct blocks: 152
inode 2:
    size: 105421 bytes
    direct blocks: 49 50 51 52 53
    indirect block: 54
    indirect data blocks: 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75
inode 9:
    size: 409305 bytes
    direct blocks: 22 23 24 25 26
    indirect block: 28
    indirect data blocks: 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 76 77 78 79 80 82 83 84 85 86 
87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 1
19 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 1
48 149 150 151
 simplefs> exit
Not currently mounted
closing emulated disk.
24 disk block reads
0 disk block writes
```

### fs_delete
In order to test the `fs_delete` function, the following commands were run:
```
[17:56:41]$ ./simplefs mydisk 50 0
opened emulated disk image mydisk with 50 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> create
created inode 0
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 0 bytes
    direct blocks:
 simplefs> copyin test.file
use: copyin <filename> <inumber>
 simplefs> copyin test.file 0
19 bytes copied
copied file test.file to inode 0
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 19 bytes
    direct blocks: 7
 simplefs> delete 0
inode 0 deleted.
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
 simplefs> exit
closing emulated disk.
42 disk block reads
12 disk block writes
```
In the above test we first format a newly created disk. We then mount the disk before proceeding to create an empty file. Next, we add data to that file using copyin. A call to debug shows that the file exists on the disk and contains data. Finally, we delete the file which calls `fs_delete`. A final call to debug shows that the file no longer exists indicating that the delete was successful.

### fs_getsize
In order to test the `fs_getsize` function, the following commands were run:
```
[18:04:03]$ ./simplefs mydisk 50 0
opened emulated disk image mydisk with 50 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> copyin test.file 0
19 bytes copied
copied file test.file to inode 0
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 19 bytes
    direct blocks: 7
 simplefs> getsize 0
inode 0 has size 19
 simplefs> create
created inode 1
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 19 bytes
    direct blocks: 7
inode 1:
    size: 0 bytes
    direct blocks:
 simplefs> getsize 1
inode 1 has size 0
 simplefs> exit
closing emulated disk.
29 disk block reads
12 disk block writes
```
In the above test we first format a newly created disk. We then mount the disk before proceeding to create an empty file. Next, we add data to that file using copyin. A call to debug shows that the file exists on the disk and has a size of 19 bytes of data. Next we call getsize which, as expected, matches the output of the debug call. The same sequence of commands was run in the solution executable which confirmed that 19 bytes was the correct size of the file.

### fs_read and fs_write
In order to test the `fs_read` function, the following commands were run:
```
[anthony@ <<18:13:34>> ~/.../Project-4/csci-5103-project-4]$ ./simplefs mydisk 50 0
opened emulated disk image mydisk with 50 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> copyin test.file 0
19 bytes copied
copied file test.file to inode 0
 simplefs> cat 0
here is some stuff
19 bytes copied
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 19 bytes
    direct blocks: 7
 simplefs> copyout 0 test-verify.file
19 bytes copied
copied inode 0 to file test-verify.file
 simplefs> debug
superblock:
    50 blocks
    6 inode blocks
    768 inodes
inode 0:
    size: 19 bytes
    direct blocks: 7
 simplefs> exit
closing emulated disk.
37 disk block reads
12 disk block writes
```
In the above test we first format a newly created disk. We then mount the disk before proceeding to create an empty file. Next, we add data to that file using copyin. A call to debug shows that the file exists on the disk and has a size of 19 bytes of data which is indicative of the correct functioning of `fs_write`. We then `cat` the file to `stdout` and see the expected line of text `here is some stuff`. A call to debug shows that `cat` which calls `fs_read` had no effect on the disk image. Finally, we write the file to an external file called `test-verify.file`. A call to diff on the original `test.file` and `test-verify.file` showed that there were no differences between the two indicating the proper function of `fs_read`.

To further test `fs_read` and `fs_write` on a much larger file, the following commands were run:
```
[19:34:50]$ ./simplefs mydisk 700 0
opened emulated disk image mydisk with 700 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> copyin image.200 0
819200 bytes copied
copied file image.200 to inode 0
 simplefs> copyout 0 image-verify.200
819200 bytes copied
copied inode 0 to file image-verify.200
 simplefs> exit
closing emulated disk.
1281 disk block reads
718 disk block writes
[19:35:52]$ diff ./image.200 ./image-verify.200
[19:36:12]$
```
In the above test we first format a newly created disk. We then mount the disk before proceeding to create an empty file. To really test the correctness of fs_read and fs_write, a large file 819200 bytes--`image.200` to be exact--is copied into the disk and then copied out to a file called `image-verify.200`. A call to diff on the original `image.200` and `image-verify.200` showed that there were no differences between the two files. 

### overflow test
In the test that follows, a small disk is created of only 10 blocks. After formatting, then, that means that 1 block will be dedicated to the super block, and 2 blocks (20% of disk) will be dedicated to inodes. We then attempt to copyin a file that exceeds the disk size. The first failed attempt shows that only valid, already created files (inodes) can be used as an argument to `copyin`. After creating an empty file, then, the copyin is reattempted. As the output below shows, the copyin only manages to copy data until the disk is full. The final debug shows that of the 7 blocks which had not been allocated at the beginning, have now all been allocated to file `0`.
```
[19:42:55]$ ./simplefs mydisk 10 0
opened emulated disk image mydisk with 10 blocks
 simplefs> debug
superblock:
    magic number is invalid
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> copyin image.200 0
fs_create not called for inumber 0
WARNING: fs_write only wrote 0 bytes, not 16384 bytes
0 bytes copied
copied file image.200 to inode 0
 simplefs> fs_create
unknown command: fs_create
type 'help' for a list of commands.
 simplefs> create
created inode 0
 simplefs> copyin image.200 0
Out of memory
WARNING: fs_write only wrote 8192 bytes, not 16384 bytes
24576 bytes copied
copied file image.200 to inode 0
 simplefs> debug
superblock:
    10 blocks
    2 inode blocks
    256 inodes
inode 0:
    size: 24576 bytes
    direct blocks: 3 4 5 6 7
    indirect block: 8
    indirect data blocks: 9
 simplefs> exit
closing emulated disk.
20 disk block reads
19 disk block writes
```

### Attempted copyin to a full disk
In the following test, a new disk is formatted and filled up with a single file. After an unmount and a remount, a new, empty file is created. We then attempt to copyin data to that empty file. Since the disk is already full, 0 bytes are copied into memory as is expected.
```
[20:42:01]$ ./simplefs mydisk 10 0
opened emulated disk image mydisk with 10 blocks
 simplefs> format
disk formatted.
 simplefs> mount
disk mounted.
 simplefs> create
created inode 0
 simplefs> copyin image.200 0
Out of memory
WARNING: fs_write only wrote 8192 bytes, not 16384 bytes
24576 bytes copied
copied file image.200 to inode 0
 simplefs> unmount
disk unmounted.
 simplefs> mount
disk mounted.
 simplefs> debug
superblock:
    10 blocks
    2 inode blocks
    256 inodes
inode 0:
    size: 24576 bytes
    direct blocks: 3 4 5 6 7
    indirect block: 8
    indirect data blocks: 9
 simplefs> create
created inode 1
 simplefs> copyin test.file 1
Out of memory
WARNING: fs_write only wrote 0 bytes, not 19 bytes
0 bytes copied
copied file test.file to inode 1
 simplefs> exit
closing emulated disk.
25 disk block reads
20 disk block writes
```

### formatting a disk of size 0
The output of this test case matches the behavior of the solution executable.
```
[20:56:31]$ ./simplefs mydisk 0 0
opened emulated disk image mydisk with 0 blocks
 simplefs> mount
ERROR: blocknum (0) is too big!
zsh: abort (core dumped)  ./simplefs mydisk 0 0
[20:56:44]$ ./simplefs-solution mydisk 0
opened emulated disk image mydisk with 0 blocks
 simplefs> mount
ERROR: blocknum (0) is too big!
zsh: abort (core dumped)  ./simplefs-solution mydisk 0
```

## Extra Credit

#### Brief description
We have implemented an LRU cache to cache the inodes and are using the write-back policy while writing dirty inodes back to disk. We have implemented the LRU cache using a linked list. We search for a given inode using linear search and place the recently accessed inode at the front of the list. Whenever there is a need for eviction, we remove a node from the back of the list. This strategy simulates LRU. 

#### Time Complexity
It has O(N) time complexity for get and remove(search and remove),  and O(1) for insert and cache eviction. This might not look very impressive as all cache operations are usually of O(1) order. But, since our cache is in the main memory, it gives a huge boost to speed. 

#### Write back
We maintain a record of all the dirty nodes and whenever they are evicted from the cache, we write the inode back to disk.

#### Design
We have tried to emulate classes in C by creating function pointers inside structures. In order to create constructors and destructors, we have created another structure and an extern const object of the same structure with the name - “Cache”. This structure has two function pointers - new and destroy. Using these member functions we create new objects of cache. The definitions are available in cache.c.

#### Performance Comparison
To test our cache we have written a shell script. This script first uses a python script to generate files and commands. Then the generated commands are fed to the simplefs executable with caching disabled, cache size = 5, cache size = 10, and cache size = 12. The disk reads and writes are grepped from the output of the executable and put in the out_file. To run the script run the following command - ./cache_testing.sh

#### Command generation process:
+ We have three types of files that we feed to our file system - small (<4KB), medium (4KB-8KB), large(8KB-16KB). We create these files in a 6:2:2 ratio. A total of 10 files are generated.
+ In our file system, we create 10 inodes and write in the 10 files generated in the previous step using copyin
+ In order to simulate file access, we ‘cat’ random files 50 times.
+ In order to simulate data exchange between files we select 2 random files, write their data in temp files and use the copyin command to copy the required temp file. This operation is done 30 times.

```
1004 disk block reads
485 disk block writes

Cache size - 5
642 disk block reads
346 disk block writes

Cache size - 10
534 disk block reads
295 disk block writes

Cache size - 12
534 disk block reads
295 disk block writes
```
As can be seen from the output, the number of disk writes and reads decreases as we increase the cache size. There is a huge performance gain between cache size 10 and cache size 0. The number of reads and writes with cache size  = 10 are almost half the number of reads and writes with cache size = 0. As expected, there is no difference in performance between reads and writes for cache sizes of 10+ since all ten of the test file inodes are stored in the cache.
