#include "HardWareTypes.h" 

#include "scanner.h"
#include "ScannerUI.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "toolbox.h"

#include "asynctmr.h"
#include "ThreadDebug.h"
#include "profile.h"

#include <ansi_c.h> 
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April/May 2007
//GCI HTS Microscope system. 
//Scanner control.
////////////////////////////////////////////////////////////////////////////

#define Round  RoundRealToNearestInteger

static char scanner_speed_names[7][16] = {"fastest", "very fast", "fast", "normal", "slow", "very slow", "slowest"};

int send_scanner_error_text (Scanner* scanner, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(scanner), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(scanner), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(scanner), "Scanner error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)     
{
	GCI_MessagePopup("Scanner Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static void scanner_read_or_write_main_panel_registry_settings(Scanner *scanner, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(scanner), write);   
}


static int SCANNER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Scanner*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Scanner *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int SCANNER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Scanner*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Scanner *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int SCANNER_PTR_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Scanner*, int, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Scanner *) args[0].void_ptr_data, (int) args[1].int_data, (int) args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void scanner_on_change(Scanner* scanner, int control)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerChanged", GCI_VOID_POINTER, scanner, GCI_INT, control);  
}

int scanner_set_init_values(Scanner* scanner)
{
	PROFILE_START("load_microscope_settings - scanner_set_speed");

	if (scanner_set_speed(scanner, scanner->_speed, scanner->_hyst_offset) == SCANNER_ERROR) {
		PROFILE_STOP("load_microscope_settings - scanner_set_speed");
		return SCANNER_ERROR;	
	}
	PROFILE_STOP("load_microscope_settings - scanner_set_speed");
	
	PROFILE_START("load_microscope_settings - scanner_set_resolution");

	if (scanner_set_resolution(scanner, scanner->_resolution) == SCANNER_ERROR) {
		PROFILE_STOP("load_microscope_settings - scanner_set_resolution");
		return SCANNER_ERROR;	  
	}

	PROFILE_STOP("load_microscope_settings - scanner_set_resolution");

	PROFILE_START("load_microscope_settings - scanner_stop_scan");

	if (scanner_stop_scan(scanner) == SCANNER_ERROR) {	  //Make sure scanning has stopped 
	
		PROFILE_STOP("load_microscope_settings - scanner_stop_scan");
		return SCANNER_ERROR;
	}
	
	PROFILE_STOP("load_microscope_settings - scanner_stop_scan");

	PROFILE_START("load_microscope_settings - scanner_set_zoom");

	if (scanner_set_zoom(scanner, scanner->_zoom) == SCANNER_ERROR) {
		PROFILE_STOP("load_microscope_settings - scanner_set_zoom");	
		return SCANNER_ERROR;	  
	}

	PROFILE_STOP("load_microscope_settings - scanner_set_zoom");

	// We reset the x and y shifts to the center on each new run of the app.
	// Otherwise the offsets get applied to the last run value and the scanner image looks
	// misplaced.
	scanner_set_shifts_to_centre(scanner);
	
	PROFILE_START("load_microscope_settings - scanner_reverse_scan");

	if (scanner_reverse_scan(scanner, scanner->_reverse_scan) == SCANNER_ERROR) {
		PROFILE_STOP("load_microscope_settings - scanner_reverse_scan");
		return SCANNER_ERROR;	  
	}

	PROFILE_STOP("load_microscope_settings - scanner_reverse_scan");
	
	PROFILE_START("load_microscope_settings - scanner_line_scan");

	if (scanner_line_scan(scanner, scanner->_line_scan) == SCANNER_ERROR) {
		PROFILE_STOP("load_microscope_settings - scanner_line_scan");
		return SCANNER_ERROR;	  
	}

	PROFILE_STOP("load_microscope_settings - scanner_line_scan");

	if (scanner_select_clock(scanner, scanner->_pixel_clock, scanner->_line_clock, scanner->_frame_clock) == SCANNER_ERROR)
		return SCANNER_ERROR;	  

	return SCANNER_SUCCESS;
}		


