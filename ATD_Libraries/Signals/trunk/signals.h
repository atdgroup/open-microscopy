#ifndef __SIGNAL_HASH_H__
#define __SIGNAL_HASH_H__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include <userint.h>
#include "signals.h"

#define SIGNAL_SUCCESS 0
#define SIGNAL_ERROR -1

#define MAX_SIGNAL_NAME_LEN 50

enum argtype { GCI_END_OF_LIST, GCI_VOID, GCI_CHAR, GCI_INT, GCI_LONG,
			   GCI_FLOAT, GCI_DOUBLE, GCI_STRING, GCI_VOID_POINTER, GCI_POINT };

typedef union {
  
    char char_data;
    char uchar_data;
    int int_data;
    unsigned int uint_data;
    long long_data;
    unsigned long ulong_data;
    float float_data;
    double double_data;
    Point point_data;
    char *string_data;
    void *void_ptr_data;

} GCI_Signal_Arg;


typedef int (*GCI_SIGNAL_MARSHALLER) (void *handler, void* callback_data, GCI_Signal_Arg* args);

typedef struct signal_handler_node {
	
	struct signal_handler_node *next_handler;
	int id;
	void *callback_handler;
	void *callback_data;
	
} signal_handler_node; 


typedef struct signal_info {

	char signal_name[MAX_SIGNAL_NAME_LEN];
	GCI_SIGNAL_MARSHALLER marshaller;
	struct signal_handler_node *next_handler;
	
} signal_info;


typedef struct signal_table {
	struct signal_hash_node_t **bucket;        	/* array of signal_hash nodes */
    int size;									/* size of the array */
    int entries;								/* number of entries in table */
    int downshift;								/* shift cound, used in signal_hash function */
    int mask;									/* used to select bits for signal_hashing */
	int lock;
} signal_table;


int VOID_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_CHAR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_FLOAT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_VOID_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_STRING_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_INT_INT_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_DOUBLE_DOUBLE_DOUBLE_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_INT_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int VOID_DOUBLE_DOUBLE_DOUBLE_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args);

int GCI_SignalSystem_Create(signal_table *signal_hash, int number_of_signals, const char *name);

int GCI_SignalSystem_Destroy(signal_table *signal_hash); 

int GCI_Signal_New(signal_table *signal_hash, char *name, GCI_SIGNAL_MARSHALLER marshaller);

int GCI_Signal_Destroy(signal_table *signal_hash, char *name);

int GCI_Signal_IsConnected(signal_table *signal_hash, char *name);

int GCI_Signal_Connect(signal_table *signal_hash, char *name, void *handler, void *callback_data);

int GCI_Signal_Disconnect(signal_table *signal_hash, char *name, int id);

int GCI_Signal_Emit(const signal_table *signal_hash, char *name, ...);

int GCI_Signal_Emit_From_MainThread(const signal_table *signal_hash, char *name, ...);

int GCI_Signal_Suspend_All_Signals(void);


#endif
