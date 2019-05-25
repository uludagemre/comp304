Run the following commands:

gcc virtmem.c -o virtmem
./virtmem BACKING_STORE.bin addresses.txt -p 0

(-p 0 for FIFO page replacement,
 -p 1 for LRU page replacement)