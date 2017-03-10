#include "ATD_Shutter_A.h"
#include "ShutterUI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include <utility.h>


int atd_shutter_a_shutter_set_automatic_control(Shutter* shutter, int computer)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;  
	unsigned char val[10];
	
	computer = !computer;
	
	memset(val, 0, 10);
	
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte & 0xfd;   // Clear control bit
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte | computer <<1;	//Set control bit
	 		
	val[0] = 0x00;	    		//R/W mode 
   	val[1] = 0xff;				//Write
    val[2] = atd_shutter_a_shutter->_fast_byte;	//Set A1 output 
    		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_fastline_write_bytes(atd_shutter_a_shutter->controller, val, 3) != FT_OK) {
		return  SHUTTER_ERROR;
	}

	#else

   	if(GCI_writeFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 3, "Shutter Manual Control"))    //Three bytes
		return  SHUTTER_ERROR;

	#endif

	return  SHUTTER_SUCCESS;
}


static int get_status(Shutter* shutter, int *shutter_charging, int *shutter_fb)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;  
	unsigned char val[10];
	int ret;
	
	memset(val, 0, 10);
	
	#ifdef HEAVY_I2C_TESTING
	{
		*shutter_charging = 0;
		*shutter_fb = atd_shutter_a_shutter->shutter_status;
		return SHUTTER_SUCCESS;
	}
	#endif

	#ifdef FTDI_NO_VIRTUAL_COMPORT

		if(ftdi_controller_i2c_fastline_read_bytes(atd_shutter_a_shutter->controller, 1, val) != FT_OK) {
			return  SHUTTER_ERROR;
		}

	#else

	ret = GCI_readFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 1, "Shutter Read Status"); 
	
	if (ret < 0) {
		logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s GCI_readFAST_multiPort failed.", UIMODULE_GET_DESCRIPTION(shutter));
		return SHUTTER_ERROR;
	}
	
	#endif

	ret = (int) val[0];
	
   	*shutter_charging = ret & 0x20;
   	*shutter_fb = !(ret & 0x10); 
		
	return SHUTTER_SUCCESS;   
}


static int open_close(Shutter* shutter, int open)
{   
	unsigned char val[10];    
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter; 
	int charging, fb, warned=0;
	double time, timeout=1.0;
	
	memset(val, 0, 10); 
	
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte & 0xf7;   //Clear control bit
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte | !open << 3;	//Set control bit
	 
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	val[0] = 0x00;	    		//R/W mode
   	val[1] = 0xff;				//Write
    val[2] = atd_shutter_a_shutter->_fast_byte;  	//Set A3 
    		
	if(open) {
		
		ftdi_controller_get_lock(atd_shutter_a_shutter->controller);

		get_status(shutter, &charging, &fb);       
		
		if(fb == open) {
			ftdi_controller_release_lock(atd_shutter_a_shutter->controller); 
			return SHUTTER_SUCCESS;
		}
		
		if (charging) time=Timer();
		
		while(charging && (Timer()-time)<timeout) {
			
			if (!warned)
			{
				logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s open request whilst still charging.", UIMODULE_GET_DESCRIPTION(shutter));
				warned = 1;
			}
		
			Delay(0.1);
			
			get_status(shutter, &charging, &fb);
 		}
		
		if (charging) {
			ftdi_controller_release_lock(atd_shutter_a_shutter->controller); 
			return  SHUTTER_ERROR ;
		}
		
   		if(ftdi_controller_i2c_fastline_write_bytes(atd_shutter_a_shutter->controller, val, 3) != FT_OK) {
			ftdi_controller_release_lock(atd_shutter_a_shutter->controller);    
			return  SHUTTER_ERROR;
		}
		
		ftdi_controller_release_lock(atd_shutter_a_shutter->controller);    
	}
	else {
		
		if(ftdi_controller_i2c_fastline_write_bytes(atd_shutter_a_shutter->controller, val, 3) != FT_OK) {
			return  SHUTTER_ERROR;
		}
	}

	#else

	val[0] = 0x00;	    		//R/W mode
   	val[1] = 0xff;				//Write
    val[2] = atd_shutter_a_shutter->_fast_byte;  	//Set A3 
    		
	if(open) {
		
		GetI2CPortLock(atd_shutter_a_shutter->_com_port, "Shutter");  
	
		get_status(shutter, &charging, &fb);       
		
		if(fb == open) {
			ReleaseI2CPortLock(atd_shutter_a_shutter->_com_port, "Shutter");  
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s already open.", UIMODULE_GET_DESCRIPTION(shutter));
			return SHUTTER_SUCCESS;
		}
		
		if (charging) time=Timer();
		
		while(charging && (Timer()-time)<timeout) {
			
			if (!warned)
			{
				logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s open request whilst still charging.", UIMODULE_GET_DESCRIPTION(shutter));
				warned = 1;
			}
		
			Delay(0.1);
			
			get_status(shutter, &charging, &fb);
 		}
		
		if (charging) {
			ReleaseI2CPortLock(atd_shutter_a_shutter->_com_port, "Shutter");    
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s is still charging.", UIMODULE_GET_DESCRIPTION(shutter));
			return  SHUTTER_ERROR ;
		}
		
   		if(GCI_writeFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 3, "Shutter Open")) {    //Three bytes
			ReleaseI2CPortLock(atd_shutter_a_shutter->_com_port, "Shutter");    
			return  SHUTTER_ERROR ;
		}
		
		ReleaseI2CPortLock(atd_shutter_a_shutter->_com_port, "Shutter"); 

		#ifdef HEAVY_I2C_TESTING
		atd_shutter_a_shutter->shutter_status = 1;
		#endif
	}
	else {
		
		if(GCI_writeFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 3, "Shutter Close") < 0) {   //Three bytes
			logger_log(UIMODULE_LOGGER(shutter), LOGGER_WARNING, "%s GCI_writeFAST_multiPort failed.", UIMODULE_GET_DESCRIPTION(shutter));
			return  SHUTTER_ERROR;
		}

		#ifdef HEAVY_I2C_TESTING
		atd_shutter_a_shutter->shutter_status = 0;
		#endif
	}
	
	#endif

	return  SHUTTER_SUCCESS; 
}

