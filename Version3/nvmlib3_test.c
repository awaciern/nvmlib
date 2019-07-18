//Testing Program for Non-Volatile Memory Library Function Definitions Version 3
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

#include "nvmlib3.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() 
{
	printf("Creating pool1 of size 10...\n");
	pool* pool1 = pool_create("newfile.txt", 10);
	printf("Created pool's root OID offset: %d\n", pool1->root->offset);
	printf("Created pool's root OID pool: %s\n", pool1->root->pool->file_name);
	printf("Created pool's size: %d\n", pool1->size);
	printf("Created pool's OID 3 steps from root offset: %d\n", pool1->root->next->next->next->offset);
	printf("Created pool's OID 3 steps from root pool: %s\n", pool1->root->next->next->next->pool->file_name);
	printf("\n");

	printf("Closing and reopening pool1...\n\n");
	pool_close(pool1);
	pool_open(pool1);

	printf("Testing pool_root function on pool1...\n");
	printf("pool1's root OID offset: %d\n", (pool_root(pool1)).offset);
	printf("\n");
	
	printf("Allocating 10 more OIDs to pool1...\n");
	OID newdata_root = pmalloc(pool1, 10);
	printf("The root OID of the data just added to pool1: %d\n", newdata_root.offset);
	printf("pool1's new size: %d\n", pool1->size);
	printf("pool1's OID 14 steps from the offset: %d\n", pool1->root->next->next->next->next->next->next->next->next->next->next->next->next->next->next->offset);
	printf("\n");

	printf("Freeing the OID at offset 10 in pool1...\n");
	pfree(newdata_root);
	printf("New size of pool1: %d\n", pool1->size);
	printf("New offset value at OID 10 steps from root of pool1: %d\n", pool1->root->next->next->next->next->next->next->next->next->next->next->offset);
	printf("\n");

	printf("Writing 'Hello World!' to pool1\n\n");
	pwritef(pool1, "Hello World!\n");
	pwritef(pool1, "Hello+World!\n");
	pwritef(pool1, "Hello World!\n");
	printf("\n");

	printf("Closing and reopening pool1...\n\n");
	pool_close(pool1);
	pool_open(pool1);
	
	printf("Contents of pool1 File:\n");
	preadf(pool1);

	return 0;
}

