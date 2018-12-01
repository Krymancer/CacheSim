# CacheSim
A simple C++ code to simulate the MMU translate a virtual address to a real and search in the cache for the real address, using l1 and l2 tables

## Usage

> ./cachesim path/to/l1table path/to/l2table path/to/address

Where l1table and l2table are the files are contains the L1 and L2 lines, and the address is the file that contains the address of are requested by CPU

## Compile

just:
> make