int scanner_save_settings(Scanner* scanner, const char *filename, const char *flags)
{
	if(filename == NULL)
    	return SCANNER_ERROR;

	if(SCANNER_VTABLE(scanner, save_settings) == NULL) {
    	send_scanner_error_text(scanner,"Save settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(scanner));
    	return SCANNER_ERROR;
  	}

  	if( (*scanner->vtable.save_settings)(scanner, filename, flags)  == SCANNER_ERROR) {
  		send_scanner_error_text(scanner,"Can not save settings for device %s\n", UIMODULE_GET_DESCRIPTION(scanner));
  		return SCANNER_ERROR;
  	}
	
	return SCANNER_SUCCESS;
}


int scanner_load_settings(Scanner* scanner, const char *filename)
{
	if(filename == NULL)
    	return SCANNER_ERROR;

	if(SCANNER_VTABLE(scanner, load_settings) == NULL) {
    	send_scanner_error_text(scanner,"Load settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(scanner));
    	return SCANNER_ERROR;
  	}

  	if( (*scanner->vtable.load_settings)(scanner, filename)  == SCANNER_ERROR)
  		return SCANNER_ERROR;
	
	return SCANNER_SUCCESS;
}

int scanner_save_settings_as_default(Scanner* scanner)
{
	char path[GCI_MAX_PATHNAME_LEN];

    sprintf(path, "%s\\%s", UIMODULE_GET_DATA_DIR(scanner), scanner->_global_settings_file);

	if(scanner_save_settings(scanner, path, "w") == SCANNER_ERROR)
		return SCANNER_ERROR;
	
	return SCANNER_SUCCESS;
}


int scanner_load_default_settings(Scanner* scanner)
{
	char path[GCI_MAX_PATHNAME_LEN] = "";

    sprintf(path, "%s\\%s", UIMODULE_GET_DATA_DIR(scanner), scanner->_global_settings_file);
	
	if(scanner_load_settings(scanner, path) == SCANNER_ERROR)
		return SCANNER_ERROR;
		
	return SCANNER_SUCCESS;
}

int scanner_signal_hide_handler_connect (Scanner* scanner, SCANNER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Hide signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}


int scanner_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_INT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "ScannerChanged", handler, data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Change signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}

int scanner_zoomed_changed_handler_connect(Scanner* scanner, SCANNER_ZOOM_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomChanged", handler, data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Zoom Change signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}

int scanner_zoomed_pre_changed_handler_connect(Scanner* scanner, SCANNER_ZOOM_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomPreChanged", handler, data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Zoom Pre Change signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}

int scanner_shift_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "ScannerShiftChanged", handler, data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Shift Change signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}

int scanner_resolution_pre_changed_handler_connect(Scanner* scanner, SCANNER_EVENT_INT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(scanner), "ScannerResolutionPreChanged", handler, data) == SIGNAL_ERROR) {
		send_scanner_error_text(scanner, "Can not connect signal handler for Scanner Resolution Pre Change signal");
		return SCANNER_ERROR;
	}

	return SCANNER_SUCCESS;
}

int scanner_hardware_initialise(Scanner* scanner)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_SCANNER_VTABLE_PTR(scanner, hw_init) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.hw_init)(scanner) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_initialise failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				scanner->_initialised = 0;  
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	scanner->_initialised = 1;	

	return SCANNER_SUCCESS;   
}

int scanner_initialise(Scanner* scanner)
{
	scanner_load_default_settings(scanner);  
	
	return SCANNER_SUCCESS;   
}

int scanner_is_hardware_initialised(Scanner* scanner)
{
	return scanner->_initialised;	
}

static void OnScannerPanelsClosedOrHidden (UIModule *module, void *data)
{
	Scanner* scanner = (Scanner*) data; 

	scanner_disable_timer(scanner);
}

static void OnScannerPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	Scanner* scanner = (Scanner*) data; 

	scanner_enable_timer(scanner);
}

