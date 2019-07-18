//Testing Program for Non-Volatile Memory Library Function Definitions Version 8
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

#include "nvmlib8.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() 
{	
	printf("Attempting to open an invalid pool name...\n");
	pool_open("Nonexistent");
	printf("\n");

	printf("Creating pool1 of size 10...\n");
	pool* pool1 = pool_create("pool1", 10);
	printf("Created pool's root OID offset: %d\n", pool1->root->offset);
	printf("Created pool's root OID pool: %s\n", pool1->root->pool->name);
	printf("Created pool's size: %d\n", pool1->size);
	printf("Created pool's OID 3 steps from root offset: %d\n", pool1->root->next->next->next->offset);
	printf("\n");

	printf("Testing pool_root function on pool1...\n");
	printf("pool1's root OID offset: %d\n", (pool_root(pool1))->offset);
	printf("\n");
	
	printf("Closing pool1...\n\n");
	pool_close(pool1);

	printf("Allocating 10 more OIDs to pool1...\n");
	OID* newdata_root = pmalloc(pool1, 10);
	printf("\n");

	printf("Reopening pool1...\n");
	pool_open("pool1");	
	printf("\n");

	printf("Allocating 10 more OIDs to pool1...\n");
	newdata_root = pmalloc(pool1, 10);
	printf("The root OID of the data just added to pool1: %d\n", newdata_root->offset);
	printf("pool1's new size: %d\n", pool1->size);
	printf("pool1's OID 14 steps from the offset: %d\n", pool1->root->next->next->next->next->next->next->next->next->next->next->next->next->next->next->offset);
	printf("\n");

	printf("Freeing the OID at offset 10 in pool1...\n");
	pfree(newdata_root);
	printf("New size of pool1: %d\n", pool1->size);
	printf("New offset value at OID 10 steps from root of pool1: %d\n", pool1->root->next->next->next->next->next->next->next->next->next->next->offset);
	printf("\n");

	printf("Writing multiples of 25 to pool1 (25 to 500)\n\n");
	int i;
	for (i = 25; i <= 500; i = i + 25)
	{
		pwriteint(pool1, i);
	}
	
	printf("Contents of pool1:\n");
	preadf(pool1);
	printf("\n");

	printf("Writing pool1 to file plus25.bin...\n");
	pfileout(pool1, "plus25.bin");
	printf("\n");

	printf("Creating pool2 of size 100...\n\n");
	pool* pool2 = pool_create("pool2", 100);

	printf("Reading file plus25.bin into pool2...\n");
	pfilein(pool2, "plus25.bin");
	printf("\n");

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Freeing the OID at offset 3 x3 and 150 in pool2 using getoid...\n");
	OID* offset3 = getoid(pool2, 3);
	pfree(offset3);
	offset3 = getoid(pool2, 3);
	pfree(offset3);
	offset3 = getoid(pool2, 3);
	pfree(offset3);
	OID* offset150 = getoid(pool2, 150);
	pfree(offset150);
	printf("\n");

	printf("Closing pool2...\n\n");
	pool_close(pool2);

	printf("Writing abcde to pool2\n\n");
	for (i = 97; i < 102; i++)
	{
		printf("%c\n", (char) i);
		pwritechar(pool2, (char) i);
	}	

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Reopening pool2...\n");
	pool_open("pool2");
	printf("\n");

	printf("Writing abcde to pool2\n\n");
	for (i = 97; i < 102; i++)
	{
		printf("%c\n", (char) i);
		pwritechar(pool2, (char) i);
	}	

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Writing 'I added this line with pwritestr!' to pool2\n\n");
	pwritestr(pool2, "\nI added this line with pwritestr!\n");

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Allocating 100 more OIDs to pool2...\n\n");
	pmalloc(pool2, 100);

	printf("Reading textfile.txt into pool2...\n");
	pfileintxt(pool2, "textfile.txt");
	printf("\n");

	printf("Contents of pool2:\n");
	preadf(pool2);
	printf("\n");

	printf("Writing pool2 to file plus25abcde2strlines.bin...\n");
	pfileout(pool2, "plus25abcde2strlines.bin");
	printf("\n");

	printf("Writing pool2 to file plus25abcde2strlines.txt...\n");
	pfileouttxt(pool2, "plus25abcde2strlines.txt");
	printf("\n");

	printf("Closing pool1...\n\n");
	pool_close(pool1);

	printf("Closing pool2...\n\n");
	pool_close(pool2);

	printf("Attempting to open an invalid pool name...\n");
	pool_open("Nonexistent");
	printf("\n");

	printf("\n");

	return 0;
}
