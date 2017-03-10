#include <utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "signals.h"


static void onDestroyed(void *data)
{
	printf("onDestroyed Called: %d\n", *(int *)data);	
	
	return;
}


static void onDestroyedTwo(void *data)
{
	printf("onDestroyedTwo Called\n");	
	
	return;
}


int main (int argc, char *argv[])  
{
	signal_table signal_table;
	int one=1, two=2, id;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */  
	
	GCI_SignalSystem_Create(&signal_table, 10, "TestSignalSystem");
	
	GCI_Signal_New(&signal_table, "Destroy", VOID_VOID_MARSHALLER);
	
	if( (id=GCI_Signal_Connect(&signal_table, "Destroy", onDestroyed, &one)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyedTwo, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Destroy", GCI_VOID) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Destroy signal\n");
	}

	printf("So I can have one signal call 2 callbacks\n");
	getchar();
		
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyed, &two) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Destroy", GCI_VOID) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Destroy signal\n");
	}

	printf("So I can have one signal call the same callback twice\n");
	getchar();

	GCI_Signal_Disconnect(&signal_table, "Destroy", id);
	GCI_Signal_Emit(&signal_table, "Destroy", GCI_VOID);

	printf("And I can specifically disconect one.\n");
	
	if(GCI_Signal_Destroy(&signal_table, "Destroy") == SIGNAL_ERROR) {
	
		printf("Error disconnecting signal\n");
	}
	
	GCI_SignalSystem_Destroy(&signal_table);   
	
	while( !KeyHit() )
		; 
	
	return 0;	
}

/* DOES NOT WORK
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


static void onDestroyed()
{
	printf("onDestroyed Called\n");	
	
	return;
}


static void onDestroyedTwo()
{
	printf("onDestroyedTwo Called\n");	
	
	return;
}


static void onHello(char *name)
{
	printf("Hello %s \n", name);	
	
	return;
}


static void onTableRowValue (int row, double value)
{
	printf("Row %d Value %f \n", row, value);	
	
	return;
}
	

typedef struct
{
	int id;

} Window;	


static int VOID_WINDOW_PTR_MARSHALLER (void *handler, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Window *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Window *) args[0].void_ptr_data);
	
	return SIGNAL_SUCCESS;	
}


static void onWindowEvent(Window *window)
{
	printf("onWindowEvent Called window id %d\n", window->id);	
	
	return;
}


int main (int argc, char *argv[])  
{
	int id;
	signal_table signal_table;
	Window win;
	
	win.id = 22;
	
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;  
	
	GCI_SignalSystem_Create(&signal_table, 10, "TestSignalSystem");
	
	GCI_Signal_New(&signal_table, "TurretChanged", VOID_INT_MARSHALLER);
	
	if( (id = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChanged, NULL)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	GCI_Signal_Diconnect(&signal_table, "TurretChanged", id); 
	
	if( (id = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChangedFunctionTwo, NULL)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
		
	GCI_Signal_Diconnect(&signal_table, "TurretChanged", id); 
		
	if( (id = GCI_Signal_Connect(&signal_table, "TurretChanged", onTurretChangedFunctionThree, NULL)) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	GCI_Signal_Diconnect(&signal_table, "TurretChanged", id); 
	
	
	GCI_Signal_New(&signal_table, "Destroy", VOID_VOID_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyed, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if( GCI_Signal_Connect(&signal_table, "Destroy", onDestroyedTwo, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	GCI_Signal_New(&signal_table, "WindowEvent", VOID_WINDOW_PTR_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "WindowEvent", onWindowEvent, NULL) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "TurretChanged", INT, 7) == SIGNAL_ERROR) {
		
		printf("Error cannot emit signal\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Destroy", VOID) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Destroy signal\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "WindowEvent", VOID_POINTER, &win) == SIGNAL_ERROR) {
		
		printf("Error cannot emit WindowEvent signal\n");
	}
		
	//if(GCI_Signal_Destroy(&signal_table, "TurretChanged") == SIGNAL_ERROR) {
	
	//	printf("Error disconnecting signal\n");
	//}
	
	if(GCI_Signal_Emit(&signal_table, "TurretChanged", INT, 6) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Turret Changed signal\n");
	}
	
	
	GCI_Signal_New(&signal_table, "Hello", VOID_STRING_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "Hello", onHello) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "Hello", STRING, "Glenn Pierce") == SIGNAL_ERROR) {
		
		printf("Error cannot emit Hello signal\n");
	}
	
	
	
	GCI_Signal_New(&signal_table, "TableRowValue", VOID_INT_DOUBLE_MARSHALLER);
	
	if( GCI_Signal_Connect(&signal_table, "TableRowValue", onTableRowValue) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
	
	if(GCI_Signal_Emit(&signal_table, "TableRowValue", INT, 5, DOUBLE, 0.22) == SIGNAL_ERROR) {
		
		printf("Error cannot emit Hello signal\n");
	}
	
	while( !KeyHit() )
		; 
	
	return 0;	
}
*/