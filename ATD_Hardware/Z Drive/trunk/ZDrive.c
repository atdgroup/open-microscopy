#include "HardWareTypes.h" 

#include "ZDrive.h"
#include "ZDriveUI.h" 
#include "string_utils.h"
#include "gci_utils.h"
#include "iniparser.h"  

#include "GL_CVIRegistry.h"

#include <utility.h>
#include "toolbox.h"

#include "asynctmr.h"

#include "ThreadDebug.h"

#include <ansi_c.h> 

void zdrive_get_lock(Z_Drive* zd, const char *name)
{
//	if(name != NULL)
//		printf ("zdrive_get_lock - %s\n", name);

	if(GciCmtGetLock(zd->_lock) < 0) {
//		printf("Error Getting Lock");
		return;
	}
}

void zdrive_release_lock(Z_Drive* zd, const char *name)
{
	int err;
	
//	if(name != NULL)
//		printf ("zdrive_release_lock - %s\n", name);

	if((err = GciCmtReleaseLock (zd->_lock)) < 0) {
		 return;
	}
}

int send_z_drive_error_text (Z_Drive* zd, char fmt[], ...)
{
	int ret=0;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(zd), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(zd), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(zd), "Z Drive Error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}

static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("Z Drive Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static int Z_DRIVE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Z_Drive*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Z_Drive *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int Z_DRIVE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Z_Drive*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Z_Drive *) args[0].void_ptr_data, (int) args[0].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

void emit_if_z_drive_changed(Z_Drive* zd)
{   // commit indicates whether the user has finished moving the slider, or has pressed a button
	double focus=0;
	
	if(zd->_prevent_change_signal_emit)
		return;

	if (z_drive_get_position(zd, &focus) == Z_DRIVE_ERROR) 
		return;
	
   	if (focus != zd->_prev_focus) 
		z_drive_emit_change_signal(zd, 0);

	zd->_prev_focus = focus;
}

void zdrive_use_cached_data_for_read(Z_Drive* zd, int val)
{
	zd->_use_cached_position = val;
}

static void z_drive_save_to_default_file(Z_Drive* zd)
{
	char path[GCI_MAX_PATHNAME_LEN];  
	
	sprintf(path, "%s\\%s.ini", zd->_data_dir, UIMODULE_GET_NAME(zd));   

	z_drive_save_settings_in_ini_fmt (zd, path);    	
}

static void z_drive_load_from_default_file(Z_Drive* zd)
{
	char path[GCI_MAX_PATHNAME_LEN];  
	
	sprintf(path, "%s\\%s.ini", zd->_data_dir, UIMODULE_GET_NAME(zd));   

	z_drive_load_settings_from_ini_fmt (zd, path);    	
}


static int CVICALLBACK OnZ_Drive_FocusChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive*zd = (Z_Drive*) callbackData;    
	
			double microns_val;
		
			GetCtrlVal(panel, control, &microns_val);
			z_drive_set_position(zd, microns_val); 
			 
			z_drive_wait_for_stop_moving (zd, 1);   // timeout 1 s  
			
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 1, GCI_VOID_POINTER, zd); 

			break;
		}

		case EVENT_VAL_CHANGED:
		{
			Z_Drive*zd = (Z_Drive*) callbackData;    
	
			double microns_val;

			if(!zd->_respond_to_event_val_changed)
				return 0;
				
			GetCtrlVal(panel, control, &microns_val);
			z_drive_set_position_no_commit(zd, microns_val);

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 0, GCI_VOID_POINTER, zd); 

			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK OnZ_Drive_FocusClosed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    
	
			//z_drive_save_to_default_file(zd); 
			
			ui_module_hide_all_panels(UIMODULE_CAST(zd));        
			
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnZ_Drive_GotoZero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    
	
			if(zd->_centre_position_set == 1) {

				z_drive_set_position(zd, zd->_centre_position);
			}
			else {

				// Goto center
				double center = zd->_max_microns - (zd->_max_microns - zd->_min_microns) / 2.0;

				z_drive_set_position(zd, center);
			}
			
			z_drive_emit_change_signal(zd, 1);

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 1, GCI_VOID_POINTER, zd); 

			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_SetZero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;   
			
			z_drive_set_zero(zd);
									
			z_drive_emit_change_signal(zd, 1);

			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_GotoStoredPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double microns_val;
			Z_Drive *zd = (Z_Drive*) callbackData;       
			
			GetCtrlVal(panel, FOCUS_STORED_POS, &microns_val);
			
			z_drive_set_position(zd, microns_val);
			
			z_drive_emit_change_signal(zd, 1);
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 1, GCI_VOID_POINTER, zd); 

			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_StorePos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    
	
			double microns_val;

			GetCtrlVal(panel, FOCUS_FOCUS, &microns_val);
			SetCtrlVal(panel, FOCUS_STORED_POS, microns_val); 
		
			zd->_stored_pos = microns_val;
			
			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_StepUp (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    
	
			double step, microns_val;
	
			GetCtrlVal(panel, FOCUS_FOCUS, &microns_val);
			GetCtrlVal(panel, FOCUS_Z_STEP, &step);
		
			z_drive_set_position(zd, microns_val+step); 
			zd->_z_step = step;
			zd->_current_pos = microns_val+step;

			z_drive_emit_change_signal(zd, 1);
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 1, GCI_VOID_POINTER, zd); 

			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_Step (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    

			GetCtrlVal(panel, FOCUS_Z_STEP, &(zd->_z_step));      
		
			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_Setup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    

			z_drive_display_setup_panel(zd);  
		
			break;
		}
	}
	return 0;
}

