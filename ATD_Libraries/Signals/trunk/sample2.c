#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"

static void onTurretChanged(int turret)
{
	printf("onTurrettChanged Called Parameter %d \n", turret);	
	
	return;
}

static void onTurretChangedFunctionTwo(int turret)
{
	printf("onTurretChangedFunctionTwo Called Parameter %d \n", turret);	
	
	return;
}

static void onTurretChangedFunctionThree(int turret)
{
	printf("onTurretChangedFunctionThree Called Parameter %d \n", turret);	
	
	return;
}


int main (int argc, char *argv[])  
{
	int id1, id2, id3;
	signal_table signal_table;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10);
	
	GCI_Signal_New(&signal_table, "TurretChanged", VOID_INT_MARSHALLER);
	
	if( (id1 = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChanged)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	
	if( (id2 = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChangedFunctionTwo)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
		
	if(GCI_Signal_Emit(&signal_table, "TurretChanged", GCI_INT, 7) == SIGNAL_ERROR) {
		
		printf("Error cannot emit signal\n");
	}
	
	GCI_Signal_Diconnect(&signal_table, "TurretChanged", id1);
	
	printf("\nRemoving signal handler onTurretChanged\n");
	
	GCI_Signal_Diconnect(&signal_table, "TurretChanged", id2); 
	
	printf("Removing signal handler onTurretChangedFunctionTwo\n\n");
		
	if( (id3 = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChangedFunctionThree)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	
	if(GCI_Signal_Emit(&signal_table, "TurretChanged", GCI_INT, 7) == SIGNAL_ERROR) {
		
		printf("Error cannot emit signal\n");
	}

	
	if(GCI_Signal_Emit(&signal_table, "TurretChanged", GCI_INT, 6) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Turret Changed signal\n");
	}
	
	GCI_SignalSystem_Destroy(&signal_table);   
	
	while( !KeyHit() )
		; 
	
	return 0;	
}
