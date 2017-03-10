#include <userint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comport_hash.h"
 
#define HASH_LIMIT 0.5

typedef struct comport_hash_node_t {
	const char * key;                   /* key for comport_hash lookup */
	struct comport_info* data;          /* data in comport_hash node */
	struct comport_hash_node_t *next;           /* next node in comport_hash chain */
} comport_hash_node_t;

 
/*
 *  comport_hash() - Hash function returns a comport_hash number for a given key.
 *
 *  tptr: Pointer to a comport_hash table
 *  key: The key to create a comport_hash number for
 */
static int comport_hash(const comport_hash_t *tptr, const char *key)
{
	int i=0;
	int comport_hashvalue;
  
	while (*key != '\0')
    	i=(i<<3)+(*key++ - '0');
  
 	comport_hashvalue = (((i*1103515249)>>tptr->downshift) & tptr->mask);
   	
   	if (comport_hashvalue < 0)
     comport_hashvalue = 0;
 
	return comport_hashvalue;
}

 
/*
 *  comport_hash_init() - Initialize a new comport_hash table.
 *
 *  tptr: Pointer to the comport_hash table to initialize
 *  buckets: The number of initial buckets to create
 */
void comport_hash_init(comport_hash_t *tptr, int buckets) {
 
	/* make sure we allocate something */
	if (buckets==0)
		buckets=16;
 
   /* initialize the table */
   tptr->entries=0;
   tptr->size=2;
   tptr->mask=1;
   tptr->downshift=29;
 
   /* ensure buckets is a power of 2 */
   while (tptr->size < buckets) {
     tptr->size<<=1;
     tptr->mask=(tptr->mask<<1)+1;
     tptr->downshift--;
   } /* while */
 
   /* allocate memory for table */
   tptr->bucket=(comport_hash_node_t **) calloc(tptr->size, sizeof(comport_hash_node_t *));
 
   return;
 }

 
/*
 *  rebuild_table() - Create new comport_hash table when old one fills up.
 *
 *  tptr: Pointer to a comport_hash table
 */
static void rebuild_table(comport_hash_t *tptr)
{
	comport_hash_node_t **old_bucket, *old_comport_hash, *tmp;
	int old_size, h, i;
 
	old_bucket=tptr->bucket;
	old_size=tptr->size;
 
	/* create a new table and recomport_hash old buckets */
	comport_hash_init(tptr, old_size<<1);
	for (i=0; i<old_size; i++) {
		old_comport_hash=old_bucket[i];

		while(old_comport_hash) {
			tmp=old_comport_hash;
			old_comport_hash=old_comport_hash->next;
			h=comport_hash(tptr, tmp->key);
			tmp->next=tptr->bucket[h];
			tptr->bucket[h]=tmp;
			tptr->entries++;
		} /* while */
	} /* for */
 
	/* free memory used by old table */
	free(old_bucket);
 
	return;
}


/*
 *  comport_hash_find_node() - Find an entry in the comport_hash table and return a pointer to
 *  the node or NULL if it wasn't found.
 *
 *  tptr: Pointer to the comport_hash table
 *  key: The key to lookup
 */
static comport_hash_node_t * comport_hash_find_node(const comport_hash_t *tptr, const char *key)
{
	int h;
	comport_hash_node_t *node;
 
 
    /* find the entry in the comport_hash table */
    h=comport_hash(tptr, key);
    for (node=tptr->bucket[h]; node!=NULL; node=node->next) {
     if (!strcmp(node->key, key))
       break;
   }
   
   /* return the entry if it exists, can be NULL */
   return node;
}
 
 
/*
 *  comport_hash_lookup() - Lookup an entry in the comport_hash table and return a pointer to
 *    it or HASH_FAIL if it wasn't found.
 *
 *  tptr: Pointer to the comport_hash table
 *  key: The key to lookup
 */
comport_info* comport_hash_lookup(const comport_hash_t *tptr, const char *key)
{

	comport_hash_node_t *node;
 
    node = comport_hash_find_node(tptr, key);
 	
	/* return the entry if it exists, or HASH_FAIL */
	return(node ? node->data : NULL);
}


/*
 * comport_hash_foreach() - Calls the given function for each of the key/value pairs in the
 * comport_hash table.  The function is passed the key and value of each
 * pair.
 */
void comport_hash_foreach (const comport_hash_t *tptr, void (*func)(const char *, comport_info*, void*), void *extra )
{
  int bucket;
  comport_hash_node_t *node;
 
  for(bucket=0; bucket < tptr->size; bucket++) {
  	for (node=tptr->bucket[bucket]; node!=NULL; node=node->next) {
  		(*func)(node->key, node->data, extra);
  	}
  }
}


/*
 * comport_hash_data_new()
 * Allocate space for a comport_info* data object and set deafult values
 * Return NULL if fail
 */
comport_info* comport_hash_data_new()
{
	comport_info* com_data;	

	if ( (com_data = (struct comport_info *) malloc(sizeof(comport_info)) ) == NULL)
		return NULL;
		
	if ( (com_data->deviceName = (char *) malloc(MAX_DEVICE_NAME_LEN + 1)) == NULL)
		return NULL;
		
	com_data->port = 1;
	com_data->baudRate = 0;
	com_data->parity = 0;
	com_data->dataBits = 8;
	com_data->stopBits = 1;
	com_data->inputQueueSize = 164;
	com_data->outputQueueSize = 164;
	com_data->configured = 0;

	return com_data;
}


/*
 * comport_hash_data_destroy()
 * Destroy comport_info* data object
 */