static int CVICALLBACK OnZ_Drive_StepDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			Z_Drive *zd = (Z_Drive*) callbackData;    

			double step, microns_val;

			GetCtrlVal(panel,FOCUS_FOCUS, &microns_val);
			GetCtrlVal(panel,FOCUS_Z_STEP, &step);
			
			z_drive_set_position(zd, microns_val-step); 
			zd->_z_step = step;
			zd->_current_pos = microns_val-step;
			
			z_drive_emit_change_signal(zd, 1);
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", GCI_INT, 1, GCI_VOID_POINTER, zd); 

			}break;
		}
	}
	return 0;
}


int z_drive_hardware_initialise (Z_Drive* zd)
{
	int status = UIMODULE_ERROR_NONE;  
	
	zd->_timer = -1;

	CHECK_Z_DRIVE_VTABLE_PTR(zd, hw_initialise) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, hw_initialise)(zd) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive hardware initialisation failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				zd->_hw_initialised = 0;   
				return Z_DRIVE_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	zd->_hw_initialised = 1;
	
  	return Z_DRIVE_SUCCESS;
}

int z_drive_initialise (Z_Drive* zd)
{
	zd->_timer = NewCtrl(zd->_panel_id, CTRL_TIMER, "", 0, 0);
		
	// Call specific device initialisation if necessary.
	if( (zd->lpVtbl.initialise != NULL)) {
		
		if( (zd->lpVtbl.initialise)(zd) == Z_DRIVE_ERROR )
			return Z_DRIVE_ERROR;  	
	}
	
	z_drive_load_from_default_file(zd);

	zd->_initialised = 1;

	z_drive_enable_timers(zd);

  	return Z_DRIVE_SUCCESS;
}


int z_drive_is_initialised(Z_Drive* zd)
{
	return (zd->_initialised && zd->_hw_initialised);	
}

int z_drive_move_begin_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveMoveBegin", handler, data) == SIGNAL_ERROR) {
		send_z_drive_error_text(zd, "Can not connect signal handler for Z Drive Move Begin signal");
		return Z_DRIVE_ERROR;
	}

	return Z_DRIVE_SUCCESS;
}

int z_drive_changed_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", handler, data) == SIGNAL_ERROR) {
		send_z_drive_error_text(zd, "Can not connect signal handler for Z Drive Change signal");
		return Z_DRIVE_ERROR;
	}

	return Z_DRIVE_SUCCESS;
}

int z_drive_changed_by_user_handler_connect(Z_Drive* zd, Z_DRIVE_INT_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", handler, data) == SIGNAL_ERROR) {
		send_z_drive_error_text(zd, "Can not connect signal handler for Z Drive Change By User signal");
		return Z_DRIVE_ERROR;
	}

	return Z_DRIVE_SUCCESS;
}