static int atd_shutter_a_shutter_inhibit(Shutter* shutter, int inhibit_in)
{
	unsigned char val[10];    
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;
	int inhibit;
	
	inhibit = !inhibit_in;  // as the bit is reversed in the hardware logic
	
	memset(val, 0, 10); 
	
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte & 0xfb;   //Clear control bit
	atd_shutter_a_shutter->_fast_byte = atd_shutter_a_shutter->_fast_byte | inhibit << 2;	//Set control bit
	 
	val[0] = 0x00;	    //R/W mode 
   	val[1] = 0xff;		//Write
    val[2] = atd_shutter_a_shutter->_fast_byte; //Set A2 
    		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

		if(ftdi_controller_i2c_fastline_write_bytes(atd_shutter_a_shutter->controller, val, 3) != FT_OK) {
			return  SHUTTER_ERROR;
		}

	#else

   	if(GCI_writeFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 3, "Shutter Inhitbit"))   //Three bytes
		return  SHUTTER_ERROR;

	#endif

	atd_shutter_a_shutter->_inhibited = inhibit_in;
	return  SHUTTER_SUCCESS;  
}

static int atd_shutter_a_shutter_is_inhibited(Shutter* shutter, int *inhibit)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter; 
	*inhibit = atd_shutter_a_shutter->_inhibited;
	
	return  SHUTTER_SUCCESS;  
}

static int configure_IO(Shutter* shutter)
{
	unsigned char val[10];
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter; 
	
	memset(val, 0, 10); 
	
	val[0] = 0xf1;	    		//Configure I/O pins : 1= input 0=output	   	   
								//0xf1==A1,A2,A3 outputs ;A4,A5,B4,B5 inputs
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

		if(ftdi_controller_i2c_fastline_write_bytes(atd_shutter_a_shutter->controller, val, 1) != FT_OK) {
			return  SHUTTER_ERROR;
		}

	#else

   	if(GCI_writeFAST_multiPort(atd_shutter_a_shutter->_com_port, val, 1, "Shutter Configure"))
		return  SHUTTER_ERROR;
   	
	#endif

	return SHUTTER_SUCCESS;    
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_shutter_a_shutter_init(Shutter* shutter)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter; 
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";

	atd_shutter_a_shutter->_inhibited = 0; 
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(shutter), "FTDI_SN", device_sn);  
	
	atd_shutter_a_shutter->controller = ftdi_controller_new();

//	ftdi_controller_set_debugging(atd_shutter_a_shutter->controller, 1);
	ftdi_controller_set_error_handler(atd_shutter_a_shutter->controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_shutter_a_shutter->controller, device_sn);

	#else

	// only needs the com port the master pic is on, works directly off its fast lines.
	get_device_param_from_ini_file(UIMODULE_GET_NAME(shutter), "COM_Port", &(atd_shutter_a_shutter->_com_port));  
	
	if(initialise_comport(atd_shutter_a_shutter->_com_port, 9600) < 0)
		return SHUTTER_ERROR; 

	#endif

	if (configure_IO(shutter) == SHUTTER_ERROR)
		return SHUTTER_ERROR; 
	
	open_close(shutter, 0); // make sure shutter is supposed to be closed before un-inhibiting
	
	if (shutter_inhibit(shutter, 0) == SHUTTER_ERROR)
		return SHUTTER_ERROR; 
	
	if (shutter_set_computer_control(shutter, 1) == SHUTTER_ERROR)
		return SHUTTER_ERROR; 
	
	return SHUTTER_SUCCESS;
}


