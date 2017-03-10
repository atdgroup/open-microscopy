#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"
 
#include <windows.h>
#include <utility.h>

#ifdef INTERACTION_REPORT
#include "gci_ui_module.h"
#endif

#include "ThreadDebug.h"

#define MAX_SIGNALS_PER_TABLE 20
#define MAX_HANDLERS_PER_SIGNAL 100
#define MAX_ARGS_PER_SIGNAL 10
#define HASH_LIMIT 0.5

#define SIGNALS_BASE         			(WM_USER + 40)
#define SIGNALS_EMIT 					(SIGNALS_BASE + 1)

LPCTSTR ClsName = "HiddenWnd";
static HWND hidden_hwnd = 0;

typedef struct signal_hash_node_t {
	const char * key;                   /* key for signal_hash lookup */
	struct signal_info* data;           /* data in signal_hash node */
	struct signal_hash_node_t *next;    /* next node in signal_hash chain */
} signal_hash_node_t;

static int suspended = 0;
static int signal_table_number = 0;

typedef struct
{
	signal_info* signal;
	void *callback_handler;
	void *callback_data;
	GCI_Signal_Arg args[MAX_ARGS_PER_SIGNAL];

} HandlerInfo;

static LRESULT CALLBACK HiddenWindowWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
		case SIGNALS_EMIT:
		{
			HandlerInfo *info = (HandlerInfo*) lParam;
	
			// Call the ginal marshaller
			(info->signal->marshaller) (info->callback_handler, info->callback_data, info->args);

			break;
		}

		default:
		{
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
    }

    return 0;
}

void CreateHiddenWindow(void)
{
    WNDCLASSEX  WndClsEx;

    WndClsEx.cbSize        = sizeof(WNDCLASSEX);
    WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc   = HiddenWindowWndProc;
    WndClsEx.cbClsExtra    = 0;
    WndClsEx.cbWndExtra    = 0;
    WndClsEx.hInstance     = 0;
    WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClsEx.lpszMenuName  = NULL;
    WndClsEx.lpszClassName = ClsName;
    WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClsEx);

    hidden_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                          ClsName,
                          "Hidden signal Window",
                          WS_OVERLAPPEDWINDOW,
                          100,
                          120,
                          640,
                          480,
                          NULL,
                          NULL,
                          0,
                          NULL);

    ShowWindow(hidden_hwnd, SW_HIDE);
    UpdateWindow(hidden_hwnd);
}


static int signal_get_lock(signal_table *signal_hash, const char *name)
{
	return GciCmtGetLock(signal_hash->lock);
}

static int signal_release_lock(signal_table *signal_hash, const char *name)
{
	return GciCmtReleaseLock(signal_hash->lock);
}

/*
 *  signal_hash() - Hash function returns a signal_hash number for a given key.
 *
 *  tptr: Pointer to a signal_hash table
 *  key: The key to create a signal_hash number for
 */
static int signal_hash(const signal_table *tptr, const char *key)
{
	int i=0;
	int signal_hashvalue;
  
	while (*key != '\0')
    	i=(i<<3)+(*key++ - '0');
  
 	signal_hashvalue = (((i*1103515249)>>tptr->downshift) & tptr->mask);
   	
   	if (signal_hashvalue < 0) {
     signal_hashvalue = 0;
   	}    
 
	return signal_hashvalue;
}

 
/*
 *  signal_hash_init() - Initialize a new signal_hash table.
 *
 *  tptr: Pointer to the signal_hash table to initialize
 *  buckets: The number of initial buckets to create
 */
static void signal_hash_init(signal_table *tptr, int buckets)
{
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
   tptr->bucket=(signal_hash_node_t **) calloc(tptr->size, sizeof(signal_hash_node_t *));
 
   return;
 }

 
/*
 *  rebuild_table() - Create new signal_hash table when old one fills up.
 *
 *  tptr: Pointer to a signal_hash table
 */