int z_drive_setup_displayed_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveSetupPanelDisplayed", handler, data) == SIGNAL_ERROR) {
		send_z_drive_error_text(zd, "Can not connect signal handler for Z Drive setup panel displayed signal");
		return Z_DRIVE_ERROR;
	}

	return Z_DRIVE_SUCCESS;
}

void zdrive_dont_respond_to_event_val_changed(Z_Drive* zd)
{
	zd->_respond_to_event_val_changed = 0;     
}

void z_drive_set_is_part_of_stage(Z_Drive* zd)
{
	zd->_is_part_of_stage = 1;
}

int z_drive_is_part_of_stage(Z_Drive* zd)
{
	return zd->_is_part_of_stage;
}

void z_drive_constructor(Z_Drive* zd, const char *name, const char *description, const char *data_dir)
{
	memset(zd, 0, sizeof(Z_Drive));

	GciCmtNewLock("ZDrive", 0, &(zd->_lock));

	zd->_initialised = 0;  
	zd->_is_part_of_stage = 0;
	zd->_hw_initialised = 0;
	zd->_respond_to_event_val_changed = 1; 
	
	strcpy(zd->_data_dir, data_dir);
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(zd), name);    
	ui_module_set_description(UIMODULE_CAST(zd), description);
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveMoveBegin", Z_DRIVE_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", Z_DRIVE_PTR_MARSHALLER);   
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChangedByUser", Z_DRIVE_PTR_INT_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveSetupPanelDisplayed", Z_DRIVE_PTR_MARSHALLER);   

	Z_DRIVE_VTABLE_PTR(zd, hw_initialise) = NULL;   
	Z_DRIVE_VTABLE_PTR(zd, initialise) = NULL;  
	Z_DRIVE_VTABLE_PTR(zd, destroy) = NULL; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position) = NULL; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position) = NULL; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_autofocus) = NULL;
	Z_DRIVE_VTABLE_PTR(zd, z_drive_wait_for_stop_moving) = NULL;

    zd->_panel_id = ui_module_add_panel(UIMODULE_CAST(zd), "ZDriveUI.uir", FOCUS, 1); 
    zd->wait_panel_id = ui_module_add_panel(UIMODULE_CAST(zd), "ZDriveUI.uir", WAIT, 0); 
	
    InstallCtrlCallback (zd->_panel_id, FOCUS_FOCUS, OnZ_Drive_FocusChanged, zd);
    InstallCtrlCallback (zd->_panel_id, FOCUS_STORED_POS, OnZ_Drive_FocusChanged, zd);
	InstallCtrlCallback (zd->_panel_id, FOCUS_CLOSE, OnZ_Drive_FocusClosed, zd); 

	InstallCtrlCallback (zd->_panel_id, FOCUS_GOTO_ZERO, OnZ_Drive_GotoZero, zd); 

	InstallCtrlCallback (zd->_panel_id, FOCUS_SET_ZERO, OnZ_Drive_SetZero, zd); 

	InstallCtrlCallback (zd->_panel_id, FOCUS_GOTO, OnZ_Drive_GotoStoredPos, zd); 
	InstallCtrlCallback (zd->_panel_id, FOCUS_STORE, OnZ_Drive_StorePos, zd); 
	InstallCtrlCallback (zd->_panel_id, FOCUS_STEP_UP, OnZ_Drive_StepUp, zd); 
	InstallCtrlCallback (zd->_panel_id, FOCUS_STEP_DOWN, OnZ_Drive_StepDown, zd); 
	InstallCtrlCallback (zd->_panel_id, FOCUS_Z_STEP, OnZ_Drive_Step, zd); 
	
	InstallCtrlCallback (zd->_panel_id, FOCUS_SETUP, OnZ_Drive_Setup, zd); 

	ui_module_set_main_panel_title (UIMODULE_CAST(zd));     
}


int z_drive_destroy(Z_Drive* zd)
{
	z_drive_disable_timers(zd);

	z_drive_save_to_default_file(zd);   
	
	CHECK_Z_DRIVE_VTABLE_PTR(zd, destroy) 
  	
	CALL_Z_DRIVE_VTABLE_PTR(zd, destroy) 
	
	CmtDiscardLock(zd->_lock);

	ui_module_destroy(UIMODULE_CAST(zd));  
  	
  	free(zd);
  	
  	return Z_DRIVE_SUCCESS;
}


