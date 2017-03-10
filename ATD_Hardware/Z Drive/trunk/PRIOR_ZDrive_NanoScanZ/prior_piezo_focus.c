#include "prior_piezo_focus.h"
#include "ZDriveUI.h" 
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

#include "ThreadDebug.h"

#define RS232_ARRAY_SIZE 100

static int GCI_ComWrt(int COMPort, char buffer[], int count)
{
	int ret;
	char read_data[RS232_ARRAY_SIZE]="";

	ret = ComWrt(COMPort, buffer, count);    
	
	if (ret < 0)
		return ret;
	
	while(GetOutQLen(COMPort)>0) {
		Delay(0.001);
		continue;
	}
	
	return ret;
}

int RS232_SendString(Z_DrivePrior* prior_zd, char* fmt, ...) 
{
    char buffer[1000];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);    
    
    GciCmtGetLock (prior_zd->_lock);					 //for multi-threading
    
	#ifdef STAGE_DEBUG
        printf(buffer);
    #endif
		
	// flush the Qs
	FlushInQ(prior_zd->_com_port);
	FlushOutQ(prior_zd->_com_port);	

	// send string 
	if (GCI_ComWrt(prior_zd->_com_port, buffer, strlen(buffer)) <= 0) {
		GciCmtReleaseLock (prior_zd->_lock);
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error"); 	
		return -1;
	}
   
	GciCmtReleaseLock (prior_zd->_lock);
	
	return 0;
}		

int RS233_ReadString(Z_DrivePrior* prior_zd, int number_of_characters, char *retval)
{
	int no_bytes_read;
	char read_data[RS232_ARRAY_SIZE]="";

	// Reads the Stage Port and returns a string
	// Returned character string is always terminated with CR, (ASCII 13)
	// will return 0 if read is successful, -1 otherwise
	GciCmtGetLock (prior_zd->_lock);					

	memset(retval, 0, RS232_ARRAY_SIZE);  
	memset(read_data, 0, RS232_ARRAY_SIZE);
	
	no_bytes_read = ComRdTerm(prior_zd->_com_port, read_data, number_of_characters * 8, 13);  
	
	if (no_bytes_read <= 0) {
		GciCmtReleaseLock (prior_zd->_lock);	
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error"); 	
		return -1;
	}
	
	if(strcmp(read_data, "E,8\n") == 0) {
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error: Value out of range"); 
		GciCmtReleaseLock (prior_zd->_lock);
		return -1;
	}
	else if(strcmp(read_data, "E,4\n") == 0) {
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error: Command pass error"); 
		GciCmtReleaseLock (prior_zd->_lock);
		return -1;
	}
	else if(strcmp(read_data, "E,5\n") == 0) {
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error: Unknown command"); 
		GciCmtReleaseLock (prior_zd->_lock);
		return -1;
	}
	else if(strcmp(read_data, "E,21\n") == 0) {
		logger_log(UIMODULE_LOGGER(prior_zd), LOGGER_ERROR, "Prior ZDrive Stage Controller Error: Invalid checksum"); 
		GciCmtReleaseLock (prior_zd->_lock);
		return -1;
	}
	
	strcpy(retval, read_data);
	
	GciCmtReleaseLock (prior_zd->_lock);
	
	return 0;
}

int initPriorRS232Port(Z_DrivePrior* prior_zd)
{
	int err, attempts=0, baud=9600;
	int parity=0, dataBits=8, stopBits=1, inputQueueSize=164, outputQueueSize=164;
	char retval[RS232_ARRAY_SIZE] = "";
	
	GciCmtGetLock (prior_zd->_lock);   
	
	while (attempts < 2) {
  
		CloseCom(prior_zd->_com_port);	//In case it was open
		err = OpenComConfig (prior_zd->_com_port, NULL, baud, 0, 8, 1, 512, 512);
		
		//RS232_SendString(prior_zd, "mode 0\n");	  // Set "host" mode. "terminal" mode is for use with Hyperterminal only.
	
		// Get the version number from the controller
    	if (RS232_SendString(prior_zd, "VER\r") == 0) {
			
			RS233_ReadString(prior_zd, 3, retval);   
				
    		if (strcmp(retval, ""))
				break;	//success
		}
		
		attempts ++;
	}
	
	GciCmtReleaseLock (prior_zd->_lock); 
	
	if (attempts > 1) 
		return -1;	// Failed
	
	return 0;
}


