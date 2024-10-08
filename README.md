Cache Simulator:

The program is a cache simulator designed to mimic the cache and to check the number of loads and stores including hits and misses. The input is took from the standard input which is given from a trace file and all the configurations are given when the executable is run.

The command for running the executable is :
./cacheSim n_sets n_blocks n_bytes write-allocate write-back fifo < fie.trace

we can configure the above configurations as required and the standard output contains the total number of hits,misses and number of load hits,load misses,store hits,store misses and total number of clock cycles.

CLOCK CYCLES:
The total number of clock cycles is the design choice of the cache. for this particular implementation when a load hit occurs since we are loading from cache it takes one clock cycle,and when a load miss occurs we load from the memory so total cycles is number of bytes transfered times the clock cycles taken for each byte. since each 4 byte quantity takes 100 clock cycles each word takes 25 clock cycles and loading from cache takes one clock cycle. so when a load miss occurs the clock cycles are 25 times number of bytes plus one.

for store hits and store misses, if a store hit occurs we are writing into the cache and depending on write back or write allocate we write into memery. if it is write back we increase the clock cycles only for writing into cache which is 1 and if it is write through we write the particular 4 byte word into memory(instead of writing the whole block) so it takes 100 clock cycles. if it is a store miss then depending on write allocate or no write allocate we increase the clock cycles. if it is write allocate then writing into memory takes 100 clock cycles for each 4 byte word and since we are loading the whole block it takes 25*n_bytes. and then depending on write back or write allocate then we increase the clock cycles accrodingly. for write back 1 clock cycle and for write through 100 clock cycles for 4 bytes.if it is no write allocate then since we write into memory it takes 100 clock cycles only since we write only one word.