void z_drive_set_error_handler(Z_Drive* zd, UI_MODULE_ERROR_HANDLER handler, void *callback_data)
{
	ui_module_set_error_handler(UIMODULE_CAST(zd), handler, callback_data);	

}

int z_drive_is_busy (Z_Drive* zd)
{
	return zd->_busy;
}

void z_drive_prevent_changed_signal_emission(Z_Drive* zd)
{
	zd->_prevent_change_signal_emit = 1;
}

void z_drive_allow_changed_signal_emission(Z_Drive* zd)
{
	zd->_prevent_change_signal_emit = 0;
}

void z_drive_emit_change_signal(Z_Drive* zd, int update)
{
	if(!zd->_prevent_change_signal_emit)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", GCI_INT, update, GCI_VOID_POINTER, zd); 
}

void z_drive_emit_move_begin_signal(Z_Drive* zd, int update)
{
	if(!zd->_prevent_change_signal_emit)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveMoveBegin", GCI_INT, update, GCI_VOID_POINTER, zd); 
}

int z_drive_set_position(Z_Drive* zd, double focus)
{
	int status = UIMODULE_ERROR_NONE;  

	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position)     
		
	zdrive_get_lock(zd, __FUNCTION__);
	z_drive_emit_move_begin_signal(zd, 1);

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_set_position)(zd, focus) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive set focus failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				zdrive_release_lock(zd, __FUNCTION__);
				return Z_DRIVE_ERROR; 			
			}
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, focus); 	    
	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, focus); 	    

	logger_log(UIMODULE_LOGGER(zd), LOGGER_INFORMATIONAL, "%s to pos %.2f", UIMODULE_GET_DESCRIPTION(zd), focus);
	
	zd->_current_pos = focus;
	z_drive_emit_change_signal(zd, 1);
	
	zdrive_release_lock(zd, __FUNCTION__);

  	return Z_DRIVE_SUCCESS;
}

int z_drive_update_current_position(Z_Drive* zd, double focus)
{	// For systems where z drive is part of the xy stage, so z drive module can be updated

	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, focus); 	    
	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, focus); 	    
	zd->_current_pos = focus;

  	return Z_DRIVE_SUCCESS;
}

int z_drive_set_position_no_commit (Z_Drive* zd, double focus)
{   // Same as z_drive_set_position() but do not call logger nor signal emit.
	
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position)     
		
	zdrive_get_lock(zd, __FUNCTION__);

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_set_position)(zd, focus) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive set focus failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				zdrive_release_lock(zd, __FUNCTION__);
				return Z_DRIVE_ERROR; 
			}
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);

	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, focus); 	    
	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, focus); 	    

	zdrive_release_lock(zd, __FUNCTION__);

  	return Z_DRIVE_SUCCESS;
}

int z_drive_set_min_position(Z_Drive* zd)
{
	return z_drive_set_position(zd, zd->_min_microns);
}

int z_drive_set_max_position(Z_Drive* zd)
{
	return z_drive_set_position(zd, zd->_max_microns);
}

int z_drive_get_position(Z_Drive* zd, double *focus)
{
	int status = UIMODULE_ERROR_NONE;  
	
	if(zd->_use_cached_position) {
		*focus = zd->_current_pos;
	
		return Z_DRIVE_SUCCESS;
	}

	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position)     
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_get_position)(zd, focus) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive get focus failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return Z_DRIVE_ERROR; 
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return Z_DRIVE_SUCCESS;
}


int	 z_drive_get_speed(Z_Drive* zd, double *speed)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_get_speed)     
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_get_speed)(zd, speed) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive get speed failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return Z_DRIVE_ERROR; 
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return Z_DRIVE_SUCCESS;
}

int	 z_drive_set_zero(Z_Drive* zd)
{	
	int status = UIMODULE_ERROR_NONE; 

	//CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_set_zero)     
	// ignore if the specific z drive does not have this function
	if (z_drive_set_zero==NULL)
		return Z_DRIVE_SUCCESS;

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_set_zero)(zd) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive set zero failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return Z_DRIVE_ERROR; 
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return Z_DRIVE_SUCCESS;
}

