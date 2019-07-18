//Non-Volatile Memory Library Function Definitions Version 3
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

//CURRENT PROBLEMS
//1. OID and OID Linked List not seperate structs
//2. pool_open uses pointer to pool rather than name
//3. Files and OID structure are operating in parallel, not together
//4. Getting File data into OIDs? - ONLY CHARS RIGHT NOW
//5. pool_root does not incorporate size
//6. OIDs do not have different address values between pools
//7. Freeing an OID leads to a skipped offest number
//8. Still writes to actual C file when pool out of OIDs - FIXED
//9. OIDs can only store chars -> model only works for text files
//10. Does not open and close pools/OIDs (only C files)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/**********************************************************************************************/


//STRUCT DEFINITIONS

//For a linked list of OIDs
typedef struct oid
{
	char data; //holds data
	int offset; //offset from root OID of pool
	struct oid * next;
	struct pool * pool; //pool identifier for OID
} OID;

//For a pool
typedef struct pool
{
	FILE* file_ptr; //uses implemented FILE structure of C language
	const char* file_name; //stores filename used on computer
	OID * root; //Linked list of OIDs starting at root OID
	int size; //# of objects in pool
} pool;


/**********************************************************************************************/


//POOL MANAGEMENT

//Create a pool with specified size (in # of objects) and a name.
pool* pool_create(const char* name, int size)
{
	FILE* f = fopen(name, "w+"); //creates new file

	pool* p = malloc(sizeof(pool)); //creates pool pointer
	p->file_ptr = f; //sets pool file pointer
	p->file_name = name; //sets pool filename
	p->size = size; //sets pool size (# of objects);
	
	p->root = malloc(sizeof(OID)); //allocate root OID for the pool
	p->root->offset = 0; //fills in OID struct fields
	p->root->next = NULL;
	p->root->pool = p;
	
	OID * tmp = p->root;
	int i;	
	for (i = 1; i < size; i++) //allocates # of OIDs specified by size
	{
		tmp->next = malloc(sizeof(OID));
		tmp->next->offset = i;
		tmp->next->next = NULL;
		tmp->next->pool = p;
		tmp = tmp->next;
	}

	return p;
}

//Reopen a pool that is previously created by the same program.
//Permissions will be checked.
pool* pool_open(pool* p)
{
	fopen(p->file_name, "r+");
	return p;
}

//Close a pool
void pool_close(pool* p)
{
	fclose(p->file_ptr);
}

//Return the root object of the pool p with specified size.
//The root object is intended for programmers to design as a directory of the pool.
OID pool_root(pool* p) //, int size)
{
	return *(p->root);
}


/**********************************************************************************************/


//OBJECT MANAGEMENT

//Allocate a chunk of persistent data of size on pool p and return the ObjectID of the first byte.
OID pmalloc(pool * p, int size)
{
	OID * newdata_root = p->root;
	int i;
	for (i = 1; i < p->size; i++) //cycles to last OID of pool
	{
		newdata_root = newdata_root->next;
	}
	
	OID * tmp = newdata_root;
	int j;	
	for (j = p->size; j < (p->size + size); j++) //allocates # of OIDs specified by size
	{
		tmp->next = malloc(sizeof(OID));
		tmp->next->offset = j;
		tmp->next->next = NULL;
		tmp->next->pool = p;
		tmp = tmp->next;
	}

	p->size = p->size + size; //increases size of the pool to accomadate new OIDs

	return  *(newdata_root->next);
}

//Free persistent data pointed by the OID
void pfree(OID oid)
{

	oid.pool->size = oid.pool->size - 1; //decrements size of the pool	

	OID * tmp = oid.pool->root;
	int i;
	for (i = 0; i < oid.offset - 1; i++) //cycles to OID of pool before OID to be deleted
	{
		tmp = tmp->next;
	}
	
	tmp->next = tmp->next->next; //skips the pool about to be freed in LL
	
	free(tmp->next); //frees specified OID
}


/**********************************************************************************************/


//READING AND WRITING

//Write to a file (using traditional C FILE structure) and pool (using OID storage)
void pwritef(pool* p, const char* string)
{
	OID * tmp = p->root;
	while (tmp->data != '\0') //cycles to first OID of the pool with no data
	{
		if (tmp->next == NULL) //identifies if pool already full
		{
			break;
		}
		else
		{
			tmp = tmp->next;
		}
	}
	
	if (tmp->next == NULL) //pool already full exception
	{
		printf("ERROR: Pool already full!\n");
	}
	else
	{
		int i;
		int sl = strlen(string);
		for (i = 0; i < sl; i++) //cycles through each char of string
		{
			tmp->data = string[i]; //writes char to OID
			fprintf(p->file_ptr, "%c", string[i]); //writes char to file
			if (tmp->next == NULL) //not enough space in pool exception
			{
				printf("ERROR: Not enough space in pool. Stopped writng to pool at string index %d\n", i);	
				break;			
			}
			else
			{
				tmp = tmp->next;
			}
		}
	}
}

//Read a file (using traditional C FILE structure) and pool (using OID storage)
void preadf(pool* p)
{
	printf("TRADITIONAL C FILE STRUCTURE:\n"); 
	char c;
	c = fgetc(p->file_ptr); //prints contents of C file
	while (c != EOF)
	{
		printf("%c", c);
		c = fgetc(p->file_ptr);
	}
	printf("n");
	
	printf("POOL OID STRUCTURE:\n");
	OID * tmp = p->root;
	int i = 0;
	c = tmp->data;
	while (c != '\0') //prints contents of OIDs
	{
		printf("%c", c);
		i++;
		if (i < p->size)
		{
			tmp = tmp->next;
			c = tmp->data;
		}
		else //pool out of space exit condition
		{
			break;
		}
	}
	
	printf("\n");
}





