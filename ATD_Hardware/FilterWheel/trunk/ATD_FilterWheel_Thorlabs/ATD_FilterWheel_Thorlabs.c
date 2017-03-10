#include "ATD_CubeSlider_A.h"
#include "CubeSliderUI.h"

#include "gci_utils.h"
#include "ATD_UsbInterface_A.h"

#include "asynctmr.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>
#include <cviauto.h>

////////////////////////////////////////////////////////////////////////////
//GP/RJL/RGN April 2007
//HTS Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

//#define DEBUG

//#define CUBE_PIC		0x64  	//Address programmed into cube slider PIC;
//#define CUBE_BUS 		2		//Set to required bus(on MPTR system) else set to 2
//#define CUBE_ADDRESS 	0		//Programable address of PIC

int atd_cubeslider_a_fluo_manager_hardware_init (FluoCubeManager* cube_manager)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;  

	initialise_comport(atd_cubeslider_a_cube_manager->_com_port, 9600);
	
	return CUBE_MANAGER_SUCCESS;
}

int atd_cubeslider_a_fluo_manager_destroy (FluoCubeManager* cube_manager)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(cube_manager->_timer);
	#endif

	close_comport(atd_cubeslider_a_cube_manager->_com_port);

	return CUBE_MANAGER_SUCCESS;
}


static int read_cube_position(FluoCubeManager* cube_manager, int *position)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	byte data[10];   
	int mode, err, number_of_cubes;

	GetI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");  
	
	memset(data, 0, 10);     
	
	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	mode = 255; 			 //Set mode for reading  
	data[0] = atd_cubeslider_a_cube_manager->_i2c_chip_type | (atd_cubeslider_a_cube_manager->_i2c_chip_address <<1);
	data[1] = mode;
	err = GCI_writeI2C_multiPort(atd_cubeslider_a_cube_manager->_com_port, 6, data, atd_cubeslider_a_cube_manager->_i2c_bus, "Cube read_cube_position");  	
	
	if (err) {
		ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position"); 
		return CUBE_MANAGER_ERROR;
	}

	data[0] = atd_cubeslider_a_cube_manager->_i2c_chip_type | (atd_cubeslider_a_cube_manager->_i2c_chip_address <<1) | 0x01;
		
	err = GCI_readI2C_multiPort(atd_cubeslider_a_cube_manager->_com_port, 1, data, atd_cubeslider_a_cube_manager->_i2c_bus, "Cube read_cube_position");
	
	if (err) {
		ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");   
		return CUBE_MANAGER_ERROR;
	}

	*position = data[0] & 0xff; 
	
	ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube read_cube_position");    
	
	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);

	if ((*position < 1) || ( *position > number_of_cubes))
		return CUBE_MANAGER_ERROR;
	
	return CUBE_MANAGER_SUCCESS;  
}
	
/*
static int wait_for_position(FluoCubeManager* cube_manager)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;  
	double start_time = Timer();
	// If the controller has not reached the desired pos within 10 seconds abort. 
	double diff = 0.0;
	int pos = -1;
	
	while (diff < 10.0) {
		
		read_cube_position(cube_manager, &pos);
		
		if (pos == atd_cubeslider_a_cube_manager->_requested_pos)
        	return CUBE_MANAGER_SUCCESS; 
		
		// Don't cram to much on the i2c bus. Rest for 0.2 seconds
        Delay(0.2);
        diff = Timer() - start_time ;
	}

	ui_module_send_error(UIMODULE_CAST(cube_manager), "Cube Error", "Cube slider failed to move");
	
	return CUBE_MANAGER_ERROR;    						 
}
*/

