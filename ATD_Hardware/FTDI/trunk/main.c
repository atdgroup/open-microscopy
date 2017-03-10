#define WIN32_LEAN_AND_MEAN

#include "ATD_CoarseZDrive_A.h"
#include "FTDI_Utils.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "asynctmr.h"

#include <userint.h>
#include <utility.h>

#include "config.h"

static int timer;

static int ui_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	MessagePopup("Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

/*
static int read_cube_position(FTDIController *controller)
{
	int mode, position = -1;

	BYTE data[10];  
	memset(data, 0, 10); 

	ftdi_controller_get_lock(controller);

	// NB Cube PIC expects 6 or more bytes even if they aren't relevant		
	data[0] = 255;	 // Set mode for reading  

	if(ftdi_controller_i2c_write_bytes(controller, data, 6, 0x32) != FT_OK) {
		ftdi_controller_release_lock(controller);
		return -1;
	}

	//data[0] = 0x64 | (0 << 1) | 0x01;
	
	if(ftdi_controller_i2c_read_bytes(controller, 0x32, 1, data) != FT_OK) {
		ftdi_controller_release_lock(controller);
		return -1;
	}

	position = data[0] & 0xff; 

	printf("Cube Position: %d\n", position);

	ftdi_controller_release_lock(controller);

	return FT_OK;
}


static int move_to_cube_position(FTDIController *controller, int position)
{
	int mode, err, number_of_cubes;
	BYTE data[10];

	memset(data, 0, 10);
	
	data[0] = 0; // Set position mode
	data[1] = position;

	if(ftdi_controller_i2c_write_bytes(controller, data, 6, 0x32) != FT_OK) {
		ftdi_controller_release_lock(controller);
		return -1;
	}

	return FT_OK;
}
*/


int CVICALLBACK OnTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			FTDIController *controller = (FTDIController *) callbackData;
	
			 //read_cube_position(controller);

            break;
		}
    }
    
	return 0;
}

static void OnCloseOrHideEvent (UIModule *module, void *data)
{
	QuitUserInterface(0);
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszzddLine, int nzddShow)
{
	char buf[100] = "";
	char **descriptions = NULL;
	int i, number_of_devices;
	BYTE a[100] = {6, 9, 4, 2, 1, 3};
	Z_Drive* zd = NULL;

	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;
	HWND hwnd;
	UINT myTimer;
	BYTE val = 0;

	FTDIController* controller = ftdi_controller_new();

	AllocConsole();
	
    handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if(controller == NULL) {
		printf("Failed to create FTDI controller\n");
		return -1;
	}

	zd = atd_coarse_zdrive_a_new("ATD_CoarseZDrive_A1", "ATD_CoarseZDrive_A1", ui_error_handler,
			TOPLEVEL_SOURCE_DIRECTORY "\\DataDir", "ATD_CoarseZDrive_A2.ini");

	val = 0x64;
	SETLSB(val)
	printf("0x%x\n", val);
	CLEARLSB(val)
	printf("0x%x\n", val);

	ftdi_controller_set_debugging(controller, 1);
	ftdi_controller_set_error_handler(controller, ftdi_error_handler, NULL);

	number_of_devices = ftdi_controller_get_number_of_connected_devices(controller);

	printf("Number of FTDI devices %d\n", number_of_devices);	

	ftdi_controller_print_serial_numbers_of_connected_devices(controller);

	ftdi_controller_open(controller, "DPCXXI5Q");

//	timer = NewAsyncTimer (1.0, -1, 1, OnTimerTick, controller);
	
	//read_cube_position(controller);
	//move_to_cube_position(controller, 2);

	//SetAsyncTimerAttribute (timer, ASYNC_ATTR_ENABLED,  1);

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(zd), OnCloseOrHideEvent, NULL);

	coarse_z_drive_hardware_initialise(zd);
	coarse_z_drive_initialise(zd);

	ui_module_display_main_panel(UIMODULE_CAST(zd));
	
	RunUserInterface();
	
	ftdi_controller_close(controller);
	coarse_z_drive_destroy(zd);

	return 0;
}