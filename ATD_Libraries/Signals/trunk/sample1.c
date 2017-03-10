#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"

static void onDestroyed(void *data)
{
	printf("onDestroyed Called\n");	
	
	return;
}


static void onDestroyedTwo()
{
	printf("onDestroyedTwo Called\n");	
	
	return;
}


int main (int argc, char *argv[])  
{
	signal_table signal_table;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10);
	
	GCI_Signal_New(&signal_table, "Destroy", VOID_VOID_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyed, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyedTwo, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Destroy", GCI_VOID) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Destroy signal\n");
	}
		
	if(GCI_Signal_Destroy(&signal_table, "Destroy") == SIGNAL_ERROR) {
	
		printf("Error disconnecting signal\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Destroy", GCI_VOID) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Destroy signal\n");
	}
	
	GCI_SignalSystem_Destroy(&signal_table);   
	
	while( !KeyHit() )
		; 
	
	return 0;	
}
