#include "ATD_Scanner_Dummy.h"
#include "ScannerUI.h"
#include "gci_utils.h"

#include "ATD_UsbInterface_A.h"

#include <cviauto.h>
#include <rs232.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define Round  RoundRealToNearestInteger 

static int manual_scanner_save_settings_to_EEPROM(Scanner* scanner)
{
	return SCANNER_SUCCESS;
}
	
static int manual_scanner_disable_scanner(Scanner* scanner, int disable)
{
	return SCANNER_SUCCESS;
}

static int manual_scanner_destroy (Scanner* scanner)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_select_clock(Scanner* scanner, int pixclk, int lineclk, int frameclk)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_load_line_data(Scanner* scanner, int line)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_rep_rate(Scanner* scanner, int rep_val,int rep_div_1)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_speed(Scanner* scanner, int speed, int hyst_offset)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_resolution(Scanner* scanner, int resolution)
{
	return SCANNER_SUCCESS;  
}


static int manual_scanner_set_zoom(Scanner* scanner, int zoom)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_x_shift(Scanner* scanner, int x_shift)
{
	scanner->_x_shift = x_shift;

	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_y_shift(Scanner* scanner, int y_shift)
{
	scanner->_y_shift = y_shift;

	return SCANNER_SUCCESS;  
}

static int manual_scanner_get_min_max_x_shift(Scanner* scanner, int *min, int *max)
{
	*min = 0;
	*max = 4095;

	return SCANNER_SUCCESS;
}

static int manual_scanner_get_min_max_y_shift(Scanner* scanner, int *min, int *max)
{
	*min = 0;
	*max = 4095;

	return SCANNER_SUCCESS;
}

static int manual_scanner_reverse_scan(Scanner* scanner, int direction)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_line_scan(Scanner* scanner, int enable)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_set_no_frames(Scanner* scanner, int frames)
{
	return SCANNER_SUCCESS;  
}


static int manual_scanner_get_frame_time(Scanner* scanner, double *frame_time)
{
	*frame_time = 1.0;

	return SCANNER_SUCCESS;  
}

static int manual_scanner_start_scan(Scanner* scanner, int frames)
{
	return SCANNER_SUCCESS;  
}
	
static int manual_scanner_stop_scan(Scanner* scanner)
{
	return SCANNER_SUCCESS;  
}
	
	
static int manual_scanner_read_error_signal(Scanner* scanner, int *scannerServoError)
{
	return SCANNER_SUCCESS;  
}


static int manual_scanner_hw_init (Scanner* scanner)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_load_settings (Scanner* scanner, const char *filepath)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_save_settings (Scanner* scanner, const char *filepath, const char *flags)
{
	return SCANNER_SUCCESS;  
}

static int manual_scanner_spc_left_position (Scanner *scanner, int res, int *pos)
{
	*pos = 0;

	return SCANNER_SUCCESS; 
}

static int manual_scanner_get_pixel_time(Scanner* scanner, double *time)
{
	*time = 0.01;

	return SCANNER_SUCCESS;
}

Scanner* manual_scanner_new(char *name, char *description, const char *data_dir, const char *data_file, UI_MODULE_ERROR_HANDLER handler, void* data)
{
	Scanner* scanner = scanner_new(name, description, data_dir, data_file, sizeof(ScannerManual));
	
	ScannerManual *scanner_hts = (ScannerManual *) scanner;
	
	ui_module_set_error_handler(UIMODULE_CAST(scanner), handler, scanner);
										    
	SCANNER_VTABLE_PTR(scanner, hw_init) = manual_scanner_hw_init;
	SCANNER_VTABLE_PTR(scanner, destroy) = manual_scanner_destroy; 
	SCANNER_VTABLE_PTR(scanner, scanner_select_clock) = manual_scanner_select_clock; 
	SCANNER_VTABLE_PTR(scanner, scanner_load_line_data) = manual_scanner_load_line_data; 
	SCANNER_VTABLE_PTR(scanner, scanner_set_rep_rate) = manual_scanner_set_rep_rate;
	SCANNER_VTABLE_PTR(scanner, scanner_set_speed) = manual_scanner_set_speed;
	SCANNER_VTABLE_PTR(scanner, scanner_set_resolution) = manual_scanner_set_resolution;
 	SCANNER_VTABLE_PTR(scanner, scanner_set_zoom) = manual_scanner_set_zoom;
	SCANNER_VTABLE_PTR(scanner, scanner_set_x_shift) = manual_scanner_set_x_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_set_y_shift) = manual_scanner_set_y_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_x_shift) = manual_scanner_get_min_max_x_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_y_shift) = manual_scanner_get_min_max_y_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_reverse_scan) = manual_scanner_reverse_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_line_scan) = manual_scanner_line_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_set_no_frames) = manual_scanner_set_no_frames;
	SCANNER_VTABLE_PTR(scanner, scanner_get_frame_time) = manual_scanner_get_frame_time;
	SCANNER_VTABLE_PTR(scanner, scanner_start_scan) = manual_scanner_start_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_stop_scan) = manual_scanner_stop_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_disable_scanner) = manual_scanner_disable_scanner;
	SCANNER_VTABLE_PTR(scanner, scanner_save_settings_to_EEPROM) = manual_scanner_save_settings_to_EEPROM;
	SCANNER_VTABLE_PTR(scanner, scanner_read_error_signal) = manual_scanner_read_error_signal;
	SCANNER_VTABLE_PTR(scanner, load_settings) = manual_scanner_load_settings;    
	SCANNER_VTABLE_PTR(scanner, save_settings) = manual_scanner_save_settings;    
	SCANNER_VTABLE_PTR(scanner, scanner_get_spc_left_position) = manual_scanner_spc_left_position;
	SCANNER_VTABLE_PTR(scanner, scanner_get_pixel_time) = manual_scanner_get_pixel_time;

	return scanner;
}
