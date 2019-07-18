//Testing Program for Non-Volatile Memory Library Function Definitions Version 8
//This Test Attempts to store oids in void* data of other oids
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

#include "nvmlib8.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	printf("Creating pool1 of size 10...\n\n");
	pool* pool1 = pool_create("pool1", 10);
	
	printf("Writing 1,2,3,4,5 to every other oid in pool1...\n\n");
	int i;
	for (i = 1; i <= 10; i++)
	{
		pwriteint(pool1, i);
	}

	printf("Contents of pool1:\n");
	preadf(pool1);
	printf("\n");

	printf("Allocating 5 more OIDs to pool1...\n");
	pmalloc(pool1, 5);
	printf("New size of pool1: %d\n\n", pool1->size);

	printf("Writing oids containing odd numbers (even offsets) to oids at offstets 10-14...\n\n");
	for (i = 0; i < 10; i = i + 2)
	{
		pwriteptr(pool1, getoid(pool1, i));
	}

	printf("Contents of pool1:\n");
	preadf(pool1);
	printf("\n");

	printf("Writing pool1 to file oddoids.bin...\n");
	pfileout(pool1, "oddoids.bin");
	printf("\n");

	printf("Writing pool1 to file oddoids.txt...\n");
	pfileouttxt(pool1, "oddoids.txt");
	printf("\n");

	printf("Creating pool2 of size 5...\n\n");
	pool* pool2 = pool_create("pool1", 5);
	
	printf("Writing oids containing even numbers (odd offsets) to oids at offstets 0-4...\n\n");
	for (i = 1; i < 10; i = i + 2)
	{
		pwriteptr(pool2, getoid(pool1, i));
	}

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Writing pool1 to file evenoids.bin...\n");
	pfileout(pool2, "evenoids.bin");
	printf("\n");

	printf("Writing pool1 to file evenoids.txt...\n");
	pfileouttxt(pool2, "evenoids.txt");
	printf("\n");

	printf("Closing pool1...\n\n");
	pool_close(pool1);

	printf("Closing pool2...\n\n");
	pool_close(pool2);

	return 0;
}
