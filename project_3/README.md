## CSCI-5103-Project-3

#### Team members
- Anthony Dierssen-Morice (diers040)
- Tushar Gowda (gowda019)

### Running individual tests
```
> make
> ./virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>
```

### Generating performance graphs
```
> ./generate_graphs.sh
> python3 compare_policies_generate_graph.py
```

See `report.pdf` for descriptions of the tests and analysis.

#### Intermediate Submission Tests
In order to test our page replacement policy, we used the given test programs. We used two types of input
- npages > nframes
- npages <= nframes

Program name - focus:
  - npages = 15, nframes = 10 (npages>nframes) : In this program we got the following output:
     ```
     Focus Successful: Result = -1528
     60 page faults, 60 disk reads, 47 disk writes. 
     ```
     
     This is expected as the number of pages is more than the number of frames available in physical memory.
     
  - npages = 8, nframes = 10 (npages <  nframes) : In this program we got the following output:
     ```
     Focus Successful: Result = -2217
     8 page faults, 8 disk reads, 0 disk writes.
     ```

Program name - scan:
  - npages = 15, nframes = 10 (npages>nframes) : In this program we got the following output:
     ```
     Scan Successful: Result = 78336000
     93 page faults, 93 disk reads, 15 disk writes.
     ```
     This is expected as the number of pages is more than the number of frames available in physical memory.
     
  - npages = 10, nframes = 15 (npages <  nframes) : In this program we got the following output:
     ```
     Scan Successful: Result = 52224000
     10 page faults, 10 disk reads, 0 disk writes.
     ```
Program name - sort:
  - npages = 15, nframes = 10 (npages>nframes) : In this program we got the following output:
     ```
     Sort Successful: Result = -85216
     54 page faults, 54 disk reads, 37 disk writes. 
     ```
     This is expected as the number of pages is more than the number of frames available in physical memory.
     
  - npages = 10, nframes = 15 (npages <  nframes) : In this program we got the following output:
     ```
     Sort Successful: Result = -51645
     10 page faults, 10 disk reads, 0 disk writes.
     ```
