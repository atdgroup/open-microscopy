#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"

static void onTableRowValue (int row, double value, void *callback_data)
{
	double *data = (double *) callback_data;
	
	printf("Row %d Value %f \n", row, value);	
	
	printf("We have user callback data of %f \n", *data);
	
	return;
}
	

int main (int argc, char *argv[])  
{
	double value = 5.668786;
	signal_table signal_table;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10);
	
	GCI_Signal_New(&signal_table, "TableRowValue", VOID_INT_DOUBLE_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "TableRowValue", onTableRowValue, &value) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "TableRowValue", GCI_INT, 5, GCI_DOUBLE, 0.22) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Hello signal\n");
	}
	
	GCI_SignalSystem_Destroy(&signal_table);  
	
	while( !KeyHit() )
		; 
	
	return 0;	
}