static void rebuild_table(signal_table *tptr)
{
	signal_hash_node_t **old_bucket, *old_signal_hash, *tmp;
	int old_size, h, i;
 
	old_bucket=tptr->bucket;
	old_size=tptr->size;
 
	/* create a new table and resignal_hash old buckets */
	signal_hash_init(tptr, old_size<<1);
	for (i=0; i<old_size; i++) {
		old_signal_hash=old_bucket[i];

		while(old_signal_hash) {
			tmp=old_signal_hash;
			old_signal_hash=old_signal_hash->next;
			h=signal_hash(tptr, tmp->key);
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
 *  signal_hash_find_node() - Find an entry in the signal_hash table and return a pointer to
 *  the node or NULL if it wasn't found.
 *
 *  tptr: Pointer to the signal_hash table
 *  key: The key to lookup
 */
static signal_hash_node_t * signal_hash_find_node(const signal_table *tptr, const char *key)
{
	int h;
	signal_hash_node_t *node;
 
    /* find the entry in the signal_hash table */
    h=signal_hash(tptr, key);
    for (node=tptr->bucket[h]; node!=NULL; node=node->next) {
     if ( strcmp(node->key, key) == 0)
       break;
   }
   
   /* return the entry if it exists, can be NULL */
   return node;
}
 
 
/*
 *  signal_hash_lookup() - Lookup an entry in the signal_hash table and return a pointer to
 *    it or SIGNAL_ERROR if it wasn't found.
 *
 *  tptr: Pointer to the signal_hash table
 *  key: The key to lookup
 */
static signal_info* signal_lookup(const signal_table *tptr, const char *key)
{
	signal_hash_node_t *node;
 
    node = signal_hash_find_node(tptr, key);
 	
	/* return the entry if it exists, or SIGNAL_ERROR */
	return(node ? node->data : NULL);
}


/*
 * signal_hash_data_new()
 * Allocate space for a signal_info* data object and set deafult values
 * Return NULL if fail
 */
static signal_info* signal_new(const char *signal_name, GCI_SIGNAL_MARSHALLER marshaller, void *callback_handler)
{
	signal_info* signal_data;

	if ( (signal_data = (signal_info *) malloc(sizeof(signal_info)) ) == NULL)
		return NULL;
	
	if(callback_handler != NULL) {
		
		if ( (signal_data->next_handler = (signal_handler_node *) malloc(sizeof(signal_handler_node)) ) == NULL)
			return NULL;
		
		signal_data->next_handler->callback_handler = callback_handler;
		signal_data->next_handler->callback_data = NULL;
		signal_data->next_handler->id = 0;		
		signal_data->next_handler->next_handler = NULL;
	}
	else {
	
		signal_data->next_handler = NULL;		
	}	
		
	strcpy(signal_data->signal_name, signal_name);

	signal_data->marshaller = marshaller;

	return signal_data;
}


/*
 * signal_hash_data_destroy()
 * Destroy signal_info* data object
 */
static void signal_handlers_delete(signal_info *signal)
{
	signal_handler_node *handler_node, *next_handler_node;	
	
	handler_node = signal->next_handler;
	
	while (handler_node != NULL) { 
		
		next_handler_node = handler_node->next_handler;
		
		free(handler_node);
		
		handler_node = next_handler_node;
	}
}


/*
 * signal_hash_data_destroy()
 * Destroy signal_info* data object
 */
static int signal_delete(signal_info *signal)
{
	/* check to see if the entry exists */
	if (signal != NULL) {

		signal_handlers_delete(signal);
	
		free(signal);	
	}
	else {
 
		return SIGNAL_ERROR;
	}
   	
	return SIGNAL_SUCCESS;	
}


/*
 *  signal_hash_delete() - Remove an entry from a comport_hash table 
 *  return HASH_FAIL if it wasn't found.
 *
 *  tptr: A pointer to the signal_hash table
 *  key: The key to remove from the signal_hash table
 */
static int signal_hash_delete(signal_table *tptr, const char *key)
{
	signal_hash_node_t *node, *last;
	int h;
 
   	/* find the node to remove */
   	h=signal_hash(tptr, key);
   	for (node=tptr->bucket[h]; node; node=node->next) {
   	  if (!strcmp(node->key, key))
   	    break;
  	}
 
    /* Didn't find anything, return HASH_FAIL */
 	if (node==NULL)
    	return SIGNAL_ERROR;
 
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

   /* free node and signal */ 
   signal_delete(node->data);
   free(node);
 
   return SIGNAL_SUCCESS;
 }


/*
 *  signal_hash_insert() - Insert an entry into the signal_hash table.  If the entry already
 *  exists it is overriden.
 *
 *  tptr: A pointer to the signal_hash table
 *  key: The key to insert into the signal_hash table
 *  data: A pointer to the data to insert into the signal_hash table
 */
static int signal_callback_insert(const signal_table *tptr, char *key, void *callback_handler, void *callback_data)
{
	signal_info *signal;
	signal_handler_node *handler_node;	
 
	if(callback_handler == NULL)
		return SIGNAL_ERROR;		
	
	/* check to see if the entry exists */
	if ((signal = signal_lookup(tptr, key)) != NULL) {

		/* Add handler */	
		handler_node = signal->next_handler;	
		
		signal->next_handler = (signal_handler_node *) malloc(sizeof(signal_handler_node));
		
		if ( signal->next_handler == NULL)
			return SIGNAL_ERROR;
				
		signal->next_handler->callback_handler = callback_handler;
		signal->next_handler->callback_data = callback_data;
		
		if ( handler_node == NULL)
			signal->next_handler->id = 1;
		else
			signal->next_handler->id = handler_node->id + 1;	

		signal->next_handler->next_handler = handler_node;
		
	}
	else {
 
		return SIGNAL_ERROR;
	}
   	
	return signal->next_handler->id;
}


static int signal_callback_remove(const signal_table *tptr, char *key, int id)
{
	signal_info *signal;
	signal_handler_node *current_handler_node_ptr, *next_handler_node_ptr;
	
	signal_handler_node **last_node_ptr_ptr;
	
	if(id <= 0)
		return SIGNAL_ERROR;		
	
	/* check to see if the entry exists */
	if ((signal = signal_lookup(tptr, key)) != NULL) {	
		
	  		last_node_ptr_ptr = &(signal->next_handler);
	  		
	  		current_handler_node_ptr = signal->next_handler;
	  
			while( current_handler_node_ptr != NULL) {
			
		   		next_handler_node_ptr = current_handler_node_ptr->next_handler;  
				
				if(current_handler_node_ptr->id == id) {

					current_handler_node_ptr->callback_handler = NULL;
					current_handler_node_ptr->callback_data = NULL;
					free(current_handler_node_ptr);

					current_handler_node_ptr = NULL;  
					
					(*last_node_ptr_ptr) = next_handler_node_ptr;
					
				}
		   		else {
		   			   
					last_node_ptr_ptr = &((*last_node_ptr_ptr)->next_handler);
				}
				
				current_handler_node_ptr = next_handler_node_ptr;   
			}
	}
	else {
 
		return SIGNAL_ERROR;
	}
   	
	return SIGNAL_SUCCESS;
}


/*
 *  signal_hash_insert() - Insert an entry into the signal_hash table.  If the entry already
 *  exists it is overriden.
 *
 *  tptr: A pointer to the signal_hash table
 *  key: The key to insert into the signal_hash table
 *  data: A pointer to the data to insert into the signal_hash table
 */
static int signal_insert(signal_table *tptr, char *key, GCI_SIGNAL_MARSHALLER marshaller, void *callback_handler)
{
	signal_hash_node_t *node;
	int h;
 
	/* check to see if the entry does not exist */
	if ((node = signal_hash_find_node(tptr, key)) == NULL) {
 
		/* expand the table if needed */
		while (tptr->entries>=HASH_LIMIT*tptr->size)
			rebuild_table(tptr);
 
		/* insert the new entry */
		h=signal_hash(tptr, key);
		node=(struct signal_hash_node_t *) malloc(sizeof(signal_hash_node_t));
		
		if ( node == NULL || (node->data = signal_new(key, marshaller, callback_handler)) == NULL)
			return SIGNAL_ERROR;
		
		node->key=key;
		node->next=tptr->bucket[h];
		tptr->bucket[h]=node;
		tptr->entries++;
	}
	else {
		
		return SIGNAL_ERROR;
	}
   	
	return SIGNAL_SUCCESS;
}

 
/*
 * signal_hash_destroy() - Delete the entire table, and all remaining entries.
 * 
 */
static void signal_hash_destroy(signal_table *tptr)
{	
	signal_hash_node_t *node, *last;
	int i;
 
	for (i=0; i<tptr->size; i++) {
		
		node = tptr->bucket[i];

		while (node != NULL) { 
			last = node;   
			node = node->next;
				
			signal_delete(last->data);				
			free(last);
		}
	}     
 
	/* free the entire array of buckets */
	if (tptr->bucket != NULL) {
		free(tptr->bucket);
		memset(tptr, 0, sizeof(signal_table));
	}
}

int VOID_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (callback_data);
	
	return SIGNAL_SUCCESS;		
}

int VOID_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_CHAR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (char, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].char_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_FLOAT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (float, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].float_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (double, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].double_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (int, int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].int_data, args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_INT_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (int, double, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].int_data, args[1].double_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_DOUBLE_DOUBLE_DOUBLE_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (double, double, double, int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].double_data, args[1].double_data, args[2].double_data, args[3].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_INT_INT_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (int, int, int, int, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].int_data, args[1].int_data, args[2].int_data, args[3].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_DOUBLE_DOUBLE_DOUBLE_DOUBLE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (double, double, double, double, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].double_data, args[1].double_data, args[2].double_data, args[3].double_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int VOID_STRING_MARSHALLER (void *handler,void *callback_data,  GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (char *, void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (args[0].string_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int VOID_VOID_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (void*);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func (callback_data);
	
	return SIGNAL_SUCCESS;	
}


int GCI_SignalSystem_Create(signal_table *signal_hash, int number_of_signals, const char *name)
{
	char buf[50] = "";

	#ifdef INTERACTION_REPORT
//	printf ("Creating signal system %s", name);
	#endif

	if(hidden_hwnd <= 0)
		CreateHiddenWindow();

	signal_hash_init(signal_hash, number_of_signals);

	sprintf(buf, "Signal-%s", name);
	GciCmtNewLock (buf, 0, &(signal_hash->lock));

	#ifdef INTERACTION_REPORT
//	printf (" %d.\n", signal_hash->lock);
	printf ("%d [label=""%s""]\n", signal_hash->lock, name);
	#endif

	signal_table_number++;

	return SIGNAL_SUCCESS;	
}


int GCI_SignalSystem_Destroy(signal_table *signal_hash)
{
	CmtDiscardLock(signal_hash->lock);

	signal_hash_destroy(signal_hash);

	signal_table_number--;

	return SIGNAL_SUCCESS;
}


int GCI_Signal_New(signal_table *signal_hash, char *name, GCI_SIGNAL_MARSHALLER marshaller)
{
	signal_get_lock(signal_hash, name);

	signal_insert(signal_hash, name, marshaller, NULL);

	signal_release_lock(signal_hash, name);

	#ifdef INTERACTION_REPORT
//	printf ("Create Signal %d:%s\n", signal_hash->lock, name);
	#endif

	return SIGNAL_SUCCESS;
}


int GCI_Signal_Destroy(signal_table *signal_hash, char *name)
{
	signal_get_lock(signal_hash, name);

	if( signal_hash_delete(signal_hash, name) == SIGNAL_ERROR) {
		signal_release_lock(signal_hash, name);
		return SIGNAL_ERROR;
	}

	signal_release_lock(signal_hash, name);

	return SIGNAL_SUCCESS;
}


int GCI_Signal_Connect(signal_table *signal_hash, char *name, void *handler, void *callback_data)
{
	int id;
	
	signal_get_lock(signal_hash, name);

	if( (id = signal_callback_insert(signal_hash, name, handler,callback_data)) == SIGNAL_ERROR) {
		signal_release_lock(signal_hash, name);
		return SIGNAL_ERROR;
	}
	
	signal_release_lock(signal_hash, name);

	#ifdef INTERACTION_REPORT
//	printf ("%s connects to Signal %d:%s\n", UIMODULE_GET_NAME(callback_data), signal_hash->lock, name);
	printf ("%d -> %s\n", signal_hash->lock, UIMODULE_GET_NAME(callback_data));
	#endif

	return id;
}


int GCI_Signal_IsConnected(signal_table *signal_hash, char *name)
{
	if(signal_lookup(signal_hash, name)->next_handler == NULL)
		return 0;
		
	return 1;
}


int GCI_Signal_Disconnect(signal_table *signal_hash, char *name, int id)
{
	signal_get_lock(signal_hash, name);

	if( (signal_callback_remove(signal_hash, name, id) ) == SIGNAL_ERROR) {
		signal_release_lock(signal_hash, name);
		return SIGNAL_ERROR;
	}

	signal_release_lock(signal_hash, name);

	return SIGNAL_SUCCESS;
}


static int GCI_Signal_EmitAdvanced(const signal_table *signal_hash, int post_to_main_thread, char *name, va_list ap)
{
	int i=0, number_of_args, count = 0;
	enum argtype type;
	signal_info* signal;
	signal_handler_node *handler_node;	
	GCI_Signal_Arg args[MAX_ARGS_PER_SIGNAL];
	HandlerInfo handlers[MAX_HANDLERS_PER_SIGNAL];

	if(suspended)
		return SIGNAL_SUCCESS;

	CmtGetLock(signal_hash->lock);

	if( (signal = signal_lookup(signal_hash, name)) == NULL ) {
		
		CmtReleaseLock(signal_hash->lock);

		return SIGNAL_ERROR;
	}

	handler_node = signal->next_handler;
	
	do {
      	// get the type of the argument 
      	type = va_arg (ap, enum argtype);
 
      	// retrieve the argument based on the type 
      	switch (type) {
	
 			case GCI_VOID:
 				break;
 
			case GCI_VOID_POINTER:
				args[i++].void_ptr_data = va_arg (ap, void *);				
        		break;
			
			case GCI_INT:
				args[i++].int_data = va_arg (ap, int);				
        		break;
        		
        	case GCI_CHAR:
				args[i++].char_data = va_arg (ap, char);				
        		break;
			
			case GCI_LONG:
				args[i++].long_data = va_arg (ap, long);				
        		break;
			
			case GCI_FLOAT:
				args[i++].float_data = va_arg (ap, float);				
        		break;
			
			case GCI_DOUBLE:
				args[i++].double_data = va_arg (ap, double);				
        		break;
			
			case GCI_STRING:
				args[i++].string_data = va_arg (ap, char *);				
        		break;
			
			case GCI_POINT:
				args[i++].point_data = va_arg (ap, Point);				
        		break;
		
   			case GCI_END_OF_LIST:
        		break;
 
   			default:
        		type = GCI_END_OF_LIST; // to break loop 
        		break;
      	} 
		
  	} while (type != GCI_END_OF_LIST);
		
	number_of_args = i;
	count = 0;

	// Notify Handlers
	while (handler_node != NULL) { 
		
		assert(handler_node != NULL);
		
		handlers[count].signal = signal;
		handlers[count].callback_handler = handler_node->callback_handler;
		handlers[count].callback_data = handler_node->callback_data;
		memcpy(&(handlers[count].args), args, sizeof(GCI_Signal_Arg) * number_of_args);

		count++;

		handler_node = handler_node->next_handler;
	}
	
	// Go through as call all the handers attached.
	for(i=0; i < count; i++)
	{
		if(post_to_main_thread) {
			SendMessageTimeout(hidden_hwnd, SIGNALS_EMIT, 0, (LPARAM) &(handlers[i]),
				SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ABORTIFHUNG, 1000, NULL); 
		}
		else {
			(handlers[i].signal->marshaller) (handlers[i].callback_handler, handlers[i].callback_data, args);
		}
	}

	CmtReleaseLock(signal_hash->lock);

	return SIGNAL_SUCCESS;	
}


int GCI_Signal_Emit(const signal_table *signal_hash, char *name, ...)
{
	int ret;
	va_list ap;

	va_start(ap, name);

	ret = GCI_Signal_EmitAdvanced(signal_hash, 0, name, ap);

	va_end(ap);

	return ret;
}

int GCI_Signal_Emit_From_MainThread(const signal_table *signal_hash, char *name, ...)
{
	int ret;
	va_list ap;

	va_start(ap, name);

	ret = GCI_Signal_EmitAdvanced(signal_hash, 1, name, ap);

	va_end(ap);

	return ret;
}

int GCI_Signal_Suspend_All_Signals(void)
{
	suspended = 1;

	return SIGNAL_SUCCESS;	
}