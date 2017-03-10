#include "minipak mirror stepper.h"

////////////////////////////////////////////////////////////////////////////
//RJL April 2007
//GCI High Throughput system. 
//Standalone test for mirror stepper control.
////////////////////////////////////////////////////////////////////////////

static void OnMirrorStepperClose(MirrorStepper* mirror_stepper, void *data)
{
	mirror_stepper_destroy(mirror_stepper);
	
	QuitUserInterface(0);	
}

static void OnMirrorStepperChanged(MirrorStepper* mirror_stepper, void *data)
{
	printf("MirrorStepper changed\n");
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	MirrorStepper* mirror_stepper;

  	if (InitCVIRTE (hInstance, 0, 0) == 0)  return -1;    /* out of memory */

	//Create mirror_stepper instance
	if( (mirror_stepper = mirror_stepper_new("1", "Mirror switch controller")) == NULL)
		return -1;
	
	mirror_stepper_signal_hide_handler_connect (mirror_stepper, OnMirrorStepperClose, mirror_stepper); 
	mirror_stepper_changed_handler_connect (mirror_stepper, OnMirrorStepperChanged, mirror_stepper); 

	mirror_stepper_display_main_ui(mirror_stepper); 
	
	RunUserInterface();

  	return 0;
}
