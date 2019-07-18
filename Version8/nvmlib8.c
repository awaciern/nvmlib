//Non-Volatile Memory Library Function Definitions Version 8
//As described in the paper "Hardware Supported Persistent Object Address Translation" by Dr. James Tuck (NCSU)
//Written by Avery Acierno (Undergraduate Research)

//Version 8 Improvements from Version 3:
//1. Pools and files operate seperately not in pairs: save file data into pool, manipulate, then save back
//2. Functions getoid, pfilein, pfileout added

//Version 8 Changes from Versions 4,5:
//1. Uses void* data that can be stored into binary files or text files
//2. data can be char or int
//3. pwritef -> pwriteint, pwritechar, pwritestr
//4. pfileintxt, pfileouttxt added

//Version 8 Changes from Version 7:
//1. Adds closed field to pool struct -> pool_close function doesn't free data, pool_open enabled
//2. Stroing oids in void* data of oid (See Test File)

//CURRENT PROBLEMS
//1. OID and OID Linked List not seperate structs
//2. Only works with binary file data to store ints
//3. pool_root does not incorporate size
//4. OIDs do not have different address values between pools
//5. Frees pools instead of "closing" them - FIXED
//6. No mode parameters in pool_create
//7. Name parameter in pool_create is just a variable name - IMPLEMENT LL OF POOLS REFERENCED BY NAME - FIXED
//8. No pool_open function - FIXED
//9. No persist function
//10. pool_root, pmalloc, pfree, getoid use OID* instead of OID
//11. Cant store char* in void* -> using cast char to ints workaround
//12. Empty oid will terminate all reading and file output of pool

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

//STRUCT DEFINITIONS

//For a linked list of OIDs
typedef struct oid
{
	void* data; //place to store data
	size_t data_size; //place to store size of data
	int data_type; //place to store type of data (1=int, 2=char, 3=oidptr)
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
	int closed; //whether the pool is open or not
	const char* name; //name of pool
	struct pool * next;
} pool;
pool* head_pool = NULL; //initializes the LL of pools with head indicator


//POOL MANAGEMENT

//Create a pool with specified size (in # of objects) and a name.
pool* pool_create(const char* name, int size)
{
	pool* p = malloc(sizeof(pool)); //creates pool pointer
	p->size = size; //sets pool size (# of objects);
	p->closed = 0;
	p->name = name; //sets pool name
	p->next = NULL; //sets NULL pointer to next pool

	if (head_pool == NULL) //inserts pool in the proper LL position
	{
		head_pool = p;
	}
	else
	{
		pool* tmp_pool = head_pool;
		while (tmp_pool->next != NULL)
		{
			tmp_pool = tmp_pool->next;
		}
		tmp_pool->next = p;
	}
	
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
pool* pool_open(const char* name)
{
	pool* p = head_pool;
	
	if (p == NULL) //NULL head_pool exception
	{
		return NULL;
	}
	
	do
	{
		if (strcmp(p->name, name) == 0)
		{
			p->closed = 0;
			printf("Pool %s successfully opened.\n", name);
			return p;
		}
		p = p->next;
	} 
	while (p != NULL);

	printf("ERROR: No pool of name %s found!\n", name);
	return NULL;
}

//Close a pool
void pool_close(pool* p)
{
	p->closed = 1;
}

//Return the root object of the pool p with specified size.
//The root object is intended for programmers to design as a directory of the pool.
OID* pool_root(pool* p) //, int size)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
		return NULL;	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
		return NULL;
	}
	else
	{
		return p->root;
	}
}


//OBJECT MANAGEMENT

//Allocate a chunk of persistent data of size on pool p and return the ObjectID of the first byte.
OID* pmalloc(pool * p, int size)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
		return NULL;	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
		return NULL;
	}
	else
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
			//tmp->next->data = '\0';
			tmp->next->empty = 1;
			tmp->next->next = NULL;
			tmp->next->pool = p;
			tmp = tmp->next;
		}

		p->size = p->size + size; //increases size of the pool to accomadate new OIDs

		return  newdata_root->next;
	}
}

//Free persistent data pointed by the OID
void pfree(OID* oid)
{
	if (oid == NULL) //oid  NULL exception
	{
		printf("ERROR: The specified oid is NULL!\n");	
	}
	else if (oid->pool->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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

			if (oid->offset == 0)
			{
				oid->pool->root = oid->next;
			}
			else if (oid->offset == 1)
			{
				oid->pool->root->next = oid->next;
			}
			else
			{
				tmp->next = tmp->next->next; //skips the pool about to be freed in LL
			}

			free(oid);

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
}


//OID ACCESS

//returns an oid at a cerain offset value
OID* getoid(pool* p, int offset)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
		return NULL;	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
		return NULL;
	}
	else
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
}

//returns a certain pool in the pool LL
pool* getpool(const char* name)
{
	pool* p = head_pool;

	if (p == NULL) //NULL head_pool exception
	{
		printf("ERROR: No pools have been created yet!\n");
		return NULL;
	}

	do
	{
		if (strcmp(p->name, name) == 0)
		{
			return p;
		}
		p = p->next;
	} 
	while (p->next != NULL);

	printf("ERROR: No pool of name %s found!\n", name);
	return NULL;
}