int atd_shutter_a_shutter_destroy (Shutter* shutter)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter; 
	
#ifndef FTDI_NO_VIRTUAL_COMPORT
	close_comport(atd_shutter_a_shutter->_com_port);
#endif

	return SHUTTER_SUCCESS;
}

// tried a threaded shutter timer here but there is a jitter in the thread schedule
// can also open the shutter in this same thread - no real difference, still a jitter the time the shutter is open
static int CVICALLBACK timed_shutter(void *callback)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) callback;
	Shutter *shutter = (Shutter *) atd_shutter_a_shutter;
	int ret = 0;
	
	if(atd_shutter_a_shutter->_open_time <= 0.0)
		return ret;
	
	// open_time is is milli seconds
	Delay(atd_shutter_a_shutter->_open_time / 1000.0);

	return open_close(shutter, 0); // Close the shutter
}

static int atd_shutter_a_shutter_open(Shutter* shutter)
{
	int ret;
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;       
	
	ret = open_close(shutter, 1);
	
	// Update shutter ui
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_OPEN, ATTR_DIMMED, 1);
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_CLOSE, ATTR_DIMMED, 0);
	
	if(atd_shutter_a_shutter->_open_time > 0.0) {
		timed_shutter(atd_shutter_a_shutter);   
		
		// Update shutter ui
		SetCtrlAttribute(shutter->_panel_id, SHUTTER_OPEN, ATTR_DIMMED, 0);
		SetCtrlAttribute(shutter->_panel_id, SHUTTER_CLOSE, ATTR_DIMMED, 1);
	}
		
	return ret;       
}

static int atd_shutter_a_shutter_close(Shutter* shutter)
{
  	int ret = open_close(shutter, 0);
	
	// Update shutter ui
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_OPEN, ATTR_DIMMED, 0);
	SetCtrlAttribute(shutter->_panel_id, SHUTTER_CLOSE, ATTR_DIMMED, 1);
	
	return ret;
}

static int atd_shutter_a_get_shutter_status (Shutter* shutter, int *status)
{
	int charging;
	
	return get_status(shutter, &charging, status);
}

static int atd_shutter_a_set_shutter_open_time (Shutter* shutter, double open_time)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;        
	
	atd_shutter_a_shutter->_open_time = open_time;
	
	return SHUTTER_SUCCESS;
}

static int atd_shutter_a_get_shutter_open_time (Shutter* shutter, double *open_time)
{
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;        
	
	*open_time = atd_shutter_a_shutter->_open_time;
	
	return SHUTTER_SUCCESS;
}

static int atd_shutter_a_shutter_set_computer_control  (Shutter* shutter, int compCtrl)
{
	return atd_shutter_a_shutter_set_automatic_control(shutter, compCtrl); 
}

static int atd_shutter_a_shutter_get_info (Shutter* shutter, char* info)
{
	if(info != NULL)
		strcpy(info, "Uniblitz VS25-52-ZM1");

	return SHUTTER_SUCCESS;	
}

Shutter* atd_shutter_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler)
{
	Shutter* shutter = (Shutter*) malloc(sizeof(ATD_SHUTTER_A));  
	ATD_SHUTTER_A* atd_shutter_a_shutter = (ATD_SHUTTER_A *) shutter;    
	
	shutter_constructor(shutter, name, description);

	ui_module_set_error_handler(UIMODULE_CAST(shutter), handler, NULL);   
	atd_shutter_a_shutter->_open_time = -1;   
	
	SHUTTER_VTABLE_PTR(shutter, hw_init) = atd_shutter_a_shutter_init; 
	SHUTTER_VTABLE_PTR(shutter, destroy) = atd_shutter_a_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = atd_shutter_a_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = atd_shutter_a_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = atd_shutter_a_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = atd_shutter_a_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = atd_shutter_a_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_inhibit) = atd_shutter_a_shutter_inhibit; 
	SHUTTER_VTABLE_PTR(shutter, shutter_is_inhibited) = atd_shutter_a_shutter_is_inhibited; 
	SHUTTER_VTABLE_PTR(shutter, shutter_set_computer_control) = atd_shutter_a_shutter_set_computer_control; 
	SHUTTER_VTABLE_PTR(shutter, shutter_get_info) = atd_shutter_a_shutter_get_info; 
	
	return shutter;
}