Scanner* scanner_new(char *name, char *description, const char *data_dir, const char *data_file, size_t size)
{
	Scanner* scanner = (Scanner*) malloc(size);
	
	memset(scanner, 0, sizeof(Scanner));
	
	strncpy(scanner->_global_settings_file, data_file, 99);

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(scanner), name);
	ui_module_set_description(UIMODULE_CAST(scanner), description);
	ui_module_set_data_dir(UIMODULE_CAST(scanner), data_dir);   
	
	scanner->_initialised = 0;

	scanner->_timer = -1;

	scanner->_speed = NORMAL_SCAN;
	scanner->_hyst_offset = DEFAULT_HYST_OFFSET;
	scanner->_resolution = 256;
	scanner->_zoom = SCANNER_ZOOM_X1;
	scanner->_x_shift = DEFAULT_SHIFT;
	scanner->_y_shift = DEFAULT_SHIFT;
	scanner->_pixel_clock = INTERNAL_CLOCK;
	scanner->_line_clock = INTERNAL_CLOCK;
	scanner->_frame_clock = INTERNAL_CLOCK;
	scanner->_reverse_scan = SCAN_DIR_NORMAL;
	scanner->_scan_disable = 1;
	scanner->_line_scan = 0;
	scanner->_frames = 0;
	scanner->_scanning = 0;
	scanner->_servo_error = 0;
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "Hide", SCANNER_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "ScannerChanged", SCANNER_PTR_INT_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomPreChanged", SCANNER_PTR_INT_INT_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "ScannerResolutionPreChanged", SCANNER_PTR_INT_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomChanged", SCANNER_PTR_INT_INT_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(scanner), "ScannerShiftChanged", SCANNER_PTR_MARSHALLER);

	scanner_set_error_handler(scanner, default_error_handler, NULL);

	scanner->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(scanner), "ScannerUI.uir", SCAN_PNL, 1);   
	scanner->_cal_ui_panel  = ui_module_add_panel(UIMODULE_CAST(scanner), "ScannerUI.uir", SCNCALPNL, 0);  

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(scanner), OnScannerPanelsClosedOrHidden, scanner);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(scanner), OnScannerPanelsDisplayed, scanner);
	
    if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_START_SCAN, cbstart_scan, scanner) < 0)
		return NULL;
  	
  	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_STOP_SCAN, cbstop_scan, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, cbframe_num, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_ZOOM, cbzoom, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_SPEED, cbspeed, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_RESOLUTION, cbresolution, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_SHIFT_RESET, cbshift_reset, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_QUIT, cbquit_scan, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_X_SHIFT, cbx_shift, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_Y_SHIFT, cby_shift, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, cbscan_disable, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_LINE_SCAN, cbline_scan, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_SAVE, cbsave_scan, scanner) < 0)
		return NULL;      	
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_DEFINE, cbdefine_scan, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_LOAD, cbload_scan, scanner) < 0)
		return NULL;      
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, SCAN_PNL_TIMER, OnScannerTimerTick, scanner) < 0)
		return NULL;      
		
	if(scanner->_cal_ui_panel != 0) {

		if ( InstallCtrlCallback (scanner->_cal_ui_panel,  SCNCALPNL_CLOSE, cbcalclose_scan, scanner) < 0)
			return NULL;      	
		
		if ( InstallCtrlCallback (scanner->_cal_ui_panel,  SCNCALPNL_HYST_OFFSET, cbhyst_offset, scanner) < 0)
			return NULL;      	
		
		if ( InstallCtrlCallback (scanner->_cal_ui_panel, SCNCALPNL_REV_SCAN, cbrev_scan, scanner) < 0)
			return NULL;      	
		
		if ( InstallCtrlCallback (scanner->_cal_ui_panel, SCNCALPNL_FRAMECLK, cbselect_clock, scanner) < 0)
			return NULL;      	
		
		if ( InstallCtrlCallback (scanner->_cal_ui_panel, SCNCALPNL_LINECLK, cbselect_clock, scanner) < 0)
			return NULL;      
		
		if ( InstallCtrlCallback (scanner->_cal_ui_panel, SCNCALPNL_PIXCLK, cbselect_clock, scanner) < 0)
			return NULL;      	

		if ( InstallCtrlCallback (scanner->_cal_ui_panel,  SCNCALPNL_X_OFFSET, OnOffsetChanged, scanner) < 0)
			return NULL;      	

		if ( InstallCtrlCallback (scanner->_cal_ui_panel,  SCNCALPNL_Y_OFFSET, OnOffsetChanged, scanner) < 0)
			return NULL;      	
	}

	ui_module_set_main_panel_title (UIMODULE_CAST(scanner));
	
	#ifdef SINGLE_THREADED_POLLING
	
	scanner->_timer = NewCtrl(scanner->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
	if ( InstallCtrlCallback (scanner->_main_ui_panel, scanner->_timer, OnScannerTimerTick, scanner) < 0)
		return NULL;      
		
	SetCtrlAttribute(scanner->_main_ui_panel, scanner->_timer, ATTR_INTERVAL, 4.0);  
	SetCtrlAttribute(scanner->_main_ui_panel, scanner->_timer, ATTR_ENABLED, 0);
		
	#else
	
	scanner->_timer = NewAsyncTimer (2.0, -1, 0, OnScannerTimerTick, scanner);
	SetAsyncTimerName(scanner->_timer, "Scanner");

	#endif
	
	GciCmtNewLock ("Scanner", 0, &(scanner->_lock) );  

	return scanner;
}


