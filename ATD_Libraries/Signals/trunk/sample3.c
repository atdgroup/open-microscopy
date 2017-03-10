#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"

static void onHello(char *name)
{
	printf("Hello %s \n", name);	
	
	return;
}


int main (int argc, char *argv[])  
{
	signal_table signal_table;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10);
	
	GCI_Signal_New(&signal_table, "Hello", VOID_STRING_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "Hello", onHello) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Hello", GCI_STRING, "Glenn Pierce") == SIGNAL_ERROR) {
		
		printf("Error cannot emit Hello signal\n");
	}

	GCI_SignalSystem_Destroy(&signal_table);   

	while( !KeyHit() )
		; 
		
	return 0;	
}