static int prior_focus_get_position(Z_Drive* zd, double *focus_microns)
{
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd;     
	char retval[RS232_ARRAY_SIZE] = "";   
	int val;
	
	int err =  RS232_SendString(prior_zd, "PZ\r");

	if (err)
        return Z_DRIVE_ERROR;
		
	RS233_ReadString(prior_zd, 4, retval);       
	
	sscanf(retval, "%d", &val);
	
	*focus_microns = (double) val + zd->_min_microns;
	
	return Z_DRIVE_SUCCESS;
}

static int prior_focus_set_position(Z_Drive* zd, double focus_microns)
{
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd;     

	int err =  RS232_SendString(prior_zd, "V %d\r", (int) (focus_microns + fabs(zd->_min_microns)));

	if (err)
        return Z_DRIVE_ERROR;
	
	return Z_DRIVE_SUCCESS;
}

static int prior_focus_get_min_max_in_microns(Z_Drive* zd, int* min_microns, int* max_microns)
{
	*min_microns = (int) zd->_min_microns;
	*max_microns = (int) zd->_max_microns; 
	
	return Z_DRIVE_SUCCESS; 	
}

static int prior_focus_hardware_init(Z_Drive* zd)
{
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd;
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(zd), device);
	 
	get_device_int_param_from_ini_file   (device, "COM_Port", &(prior_zd->_com_port));  
	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));
	get_device_double_param_from_ini_file(device, "Min Microns", &(zd->_min_microns));
	get_device_double_param_from_ini_file(device, "Max Microns", &(zd->_max_microns));
	get_device_double_param_from_ini_file(device, "Speed", &(zd->_speed));

	prior_zd->_range =  zd->_max_microns - zd->_min_microns;
	
	initPriorRS232Port(prior_zd);
	
	z_drive_hide_autofocus_controls(zd);  
	
	return Z_DRIVE_SUCCESS;  
}


static int prior_focus_init(Z_Drive* zd)
{
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd; 
	
	//prior_zd->monitor_timer = NewAsyncTimer (0.5, -1, 0, OnTimerTick, zd);
	//SetAsyncTimerAttribute (prior_zd->monitor_timer, ASYNC_ATTR_ENABLED,  1);

	return Z_DRIVE_SUCCESS;   
}


int prior_focus_destroy (Z_Drive* zd)
{
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd;    

//	StoreAutofocusSettings(autofocusCtrl);	 	 //Save to disc

//	autofocus_read_or_write_panel_registry_settings(autofocusCtrl, autofocusCtrl->_autofocus_ui_pnl, 1);

	CmtDiscardLock(prior_zd->_lock);
  	
	return Z_DRIVE_SUCCESS;  
}


Z_Drive* prior_zdrive_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Z_Drive* zd = (Z_Drive*) malloc(sizeof(Z_DrivePrior));  
	Z_DrivePrior* prior_zd = (Z_DrivePrior *) zd;    
	
	z_drive_constructor(zd, name, description, data_dir);

	GciCmtNewLock("PiezoFocus", 0, &(prior_zd->_lock));
	
	Z_DRIVE_VTABLE_PTR(zd, hw_initialise) = prior_focus_hardware_init; 
	Z_DRIVE_VTABLE_PTR(zd, initialise) = prior_focus_init; 
	Z_DRIVE_VTABLE_PTR(zd, destroy) = prior_focus_destroy; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position) = prior_focus_set_position; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position) = prior_focus_get_position;  
	
	return zd;
}