int scanner_destroy(Scanner* scanner)
{
	scanner_disable_timer(scanner);
	
	#ifndef SINGLE_THREADED_POLLING
	DiscardAsyncTimer(scanner->_timer);
	#endif

	scanner_disable_scanner(scanner, 1);
	scanner_stop_scan(scanner);

	CHECK_SCANNER_VTABLE_PTR(scanner, destroy) 
	CALL_SCANNER_VTABLE_PTR(scanner, destroy) 
	
	ui_module_destroy(UIMODULE_CAST(scanner));
		
  	CmtDiscardLock (scanner->_lock);
  	
  	free(scanner);
  	
  	return SCANNER_SUCCESS;
}


int scanner_select_clock(Scanner* scanner, int pixclk, int lineclk, int frameclk)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_select_clock)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_select_clock)(scanner, pixclk, lineclk, frameclk) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_select_clock failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_load_line_data(Scanner* scanner, int line)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_load_line_data)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_load_line_data)(scanner, line) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_load_line_data failed");  
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_set_rep_rate(Scanner* scanner, int rep_val,int rep_div_1)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_rep_rate)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_rep_rate)(scanner, rep_val, rep_div_1) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_rep_rate failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_set_speed(Scanner* scanner, int speed,int hyst_offset)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_speed)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_speed)(scanner, speed, hyst_offset) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_speed failed");        
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	scanner->_speed = speed;

	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SPEED, scanner->_speed);
	
	scanner_on_change(scanner, SCAN_PNL_SPEED);					 

	return SCANNER_SUCCESS;
}

int scanner_get_speed(Scanner* scanner, int *speed)
{
	*speed = scanner->_speed;

	return SCANNER_SUCCESS;
}

int scanner_set_resolution(Scanner* scanner, int resolution)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_resolution)

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerResolutionPreChanged", GCI_VOID_POINTER, scanner,
		GCI_INT, resolution);     // pass the resolution we are changing to

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_resolution)(scanner, resolution) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_resolution failed");               
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	scanner->_resolution = resolution;

	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_RESOLUTION, scanner->_resolution);

	scanner_on_change(scanner, SCAN_PNL_RESOLUTION);	

	return SCANNER_SUCCESS;
}

int scanner_get_resolution_index(Scanner* scanner, int *resolution_index)
{
	*resolution_index =  scanner->_resolution;

	return SCANNER_SUCCESS;
}

int scanner_get_resolution(Scanner* scanner)
{
	return (scanner->_resolution);
}

int scanner_get_hyst_offset(Scanner* scanner, int *offset)
{
	*offset = scanner->_hyst_offset;

	return SCANNER_SUCCESS;
}

int scanner_get_spc_left_position (Scanner *scanner, int res, int *pos)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_spc_left_position)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_get_spc_left_position)(scanner, res, pos) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_get_spc_left_position failed");                         
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_reset_shifts(Scanner *scanner)
{
	//Set scanner offsets to mid range
	scanner->_x_shift = DEFAULT_SHIFT;
	scanner->_y_shift = DEFAULT_SHIFT;
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_X_SHIFT, scanner->_x_shift);  
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_Y_SHIFT, scanner->_y_shift); 
	scanner_set_x_shift(scanner, scanner->_x_shift);
	scanner_set_y_shift(scanner, scanner->_y_shift);

	return SCANNER_SUCCESS;
}

int scanner_set_zoom(Scanner* scanner, int zoom)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_zoom)

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomPreChanged", GCI_VOID_POINTER, scanner,
		GCI_INT,  scanner->_zoom, GCI_INT,  zoom);  
	
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_zoom)(scanner, zoom) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_zoom failed");                 
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerZoomChanged", GCI_VOID_POINTER, scanner,
		GCI_INT,  scanner->_zoom, GCI_INT,  zoom);  
	
	scanner->_zoom = zoom;
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_ZOOM, scanner->_zoom);
		
	if(zoom == SCANNER_ZOOM_X1) {	// zoom x1

		scanner_reset_shifts(scanner);
	}

	scanner_on_change(scanner, SCAN_PNL_ZOOM);

	return SCANNER_SUCCESS;
}

int scanner_get_zoom_index(Scanner* scanner, int *zoom)
{
	*zoom = scanner->_zoom;

	return SCANNER_SUCCESS;
}