int  z_drive_get_accel(Z_Drive* zd, double *accel)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_get_accel)     
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_get_accel)(zd, accel) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "Z Drive get accel failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return Z_DRIVE_ERROR; 
		}
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return Z_DRIVE_SUCCESS;
}

int z_drive_enable_autofocus(Z_Drive* zd, int enable)
{
	int status = UIMODULE_ERROR_NONE;  
	
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_autofocus)  
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( Z_DRIVE_VTABLE(zd, z_drive_enable_autofocus)(zd, enable) == Z_DRIVE_ERROR ) {
			status = send_z_drive_error_text(zd, "z_drive_enable_autofocus failed");
		
			if(status == UIMODULE_ERROR_IGNORE) 
				return Z_DRIVE_ERROR; 
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
  	return Z_DRIVE_SUCCESS;
}

void z_drive_set_setup_panel(Z_Drive* zd, int panel_id)
{
	zd->_setup_panel_id = panel_id;
	SetCtrlAttribute(zd->_panel_id, FOCUS_SETUP, ATTR_DIMMED, 0);
	SetCtrlAttribute(zd->_panel_id, FOCUS_SETUP, ATTR_VISIBLE, 1);
}

void z_drive_display_setup_panel(Z_Drive* zd)
{
	ui_module_display_panel(UIMODULE_CAST(zd), zd->_setup_panel_id);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveSetupPanelDisplayed", GCI_VOID_POINTER, zd); 
}

int  z_drive_save_settings_in_ini_fmt (Z_Drive* zd, const char *filepath)
{
	double focus;
	char buffer[256];
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(zd), device);
	 
	z_drive_get_position (zd, &focus);      

	sprintf(buffer, "Focus=%.2f\nStored Pos=%.2f\nZ Step=%.2f\n\n",
		focus, zd->_stored_pos, zd->_z_step);
	
	str_change_char(buffer, '\n', '\0'); 
	
	if(!WritePrivateProfileSection(device, buffer, filepath))
		return Z_DRIVE_ERROR;	  
	
	return Z_DRIVE_SUCCESS;	  
}

int  z_drive_set_min_max (Z_Drive* zd, double min, double max)
{
	zd->_min_microns = min;
	zd->_max_microns = max;

	SetCtrlAttribute(zd->_panel_id, FOCUS_FOCUS, ATTR_MIN_VALUE, zd->_min_microns);
	SetCtrlAttribute(zd->_panel_id, FOCUS_FOCUS, ATTR_MAX_VALUE, zd->_max_microns);

	SetCtrlAttribute(zd->_panel_id, FOCUS_STORED_POS, ATTR_MIN_VALUE, zd->_min_microns);
	SetCtrlAttribute(zd->_panel_id, FOCUS_STORED_POS, ATTR_MAX_VALUE, zd->_max_microns);

	if (zd->_current_pos < zd->_min_microns)
		zd->_current_pos = zd->_min_microns;

	if (zd->_current_pos > zd->_max_microns)
		zd->_current_pos = zd->_max_microns;

	return Z_DRIVE_SUCCESS;
}

int  z_drive_load_settings_from_ini_fmt (Z_Drive* zd, const char *filepath)
{
	double val;
	char buffer[256];
	char device[UIMODULE_NAME_LEN];
	dictionary* ini = iniparser_load(filepath);   
	
	ui_module_get_name(UIMODULE_CAST(zd), device);

	SetCtrlAttribute(zd->_panel_id, FOCUS_FOCUS, ATTR_MIN_VALUE, zd->_min_microns);
	SetCtrlAttribute(zd->_panel_id, FOCUS_FOCUS, ATTR_MAX_VALUE, zd->_max_microns);

	SetCtrlAttribute(zd->_panel_id, FOCUS_STORED_POS, ATTR_MIN_VALUE, zd->_min_microns);
	SetCtrlAttribute(zd->_panel_id, FOCUS_STORED_POS, ATTR_MAX_VALUE, zd->_max_microns);

	sprintf(buffer, "%s:Stored Pos", device);
	val = iniparser_getdouble(ini, buffer, zd->_min_microns); 
	
	if (val >= zd->_min_microns && val <= zd->_max_microns)
		zd->_stored_pos = val;
	else
		zd->_stored_pos = zd->_min_microns;

	sprintf(buffer, "%s:Z Step", device);
	val = iniparser_getdouble(ini, buffer, 10); 
	
	if (val > 0 && val <= zd->_max_microns)
		zd->_z_step = val;
	else
		zd->_z_step = 10;

	SetCtrlVal(zd->_panel_id, FOCUS_STORED_POS, zd->_stored_pos);
	SetCtrlVal(zd->_panel_id, FOCUS_Z_STEP, zd->_z_step);
	
// Why do this, may cause problems with some drives
//	sprintf(buffer, "%s:Focus", device);
//	val = iniparser_getdouble(ini, buffer, 1.0e10);
//	if (val<1.0e10) z_drive_set_position(zd, val); 

	iniparser_freedict(ini);
	
	return Z_DRIVE_SUCCESS;		
}