//READING AND WRITING:

//Write int to a pool
void pwriteint(pool* p, int num)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
	
		if (tmp->offset >= p->size || tmp->empty == 0)
		{
			printf("ERROR: Pool already full!\n");
		}
		else
		{
			tmp->data = (int*) num;
			tmp->data_size = sizeof(int);
			tmp->data_type = 1;
			tmp->empty = 0;
		}
	}
}

//Write char to a pool
void pwritechar(pool* p, char c)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
	
		if (tmp->offset >= p->size || tmp->empty == 0)
		{
			printf("ERROR: Pool already full!\n");
		}
		else
		{
			tmp->data = (int*) ((int)c);
			tmp->data_size = sizeof(int);
			tmp->data_type = 2;
			tmp->empty = 0;
		}
	}
}

//Write string to a pool
void pwritestr(pool* p, const char* string)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
				tmp->data = (int*) ((int)string[i]); //writes char to OIDs
				tmp->data_size = sizeof(int);
				tmp->data_type = 2;
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
}

//Write ptr to a pool
void pwriteptr(pool* p, void* ptr)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
	
		if (tmp->offset >= p->size || tmp->empty == 0)
		{
			printf("ERROR: Pool already full!\n");
		}
		else
		{
			tmp->data = ptr;
			tmp->data_size = sizeof(ptr);
			tmp->data_type = 3;
			tmp->empty = 0;
		}
	}
}

//Read a pool
void preadf(pool* p)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");	
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
	{
		OID * tmp = p->root;
		int i = 0;
		while (tmp->empty != 1)
		{
			if (tmp->data_type == 1)
			{
				printf("%d|", (int) tmp->data);
			}
			else if (tmp->data_type == 2)
			{
				printf("%c|", (int) tmp->data);
			}
			else if (tmp->data_type == 3)
			{
				printf("oidptr: pool:%s offset:%d\n", ((OID*)(tmp->data))->pool->name, ((OID*)(tmp->data))->offset);
			}
			else
			{
				printf("ERROR: Invalid data type at offset %d!\n", i);
			}
			i++;
			if (i < p->size)
			{
				tmp = tmp->next;
			}
			else
			{
				break;
			}
		}
	
		printf("\n");
	}
}


//FILE COMMUNICATION

//Read contents of a binary file into a pool
void pfilein(pool* p, char* filename)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
		if (tmp->offset >= p->size || tmp->empty == 0)
		{
			printf("ERROR: Pool already full!\n");
		}
		else
		{
			FILE* file_ptr = fopen(filename, "rb");
			int i = tmp->offset;
			while (fread(&(tmp->data), sizeof(int), 1, file_ptr) == 1)
			{
				tmp->empty = 0;
				tmp->data_size = sizeof(int);
				tmp->data_type = 1;
				if (tmp->next == NULL)
				{
					printf("ERROR: Not enough space in pool. Stopped writng to pool at file index %d\n", i);	
					break;			
				}
				else
				{
					tmp = tmp->next;
					i++;
				}
			}
			fclose(file_ptr);
		}
	}
}

//Read contents of a txt file into a pool
void pfileintxt(pool* p, char* filename)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
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
		if (tmp->offset >= p->size || tmp->empty == 0)
		{
			printf("ERROR: Pool already full!\n");
		}
		else
		{
			FILE* file_ptr = fopen(filename, "r");
			char c;
			c = fgetc(file_ptr);
			int i = tmp->offset;
			while (c != EOF)
			{
				tmp->data = (int*) ((int)c);
				tmp->empty = 0;
				tmp->data_size = sizeof(int);
				tmp->data_type = 2;
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
}

//Print contents of a pool out to a binary file
void pfileout(pool* p, const char* filename)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
	{
		FILE* file_ptr = fopen(filename, "wb");
		OID * tmp = p->root;

		while (tmp->empty != 1)
		{
			fwrite(&(tmp->data), tmp->data_size, 1, file_ptr);
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
}

//Print contents of a pool out to a txt file
void pfileouttxt(pool* p, const char* filename)
{
	if (p == NULL) //pool NULL exception
	{
		printf("ERROR: The specified pool is NULL!\n");
	}
	else if (p->closed == 1) //pool closed exception
	{
		printf("ERROR: The specified pool is closed!\n");
	}
	else
	{
		FILE* file_ptr = fopen(filename, "w");
		OID * tmp = p->root;

		while (tmp->empty != 1)
		{
			if (tmp->data_type == 1)
			{
				fprintf(file_ptr, "int data: %d\n", (int)tmp->data);
			}
			else if (tmp->data_type == 2)
			{
				fprintf(file_ptr, "%c", (int)tmp->data);
			}
			else if (tmp->data_type == 3)
			{
				fprintf(file_ptr, "oidptr: pool:%s offset:%d\n", ((OID*)(tmp->data))->pool->name, ((OID*)(tmp->data))->offset);
			}
			else
			{
				fprintf(file_ptr, "?");
			}

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
}