int scanner_get_zoom(Scanner* scanner)
{
	return (scanner->_zoom);
}

int scanner_get_line_scan(Scanner* scanner)
{
	return scanner->_line_scan;  //1=line scan, 0=xy scan
}

int scanner_set_shifts_to_centre(Scanner* scanner)
{
	int shift, min, max;

	GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_X_SHIFT, &shift);

	scanner_get_min_max_x_shift(scanner, &min, &max);
	if(scanner_set_x_shift(scanner, (max - min) / 2) == SCANNER_ERROR)
		return SCANNER_ERROR;

	scanner_get_min_max_y_shift(scanner, &min, &max);
	if(scanner_set_y_shift(scanner, (max - min) / 2) == SCANNER_ERROR)
		return SCANNER_ERROR;

	return SCANNER_SUCCESS;
}

int scanner_set_x_shift(Scanner* scanner, int x_shift)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_x_shift)

	x_shift -= scanner->_x_offset;

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_x_shift)(scanner, x_shift) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_x_shift failed");                
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_X_SHIFT, scanner->_x_shift);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerShiftChanged", GCI_VOID_POINTER, scanner);  
	
	scanner_on_change(scanner, SCAN_PNL_X_SHIFT);

	return SCANNER_SUCCESS;
}

int scanner_get_x_shift(Scanner* scanner)
{
	return scanner->_x_shift;
}

int scanner_get_y_shift(Scanner* scanner)
{
	return scanner->_y_shift;
}

int scanner_set_y_shift(Scanner* scanner, int y_shift)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_y_shift)

	y_shift -= scanner->_y_offset;

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_y_shift)(scanner, y_shift) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_y_shift failed");                      
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_Y_SHIFT, scanner->_y_shift);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "ScannerShiftChanged", GCI_VOID_POINTER, scanner);  

	scanner_on_change(scanner, SCAN_PNL_Y_SHIFT);

	return SCANNER_SUCCESS;
}

int scanner_get_min_max_x_shift (Scanner *scanner, int *min, int *max)
{
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_x_shift)

	return (*scanner->vtable.scanner_get_min_max_x_shift)(scanner, min, max);
}

int scanner_get_min_max_y_shift (Scanner *scanner, int *min, int *max)
{
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_y_shift)

	return (*scanner->vtable.scanner_get_min_max_y_shift)(scanner, min, max);
}

int scanner_reverse_scan(Scanner* scanner, int direction)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_reverse_scan)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_reverse_scan)(scanner, direction) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_reverse_scan failed");                       
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_line_scan(Scanner* scanner, int enable)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_line_scan)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_line_scan)(scanner, enable) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_line_scan failed");                        
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_LINE_SCAN, scanner->_line_scan);

	scanner_on_change(scanner, SCAN_PNL_LINE_SCAN);					

	return SCANNER_SUCCESS;
}

int scanner_set_no_frames(Scanner* scanner, int frames)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_set_no_frames)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_set_no_frames)(scanner, frames) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_set_no_frames failed");                       
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	scanner->_frames = frames;
	
	return SCANNER_SUCCESS;
}

int scanner_get_frame_time(Scanner* scanner, double *frame_time)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_frame_time)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_get_frame_time)(scanner, frame_time) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_get_frame_time failed");                         
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_get_line_time(Scanner* scanner, double *time)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_line_time)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_get_line_time)(scanner, time) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_get_line_time failed");                         
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_get_pixel_time(Scanner* scanner, double *time)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_get_pixel_time)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_get_pixel_time)(scanner, time) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_get_pixel_time failed");                         
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_start_scan(Scanner* scanner, int frames)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_start_scan)

	logger_log(UIMODULE_LOGGER(scanner), LOGGER_INFORMATIONAL, "%s On", UIMODULE_GET_DESCRIPTION(scanner));

	if(frames >= 0) {
		scanner->_frames = frames;
		SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, scanner->_frames);  
	}
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_start_scan)(scanner, scanner->_frames) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_start_scan failed");                      
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	if (scanner->_scanning == 1)
		return SCANNER_SUCCESS;		   //already scanning
	
	scanner->_scanning = 1;

	return SCANNER_SUCCESS;
}

int scanner_stop_scan(Scanner* scanner)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_stop_scan)

	logger_log(UIMODULE_LOGGER(scanner), LOGGER_INFORMATIONAL, "%s Off", UIMODULE_GET_DESCRIPTION(scanner));

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_stop_scan)(scanner) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_scanner_stop_scan failed");                     
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, 0);
	scanner->_scanning = 0;

	return SCANNER_SUCCESS;
}

