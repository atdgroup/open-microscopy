#ifndef __COMPORT_HASH_H__
#define __COMPORT_HASH_H__

#define HASH_FAIL -1
#define HASH_SUCCESS 1

#define MAX_DEVICE_NAME_LEN 15

typedef struct comport_info {
	int port;
	char *deviceName;
	long baudRate;
	int parity;
	int dataBits;
	int stopBits;
	int inputQueueSize;
	int outputQueueSize;
	int configured;
} comport_info;

typedef struct comport_hash_t {
	struct comport_hash_node_t **bucket;        /* array of comport_hash nodes */
    int size;                           /* size of the array */
    int entries;                        /* number of entries in table */
    int downshift;                      /* shift cound, used in comport_hash function */
    int mask;                           /* used to select bits for comport_hashing */
} comport_hash_t;


void comport_hash_init(comport_hash_t *, int);

comport_info* comport_hash_data_new(void);

void comport_hash_data_destroy(comport_info *data);

comport_info* comport_hash_data_new_copy(const comport_info *data);
 
comport_info* comport_hash_lookup (const comport_hash_t *, const char *);

int comport_hash_insert (comport_hash_t *, const char *, const comport_info*);
 
int comport_hash_delete (comport_hash_t *, const char *);

void comport_hash_foreach (const comport_hash_t *tptr, void (*func)(const char *, comport_info*, void*), void*);

int comport_hash_entry_count (comport_hash_t *);

int comport_hash_is_empty (comport_hash_t *);
 
void comport_hash_destroy(comport_hash_t *);
 
char *comport_hash_stats (const comport_hash_t *);

#endif

