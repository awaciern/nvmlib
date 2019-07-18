//Testing Program 2 for Non-Volatile Memory Library Function Definitions Version 4
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

#include "nvmlib4.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() 
{
	printf("Creating pool1 of size 1000.\n");
	pool* pool1 = pool_create("pool1", 1000);
	printf("Created pool's size: %d\n", pool1->size);
	printf("\n");

	printf("words.input.txt has already been created to use with pool1...\n\n");

	printf("Reading words.input.txt file contents into pool1...\n\n");
	pfilein(pool1, "words.input.txt");

	printf("Contents of pool1:\n");
	preadf(pool1);
	printf("\n");

	printf("Freeing the OID at offset 0 x12 and 150 in pool2 using getoid...\n");
	int i;
	for (i=0; i<12; i++)
	{
		OID* offset0 = getoid(pool1, 0);
		printf("%c\n", offset0->data);
		pfree(offset0);
	}

	printf("Writing Edits to pool1...\n\n");
	pwritef(pool1, "\nI deleted the first 2 words and added this line!!!");

	printf("Contents of pool1:\n");
	preadf(pool1);
	printf("\n");

	printf("Writing pool1 contents to words.output.txt...\n\n");
	pfileout(pool1, "words.output.txt");

	printf("Closing pool1...\n\n");
	pool_close(pool1);

	return 0;
}