static int atd_cubeslider_a_move_to_cube_position(FluoCubeManager* cube_manager, int position)
{
	int mode, err, number_of_cubes;
	byte data[10];
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	
	memset(data, 0, 10);
	
	//Move filter cassette to specified position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;
	
	cube_manager_get_number_of_cubes(cube_manager, &number_of_cubes);
	
	if ((position < 1) || ( position > number_of_cubes)) {		
		return CUBE_MANAGER_ERROR;
	}

	GetI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube atd_cubeslider_a_move_to_cube_position");    
	
	//NB Cube PIC expects 6 or more bytes even if they aren't relevant
	mode = 0; 		//Set position mode
	data[0] = mode;
	data[1] = position;
	err = GCI_Out_PIC_multiPort (atd_cubeslider_a_cube_manager->_com_port, atd_cubeslider_a_cube_manager->_i2c_bus,
		atd_cubeslider_a_cube_manager->_i2c_chip_type, atd_cubeslider_a_cube_manager->_i2c_chip_address, 6, data);

	ReleaseI2CPortLock(atd_cubeslider_a_cube_manager->_com_port, "Cube atd_cubeslider_a_move_to_cube_position");  
	
	if (err) {
		printf("Err", "GCI_Out_PIC_multiPort returned error %d\n", err);	
		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;
}


static int atd_cubeslider_a_get_current_cube_position (FluoCubeManager* cube_manager, int *position)
{
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;   
	int error_count = 0;
    
	//Read current cube position, (1 to CUBE_TURRET_SIZE).

	if (cube_manager == NULL)
		return CUBE_MANAGER_ERROR;

	do {
		
		if(read_cube_position(cube_manager, position) == CUBE_MANAGER_SUCCESS)
			goto SUCCESS;
		
		// Don't cram to much on the i2c bus
		Delay(0.3);
	}
	while(error_count++ < 5);

	return CUBE_MANAGER_ERROR;
	
	SUCCESS:
	
	return CUBE_MANAGER_SUCCESS;
}


FluoCubeManager* atd_cubeslider_a_new(const char *name, const char *description, const char *data_dir, const char *file, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	FluoCubeManager* cube_manager = cube_manager_new(name, description, data_dir, file, sizeof(ATD_CUBESLIDER_A));
	ATD_CUBESLIDER_A* atd_cubeslider_a_cube_manager = (ATD_CUBESLIDER_A*) cube_manager;
	
	ui_module_set_error_handler(UIMODULE_CAST(cube_manager), error_handler, data); 
	
	CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) = atd_cubeslider_a_fluo_manager_hardware_init;   
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = atd_cubeslider_a_fluo_manager_destroy;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = atd_cubeslider_a_move_to_cube_position; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = atd_cubeslider_a_get_current_cube_position; 

	get_device_param_from_ini_file(name, "i2c_Bus", &(atd_cubeslider_a_cube_manager->_i2c_bus));
	get_device_param_from_ini_file(name, "i2c_ChipAddress", &(atd_cubeslider_a_cube_manager->_i2c_chip_address));  
	get_device_param_from_ini_file(name, "i2c_ChipType", &(atd_cubeslider_a_cube_manager->_i2c_chip_type));  
	get_device_param_from_ini_file(name, "COM_Port", &(atd_cubeslider_a_cube_manager->_com_port));  
	
	return cube_manager;
}









/*


Code from DAN
for the Thorlabs filter wheel

#include "toolbox.h"
#include <string.h>
#include <ansi_c.h>
#include <formatio.h>
#include <rs232.h>
#include <cvirte.h>		
#include <userint.h>
#include "sample1.h"


static int panelHandle;

int panel_handle,
    RS232Error;
char COMX[5] = "COM5";
int comport = 5;


void DisplayRS232Error (void);




int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	
	if ((panelHandle = LoadPanel (0, "sample1.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK panelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CloseCom(comport);
			QuitUserInterface (0);
			break;
		case EVENT_LEFT_CLICK:

			break;
	}
	return 0;
}

int CVICALLBACK setCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char send_data[7] = "pos=n\r";
	char send_data1[8] = "pos=1n\r";
	int P;
	int p;
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_LEFT_CLICK:
			GetCtrlVal(panelHandle,PANEL_FILTER_POS,&p);
			SetCtrlVal(panelHandle,PANEL_DISPLAY,p);   //displays vaule of p
			if(p<=9)								   // p is under 9
			{
				P = p + 48;							   // converting the number to the ASCII vaule
				send_data[4] = P;
				//SetCtrlVal(panelHandle,PANEL_OUTPUT,send_data);
				ComWrt(comport,send_data,6);
				RS232Error = ReturnRS232Err ();
				if (RS232Error)
            	DisplayRS232Error ();
			}
			else									   // p is 10, 11 or 12, extra character needed
			{
				P = p + 38;
				send_data1[5] = P;
				//SetCtrlVal(panelHandle,PANEL_OUTPUT,send_data1);
				ComWrt(comport,send_data1,7);
				RS232Error = ReturnRS232Err ();
				if (RS232Error)
            	DisplayRS232Error ();
			}
				
			break;
	}
	return 0;
}


int CVICALLBACK ConnectCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
		case EVENT_LEFT_CLICK:
			
			OpenComConfig (comport, COMX, 115200, 0, 8, 1, 512, 512);		   
			RS232Error = ReturnRS232Err ();
			if(RS232Error == 0)
			{
				MessagePopup ("RS232 Message" , "Filter Wheel connected and configured :-p");
				SetCtrlVal(panelHandle,PANEL_CONNECT_LED,1);
			}
			if (RS232Error)
				DisplayRS232Error ();
			
			//OpenCom (comport, COMX);
			
		break;
	}
	return 0;
}


void DisplayRS232Error (void)
{
    char ErrorMessage[200];
    switch (RS232Error)
        {
        default :
            if (RS232Error < 0)
                {  
                Fmt (ErrorMessage, "%s<RS232 error number %i \n check port connection", RS232Error);
                MessagePopup ("RS232 Message", ErrorMessage);
                }
            break;
        case 0  :
            MessagePopup ("RS232 Message", "No errors.");
            break;
        case -2 :
            Fmt (ErrorMessage, "%s", "Invalid port number (must be in the "
                                     "range 1 to 8).");
            MessagePopup ("RS232 Message", ErrorMessage);
            break;
        case -3 :
            Fmt (ErrorMessage, "%s", "No port is open.\n"
                 "Press connect to connect");
            MessagePopup ("RS232 Message", ErrorMessage);
            break;
        case -99 :
            Fmt (ErrorMessage, "%s", "Timeout error.\n\n"
                 
                 "       Check COM Port setting, or\n"
                 "       check device.");
            MessagePopup ("RS232 Message", ErrorMessage);
            break;
        }
}




*/