int z_drive_disable_timers(Z_Drive* zd)
{
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_disable_timers)  
		
	if( Z_DRIVE_VTABLE(zd, z_drive_enable_disable_timers)(zd, 0) == Z_DRIVE_ERROR ) {
		send_z_drive_error_text(zd, "z_drive_enable_disable_timers failed");
	}

	return Z_DRIVE_SUCCESS;
}

int z_drive_enable_timers(Z_Drive* zd)
{
	CHECK_Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_disable_timers)  
		
	if( Z_DRIVE_VTABLE(zd, z_drive_enable_disable_timers)(zd, 1) == Z_DRIVE_ERROR ) {
		send_z_drive_error_text(zd, "z_drive_enable_disable_timers failed");
	}

	return Z_DRIVE_SUCCESS;
}

int z_drive_wait_for_stop_moving (Z_Drive* zd, double timeout)
{
	int status = UIMODULE_ERROR_NONE;  

	if( Z_DRIVE_VTABLE(zd, z_drive_wait_for_stop_moving) != NULL) {

		zdrive_get_lock(zd, __FUNCTION__);
		
		do {
			status = UIMODULE_ERROR_NONE;
			
			if( Z_DRIVE_VTABLE(zd, z_drive_wait_for_stop_moving)(zd, timeout) == Z_DRIVE_ERROR ) {
				status = send_z_drive_error_text(zd, "Z Drive wait to stop moving failed");
			
				if(status == UIMODULE_ERROR_IGNORE) {
					zdrive_release_lock(zd, __FUNCTION__);
					return Z_DRIVE_ERROR; 			
				}
			}
		} 
		while(status == UIMODULE_ERROR_RETRY);
		
		zdrive_release_lock(zd, __FUNCTION__);
	}
	else {
		// Use a generic wait for stop moving that uses zd->_busy

		double time = Timer();
		
		while ((Timer()-time) < timeout)
		{
			if(!z_drive_is_busy (zd)) 
			{
				return Z_DRIVE_SUCCESS;
			}
		}
	}

  	return Z_DRIVE_SUCCESS;
}

void z_drive_hide_autofocus_controls(Z_Drive* zd)
{
	SetCtrlAttribute(zd->_panel_id, FOCUS_AUTOFOCUS_SETUP, ATTR_VISIBLE, 0);
	SetCtrlAttribute(zd->_panel_id, FOCUS_TOGGLEBUTTON, ATTR_VISIBLE, 0); 
}

void z_drive_reveal_message_controls(Z_Drive* zd)
{
	SetPanelAttribute(zd->_panel_id, ATTR_HEIGHT, 140);
	SetCtrlAttribute(zd->_panel_id, FOCUS_MESSAGE, ATTR_VISIBLE, 1);
	SetCtrlAttribute(zd->_panel_id, FOCUS_CLOSE, ATTR_TOP, 109); 
}

void z_drive_reveal_set_datum_control(Z_Drive* zd)
{
	SetCtrlAttribute(zd->_panel_id, FOCUS_SET_ZERO, ATTR_VISIBLE, 1);
}

void z_drive_set_message(Z_Drive* zd, char *message)
{
	SetCtrlVal(zd->_panel_id, FOCUS_MESSAGE, message);
}

void z_drive_set_centre_position(Z_Drive* zd, double position)
{
  zd->_centre_position = 0.0;
  zd->_centre_position_set = 1;
}