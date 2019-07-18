# nvmlib
Non-Volatile Memory Library Function Definitions as described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)

Designed by Avery Acierno (Undergraduate Research)

Version 3 is an older model where Pools and regular C File I/O operate in parallel

Versions 4 and 5 use Pools to manipulate data sepearate from C File I/O, but also have interfaces to read data from and write data to C files

Version 4 uses char data and txt files

Version 5 uses int* data and binary files

There is no Version 6

Version 7 uses void* data and is compatible with both binary and txt files

Version 8:
1. adds a closed variable to pools instead of freeing all oids in the pool when closed
2. Enables other oids to be stored in data

*Additional Information (Changes, Imporvements, Problems) are located in the comments of the nvmlib<v#>.c files