void comport_hash_data_destroy(comport_info *data)
{
	free(data->deviceName);
	free(data);
}


/*
 * comport_hash_data_new(comport_info *data)
 * Allocate space and copy variables from struct comport_info* data
 * Return NULL if fail
 */
comport_info* comport_hash_data_new_copy(const comport_info *data)
{
	comport_info* com_data;	

	if ( (com_data = comport_hash_data_new()) == NULL )
		return NULL;
		
	if (com_data->deviceName != NULL)
		strcpy(com_data->deviceName, data->deviceName);

	com_data->port = data->port;
	com_data->baudRate = data->baudRate;
	com_data->parity = data->parity;
	com_data->dataBits = data->dataBits;
	com_data->stopBits = data->stopBits;
	com_data->inputQueueSize = data->inputQueueSize;
	com_data->outputQueueSize = data->outputQueueSize;
	com_data->configured = data->configured;

	return com_data;
}


/*
 *  comport_hash_insert() - Insert an entry into the comport_hash table.  If the entry already
 *  exists it is overriden.
 *
 *  tptr: A pointer to the comport_hash table
 *  key: The key to insert into the comport_hash table
 *  data: A pointer to the data to insert into the comport_hash table
 */
int comport_hash_insert(comport_hash_t *tptr, const char *key, const comport_info *data) {
 
	comport_hash_node_t *node;
	int h;
 
	/* check to see if the entry exists */
	if ((node = comport_hash_find_node(tptr, key)) != NULL) {
		/* Overrite Value and Return it*/
		comport_hash_data_destroy(node->data);
		
		if ( (node->data = comport_hash_data_new_copy(data)) == NULL )
			return HASH_FAIL;
	}
	else {
 
		/* expand the table if needed */
		while (tptr->entries>=HASH_LIMIT*tptr->size)
			rebuild_table(tptr);
 
		/* insert the new entry */
		h=comport_hash(tptr, key);
		node=(struct comport_hash_node_t *) malloc(sizeof(comport_hash_node_t));
		
		if ( node == NULL || (node->data = comport_hash_data_new_copy(data)) == NULL)
			return HASH_FAIL;
		
		node->key=key;
		node->next=tptr->bucket[h];
		tptr->bucket[h]=node;
		tptr->entries++;
	}
   	
	return HASH_SUCCESS;
}


/*
 *  comport_hash_delete() - Remove an entry from a comport_hash table 
 *  return HASH_FAIL if it wasn't found.
 *
 *  tptr: A pointer to the comport_hash table
 *  key: The key to remove from the comport_hash table
 */
int comport_hash_delete(comport_hash_t *tptr, const char *key) {

	comport_hash_node_t *node, *last;
	int h;
 
   	/* find the node to remove */
   	h=comport_hash(tptr, key);
   	for (node=tptr->bucket[h]; node; node=node->next) {
   	  if (!strcmp(node->key, key))
   	    break;
  	}
 
    /* Didn't find anything, return HASH_FAIL */
 	if (node==NULL)
    	return HASH_FAIL;
 
    /* if node is at head of bucket, we have it easy */
    if (node==tptr->bucket[h])
    	tptr->bucket[h]=node->next;
    else {
     	/* find the node before the node we want to remove */
     	for (last=tptr->bucket[h]; last && last->next; last=last->next) {
       		if (last->next==node)
         		break;
     	}
     	
     last->next=node->next;
   }

   /* free node and data */
   comport_hash_data_destroy(node->data);
   free(node);
 
   return HASH_SUCCESS;
 }
 
 
/*
 * comport_hash_destroy() - Delete the entire table, and all remaining entries.
 * 
 */
void comport_hash_destroy(comport_hash_t *tptr) {
	comport_hash_node_t *node, *last;
	int i;
 
	for (i=0; i<tptr->size; i++) {
		node = tptr->bucket[i];
		while (node != NULL) { 
			last = node;   
			node = node->next;
			
			comport_hash_data_destroy(last->data);
			free(last);
		}
	}     
 
	/* free the entire array of buckets */
	if (tptr->bucket != NULL) {
	free(tptr->bucket);
		memset(tptr, 0, sizeof(comport_hash_t));
	}
}

 
/*
 *  alos() - Find the average length of search.
 *
 *  tptr: Pointer to a comport_hash table
 */
static float alos(const comport_hash_t *tptr)
{

	int i,j;
	float alos=0;
	comport_hash_node_t *node;
 
 
	for (i=0; i<tptr->size; i++) {
		for (node=tptr->bucket[i], j=0; node!=NULL; node=node->next, j++);
			if (j)
				alos+=((j*(j+1))>>1);
	} /* for */
 
	return(tptr->entries ? alos/tptr->entries : 0);
}


/*
 * Return the size of the comport_hash table 
 */
int comport_hash_entry_count (comport_hash_t *tptr)
{
	return tptr->entries;
}

 
/*
 * Return positive integer if the comport_hash is not empty
 */
int comport_hash_is_empty (comport_hash_t *tptr)
{
	return (tptr->entries) ? 1 : 0;
}


/*
 *  comport_hash_stats() - Return a string with stats about a comport_hash table.
 *
 *  tptr: A pointer to the comport_hash table
 */
char * comport_hash_stats(const comport_hash_t *tptr)
{
   static char buf[1024];
 
   sprintf(buf, "%u slots, %u entries, and %1.2f ALOS",
     (int)tptr->size, (int)tptr->entries, alos(tptr));
 
   return(buf);
 }
