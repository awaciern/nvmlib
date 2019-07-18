//Non-Volatile Memory Library Function Definitions Version 4
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

//Version 4 Improvements from Version 3:
//1. Pools and files operate seperately not in pairs: save file data into pool, manipulate, then save back
//2. Functions getoid, pfilein, pfileout added

//CURRENT PROBLEMS
//1. OID and OID Linked List not seperate structs
//2. pool_open uses pointer to pool rather than name
//3. Getting File data into OIDs? - ONLY CHARS RIGHT NOW -> ONLY WORKS WITH TEXT FILES
//4. pool_root does not incorporate size
//5. OIDs do not have different address values between pools
//6. Freeing an OID leads to a skipped offest number - FIXED
//7. Still writes to actual C file when pool out of OIDs - FIXED
//8. Frees pools instead of "closing" them 
//9. Only 1 pool can be in use at a tme (Else -> Segmentation Fault) - FIXED
//10. No mode parameters in pool_create
//11. Name parameter in pool_create is just a variable name - IMPLEMENT LL OF POOLS REFERENCED BY NAME
//12. No pool_open function
//13. No persist function
//14. fclose in pfilein leads to seg fault - FIXED
//15. Can't free multiple OIDs in succession - FIXED
//16. pool_root, pmalloc, pfree, getoid use OID* instead of OID

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//STRUCT DEFINITIONS

//For a linked list of OIDs
typedef struct oid
{
	char data; //place to store char data
	int offset; //offset from root OID of pool
	int empty; //1 if char data has been written to, 0 else
	struct oid * next;
	struct pool * pool;
} OID;

//For a pool
typedef struct pool
{
	OID * root; //Linked list of OIDs starting at root OID
	int size; //# of objects in pool
	const char* name; //name of pool
} pool;


//POOL MANAGEMENT

//Create a pool with specified size (in # of objects) and a name.
pool* pool_create(const char* name, int size)
{
	pool* p = malloc(sizeof(pool)); //creates pool pointer
	p->size = size; //sets pool size (# of objects);
	p->name = name; //sets pool name
	
	p->root = malloc(sizeof(OID)); //allocate root OID for the pool
	p->root->offset = 0;
	p->root->empty = 1;	
	p->root->next = NULL;
	p->root->pool = p;
	
	OID * tmp = p->root;
	int i;	
	for (i = 1; i < size; i++) //allocates # of OIDs specified by size
	{
		tmp->next = malloc(sizeof(OID));
		tmp->next->offset = i;
		tmp->next->empty = 1;
		tmp->next->next = NULL;
		tmp->next->pool = p;
		tmp = tmp->next;
	}

	return p;
}

//Reopen a pool that is previously created by the same program.
//Permissions will be checked.
/*pool* pool_open(pool* p)
{
	fopen(p->file_name, "r+");
	return p;
}*/

//Close a pool
void pool_close(pool* p)
{
	OID * tmp;
	int i;
	for(i = 0; i < p->size - 1; i++) //loops through all OIDs in pool and frees them
	{
		tmp = p->root;
		p->root = p->root->next;
		free(tmp);
	}
	free(p);
}

//Return the root object of the pool p with specified size.
//The root object is intended for programmers to design as a directory of the pool.
OID* pool_root(pool* p) //, int size)
{
	return p->root;
}


//OBJECT MANAGEMENT

//Allocate a chunk of persistent data of size on pool p and return the ObjectID of the first byte.
OID* pmalloc(pool * p, int size)
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
		tmp->next->empty = 1;
		tmp->next->next = NULL;
		tmp->next->pool = p;
		tmp = tmp->next;
	}

	p->size = p->size + size; //increases size of the pool to accomadate new OIDs

	return  newdata_root->next;
}

//Free persistent data pointed by the OID
void pfree(OID* oid)
{
	if (oid != NULL)
	{
		oid->pool->size = oid->pool->size - 1; //decrements size of the pool	

		OID * tmp = oid->pool->root;
		int i;
		for (i = 0; i < oid->offset - 1; i++) //cycles to OID of pool before OID to be deleted
		{
			tmp = tmp->next;
		}
	
		free(oid);

		tmp->next = tmp->next->next; //skips the pool about to be freed in LL

		for (; i < tmp->pool->size - 1; i++)
		{
			tmp->next->offset = tmp->next->offset - 1;
			tmp = tmp->next;
		}
	}
	else
	{
		printf("ERROR: oid passed to pfree is NULL!\n");
	}
}


//OID ACCESS

//returns an oid at a cerain offset value
OID* getoid(pool* p, int offset)
{
	if (offset >= p->size)
	{
		printf("ERROR: offset too large for pool size!\n");
		return NULL;
	}
	else
	{
		OID * tmp = p->root;
		while (tmp->offset != offset)
		{
			tmp = tmp->next;
		}
		return tmp;
	}
}


//READING AND WRITING:

//Write to a pool
void pwritef(pool* p, const char* string)
{
	OID * tmp = p->root;
	while (tmp->empty != 1) //cycles to first OID of the pool with no data
	{
		if (tmp->next == NULL)
		{
			break;
		}
		else
		{
			tmp = tmp->next;
		}
	}
	
	if (tmp->offset >= p->size - 1)
	{
		printf("ERROR: Pool already full!\n");
	}
	else
	{
		int i;
		int sl = strlen(string);
		for (i = 0; i < sl; i++)
		{
			tmp->data = string[i]; //writes char to OIDs
			tmp->empty = 0;
			if (tmp->offset >= p->size - 1)
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

//Read a pool
void preadf(pool* p)
{
	OID * tmp = p->root;
	int i = 0;
	char c = tmp->data;
	while (tmp->empty != 1)
	{
		printf("%c", c);
		i++;
		if (i < p->size)
		{
			tmp = tmp->next;
			c = tmp->data;
		}
		else
		{
			break;
		}
	}
	
	printf("\n");
}


//FILE COMMUNICATION

//Read contents of a file into a pool
void pfilein(pool* p, char* filename)
{
	OID * tmp = p->root;
	while (tmp->empty != 1) //cycles to first OID of the pool with no data
	{
		if (tmp->next == NULL)
		{
			break;
		}
		else
		{
			tmp = tmp->next;
		}
	}
	if (tmp->offset >= p->size)
	{
		printf("ERROR: Pool already full!\n");
	}
	else
	{
		FILE* file_ptr = fopen(filename, "r");
		char c;
		c = fgetc(file_ptr);
		int i = 0;
		while (c != EOF)
		{
			tmp->data = c;
			tmp->empty = 0;
			if (tmp->next == NULL)
			{
				printf("ERROR: Not enough space in pool. Stopped writng to pool at file index %d\n", i);	
				break;			
			}
			else
			{
				tmp = tmp->next;
				c = fgetc(file_ptr);
				i++;
			}
		}
		fclose(file_ptr);
	}
}

//Print contents of a pool out to a file
void pfileout(pool* p, const char* filename)
{
	FILE* file_ptr = fopen(filename, "w");
	OID * tmp = p->root;
	while (tmp->empty != 1)
	{
		fprintf(file_ptr, "%c", tmp->data);
		if (tmp->next == NULL)
		{
			break;
		}
		else
		{
			tmp = tmp->next;
		}
	}
	fclose(file_ptr);
}