int scanner_disable_scanner(Scanner* scanner, int disable)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_disable_scanner)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_disable_scanner)(scanner, disable) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_disable_scanner failed");                    
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	if (disable == 1)
		scanner_stop_scan(scanner);

	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, scanner->_scan_disable);

	return SCANNER_SUCCESS;
}

int scanner_save_settings_to_EEPROM(Scanner* scanner)
{
	int status = UIMODULE_ERROR_NONE;   
	
	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_save_settings_to_EEPROM)

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*scanner->vtable.scanner_save_settings_to_EEPROM)(scanner) == SCANNER_ERROR ) {
			status = send_scanner_error_text(scanner, "scanner_save_settings_to_EEPROM failed");                       
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return SCANNER_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	return SCANNER_SUCCESS;
}

int scanner_read_error_signal(Scanner* scanner, int *scannerServoError)
{
	const int max_errors = 10;

	CHECK_SCANNER_VTABLE_PTR(scanner, scanner_read_error_signal)

	if( (*scanner->vtable.scanner_read_error_signal)(scanner, scannerServoError) == SCANNER_ERROR ) {

		if(hardware_device_get_number_of_consecutive_errors(HARDWARE_DEVICE_CAST(scanner)) >= max_errors) {
			return SCANNER_ERROR;
		}

		// This function is continously called in a loop.
		// Lets not proprogate the error up to the user.
		// We will log in and it will probablt correct itself.
		logger_log(UIMODULE_LOGGER(scanner), LOGGER_ERROR, "scanner_read_error_signal failed");
		hardware_device_increment_consecutive_errors(HARDWARE_DEVICE_CAST(scanner));
		return SCANNER_ERROR;
	}
			
	hardware_device_reset_consecutive_errors(HARDWARE_DEVICE_CAST(scanner));

	return SCANNER_SUCCESS;
}

void scanner_set_error_handler(Scanner* scanner,
	UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	ui_module_set_error_handler(UIMODULE_CAST(scanner), handler, callback_data);	
}


void scanner_disable_timer(Scanner* scanner)
{
	#ifdef ENABLE_SCANNER_STATUS_POLLING
		#ifdef MT
			SetAsyncTimerAttribute (scanner->_timer, ASYNC_ATTR_ENABLED,  0);
		#else
			SetCtrlAttribute (scanner->_main_ui_panel, SCAN_PNL_TIMER, ATTR_ENABLED, 0);
		#endif	
	#endif
}

void scanner_enable_timer(Scanner* scanner)
{
	#ifdef ENABLE_SCANNER_STATUS_POLLING
		#ifdef MT
			SetAsyncTimerAttribute (scanner->_timer, ASYNC_ATTR_ENABLED,  1);
		#else
			SetCtrlAttribute (scanner->_main_ui_panel, SCAN_PNL_TIMER, ATTR_ENABLED, 1);
		#endif	
	#endif
}

int scanner_display_main_ui(Scanner* scanner)
{
	ui_module_display_main_panel(UIMODULE_CAST(scanner));   
	
	return SCANNER_SUCCESS;
}


int scanner_hide_main_ui(Scanner* scanner)
{
	ui_module_hide_main_panel(UIMODULE_CAST(scanner));

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(scanner), "Hide", GCI_VOID_POINTER, scanner); 

	return SCANNER_SUCCESS;
}


int scanner_is_main_ui_visible(Scanner* scanner)
{
	return ui_module_main_panel_is_visible(UIMODULE_CAST(scanner));   
}

						   
dictionary* scanner_add_metadata(Scanner* scanner, dictionary *d)
{
	double pixel_time;

	scanner_get_pixel_time(scanner, &pixel_time);
	
	dictionary_setint(d, "Scanner Zoom", scanner->_zoom);       
	dictionary_set   (d, "Scanner Speed", scanner_speed_names[scanner->_speed]);      
	dictionary_setint(d, "Scanner Resolution", scanner->_resolution);       
	dictionary_setint(d, "Scanner XShift", scanner->_x_shift);       
	dictionary_setint(d, "Scanner YShift", scanner->_y_shift);   
	dictionary_setdouble(d, "Scanner Pixel Time (s)", pixel_time);   
	dictionary_setint(d, "Scanner Lag", scanner->_hyst_offset);  

	return d;
}
