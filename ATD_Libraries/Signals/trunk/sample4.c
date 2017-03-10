#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"

typedef struct
{
	int id;

} Window;	


static int VOID_WINDOW_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Window *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Window *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static void onWindowEvent(Window *window, void *callback_data)
{
	int *value;
	
	value = (int *) callback_data; 

	printf("onWindowEvent Called window id %d callback_data = %d \n", window->id, *value);	
	
	return;
}


int main (int argc, char *argv[])  
{
	signal_table signal_table;
	Window win;
	int data = 5;
	
	win.id = 22;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10);
	
	
	GCI_Signal_New(&signal_table, "WindowEvent", VOID_WINDOW_PTR_MARSHALLER);
	
	
	if( GCI_Signal_Connect(&signal_table, "WindowEvent", onWindowEvent, &data) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	
	if(GCI_Signal_Emit(&signal_table, "WindowEvent", GCI_VOID_POINTER, &win) == SIGNAL_ERROR) {
		
		printf("Error cannot emit WindowEvent signal\n");
	}

	GCI_SignalSystem_Destroy(&signal_table);   
	
	while( !KeyHit() )
		; 
	
	return 0;	
}
