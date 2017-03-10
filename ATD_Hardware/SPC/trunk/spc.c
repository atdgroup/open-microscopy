#include "spc.h"
#include "spc_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "file_prefix_dialog.h"
#include "ScannerUI.h"

#include "FilenameUtils.h"

#include "FreeImageAlgorithms_Palettes.h" 
#include "FreeImageAlgorithms_Statistics.h"
#include "FreeImageAlgorithms_Drawing.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageIcs_IO.h"
#include "icsviewer_signals.h"
#include "icsviewer_tools.h"
#include "linearscale_plugin.h"

#ifdef BUILD_MODULE_LASER_POWER_MONITOR
#include "LaserPowerMonitor.h"
#endif

#include <analysis.h>
#include <userint.h>
#include <utility.h>

#include "asynctmr.h"
#include "ThreadDebug.h"

#define SPC_BOARD_MAX_UNITS 16777216

#define SPC_BASE_MSG (WM_USER+90) 
#define SPC_DISPLAY_IMAGE_MSG (SPC_BASE_MSG+1) 

enum {REVERSE_RAINBOW, PILEUP, GREY};

char time_str[256];

typedef enum {SPC_SCANNER_X, SPC_SCANNER_Y} SPC_SCANNER;

LRESULT CALLBACK SpcWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

static int SPC_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Spc*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Spc *) args[0].void_ptr_data, callback_data);
	
	return SPC_SUCCESS;	
}

int bh_changed_handler_connect (Spc* spc, SPC_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(spc), "ParamChanged", handler, callback_data) == SIGNAL_ERROR) {
		send_spc_error_text(spc, "Can not connect signal handler for ParamChanged signal");
		return SPC_ERROR;
	}

	return SPC_SUCCESS;
}

int send_spc_error_text (Spc* spc, char fmt[], ...)
{
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);

	if(spc != NULL)
		logger_log_ap(UIMODULE_LOGGER(spc), LOGGER_ERROR, fmt, ap);
	
	va_end(ap);
	
	return SPC_SUCCESS;
}


int spc_check_error(Spc *spc, int errcode)
{
	char errstr[100] = "";
	
	if (errcode == 0)
		return 0;
	
	SPC_get_error_string(errcode, errstr, 100);
   	send_spc_error_text(spc, errstr); 

 	return errcode;
}

int spc_get_max_value_for_adc_res(Spc *spc)
{
	return Round(pow(2.0,spc->_spc_data->adc_resolution));
}

static void dictionary_ics_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data;
	
	FreeImageIcs_IcsAddHistoryString (ics, key, val);         	
}


/*
struct panel_ctrl_data
{
	int panel;
	int ctrl;
};

static void AddKeyValueToTree(int panel, int ctrl, const char *key, const char *value)
{
	int index = InsertTreeItem (panel, ctrl, VAL_SIBLING, 0, VAL_LAST, key, 0, 0, 0);

	SetTreeCellAttribute (panel, ctrl, index, 1, ATTR_LABEL_TEXT, key);
	SetTreeCellAttribute (panel, ctrl, index, 2, ATTR_LABEL_TEXT, value);
}

static void dictionary_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) data;
	
	AddKeyValueToTree(pcd->panel, pcd->ctrl, key, val);          	
}

void spc_on_show_metadata (IcsViewerWindow* window, int panel, int ctrl, void* callback)
{
	Spc *spc = (Spc *) callback;   
		
	struct panel_ctrl_data * pcd = (struct panel_ctrl_data*) malloc(sizeof(struct panel_ctrl_data));

	pcd->panel = panel;
	pcd->ctrl = ctrl;
	
	dictionary_foreach(spc->_metadata, dictionary_keyval_callback, pcd);
	
	free(pcd);
}
*/

static void spc_dictionary_set_from_ring_label(dictionary *d, const char *key, int panel_id, int ctrl_id)
{
	int index;
	char label[100] = "";

	GetCtrlIndex (panel_id, ctrl_id, &index);
	
	GetLabelFromIndex (panel_id, ctrl_id, index, label);

	dictionary_set(d, key, label);
}

dictionary* spc_get_metadata(Spc* spc, dictionary *d)
{
	char buffer[256];
	double extents_x, extents_y, timebase, rate;
	int x, y;
	float f_val;

	Microscope *ms = (Microscope*) spc->_ms;

	microscope_add_microscope_metadata(ms, d);

	// image metadata
	// type
	dictionary_set(d, "type", "Time Resolved");      
	dictionary_set(d, "type", "FluorescenceLifetime");      
	dictionary_set(d, "labels", "t x y");      

	x = spc->_spc_data->scan_size_x;
	y = spc->_spc_data->scan_size_y;
	sprintf(buffer, "%d %d %d", spc_get_max_value_for_adc_res(spc), x, y); 
	dictionary_set(d, "dimensions", buffer);
	dictionary_set(d, "offsets", "0 0 0");     

	// Units
	dictionary_set(d, "units", "s m m");      

	extents_x = x * spc->_umPerPixel;												//microns
	extents_y = y * spc->_umPerPixel;												//microns
	timebase = spc->_spc_data->tac_range/spc->_spc_data->tac_gain * 1e-9;	//seconds
	sprintf(buffer, "%e %.4e %.4e", timebase, extents_x * 1e-6, extents_y * 1e-6); //seconds and metres 
	dictionary_set(d, "extents", buffer);  

	// other mscope hardware
	microscope_add_hardware_metadata(ms, d);

	// Scanner
	scanner_add_metadata(ms->_scanner, d); 

	// TCSPC
	if (spc->_mod_info_found) {
		sprintf(buffer, "bh SPC-%d", spc->mod_info.module_type);
		dictionary_set(d, "TCSPC", buffer);
	}
	else
		dictionary_set(d, "TCSPC", "bh SPC");

	dictionary_setint(d, "TCSPC total frames", spc->_acc_frames);
	sprintf(buffer, "%.1f", spc->_acc_time);  // round to 0.1 of a second
	dictionary_set(d, "TCSPC total time (s)", buffer);

	dictionary_setint(d, "TCSPC adc res", spc_get_max_value_for_adc_res(spc));

	sprintf(buffer, "%d, %d", spc->_time_window_min, spc->_time_window_max);
	dictionary_set(d, "TCSPC time window", buffer);
	
	spc_dictionary_set_from_ring_label(d, "TCSPC operation mode", spc->_params_ui_panel, SPC_PARAM_OP_MODE);
	spc_dictionary_set_from_ring_label(d, "TCSPC overflow", spc->_params_ui_panel, SPC_PARAM_OFLO_CTRL);
	spc_dictionary_set_from_ring_label(d, "TCSPC trigger", spc->_params_ui_panel, SPC_PARAM_TRIGGER);

	dictionary_setdouble(d, "TCSPC collection time", spc->_spc_data->collect_time);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DEAD_TIME, &x);
	dictionary_setint(d, "TCSPC dead time enabled", x);

	spc_dictionary_set_from_ring_label(d, "TCSPC x sync polarity", spc->_params_ui_panel, SPC_PARAM_X_POLARITY);
	spc_dictionary_set_from_ring_label(d, "TCSPC y sync polarity", spc->_params_ui_panel, SPC_PARAM_Y_POLARITY);
	spc_dictionary_set_from_ring_label(d, "TCSPC clock polarity", spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_POLARITY);
	spc_dictionary_set_from_ring_label(d, "TCSPC line compression", spc->_params_ui_panel, SPC_PARAM_LINE_COMPRESSION);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_DIVIDER, &x);
	dictionary_setint(d, "TCSPC pixel clk divider", x);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LEFT_BORDER, &x);
	dictionary_setint(d, "TCSPC left border", x);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TOP_BORDER, &x);
	dictionary_setint(d, "TCSPC top border", x);

	spc_dictionary_set_from_ring_label(d, "TCSPC pixel clock", spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK);

	dictionary_setdouble(d, "TCSPC cfd limit low", spc->_spc_data->cfd_limit_low);
	dictionary_setdouble(d, "TCSPC cfd limit high", spc->_spc_data->cfd_limit_high);
	dictionary_setdouble(d, "TCSPC cfd zc level", spc->_spc_data->cfd_zc_level);
	dictionary_setdouble(d, "TCSPC cfd holdoff", spc->_spc_data->cfd_holdoff);

	spc_dictionary_set_from_ring_label(d, "TCSPC adc resolution", spc->_params_ui_panel, SPC_PARAM_ADC_RES);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_MEM_OFFSET, &f_val);
	dictionary_setdouble(d, "TCSPC mem offset", f_val);

	spc_dictionary_set_from_ring_label(d, "TCSPC dither range", spc->_params_ui_panel, SPC_PARAM_DTHER);
	
	dictionary_setint(d, "TCSPC count increment", spc->_spc_data->count_incr);

	dictionary_setint(d, "TCSPC delay", spc->_spc_data->ext_latch_delay);
	dictionary_setint(d, "TCSPC routing x", spc->_spc_data->scan_rout_x);
	dictionary_setint(d, "TCSPC routing y", spc->_spc_data->scan_rout_y);
	dictionary_setint(d, "TCSPC scan x", spc->_spc_data->scan_size_x);
	dictionary_setint(d, "TCSPC scan y", spc->_spc_data->scan_size_y);

	dictionary_setdouble(d, "TCSPC tac range", spc->_spc_data->tac_range);
	dictionary_setint(d, "TCSPC tac gain", spc->_spc_data->tac_gain);
	dictionary_setdouble(d, "TCSPC tac offset", spc->_spc_data->tac_offset);
	dictionary_setdouble(d, "TCSPC tac limit low", spc->_spc_data->tac_limit_low);
	dictionary_setdouble(d, "TCSPC tac limit high", spc->_spc_data->tac_limit_high);

	dictionary_setdouble(d, "TCSPC sync zc level", spc->_spc_data->sync_zc_level);
	spc_dictionary_set_from_ring_label(d, "TCSPC sync divider", spc->_params_ui_panel, SPC_PARAM_SYNC_FREQ_DIV);

	dictionary_setdouble(d, "TCSPC sync holdoff", spc->_spc_data->sync_holdoff);
	dictionary_setdouble(d, "TCSPC sync threshold", spc->_spc_data->sync_threshold);

	// MUST GET AN AVERAGE RATE OR RATE FROM TH EMIDDLE OF THE IMAGE
	// CURRENTLY THIS IS CALLED BEFORE THE IMAGE IS STARTED

	GetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC, &rate);
	dictionary_setdouble(d, "TCSPC sync rate", rate);

	GetCtrlVal(spc->_rates_ui_panel, SPC_RATES_CFD, &rate);
	dictionary_setdouble(d, "TCSPC approx cfd rate", rate);

	GetCtrlVal(spc->_rates_ui_panel, SPC_RATES_TAC, &rate);
	dictionary_setdouble(d, "TCSPC approx tac rate", rate);

	GetCtrlVal(spc->_rates_ui_panel, SPC_RATES_ADC, &rate);
	dictionary_setdouble(d, "TCSPC approx adc rate", rate);

	return d;
}


//////////////////////////////////////////////////////////////////////
// Memory Configuration

static int test_fill_state(Spc* spc)
{
	int i,ret;
	short state;
	double stime;

	for( i = 0; i < MAX_NO_OF_SPC; i++) {
		if(spc->_modules_active[i]) {
			stime = Timer();
			while(1) {
				ret = SPC_test_state( i, &state);
				spc_check_error(spc, ret);
				if(ret < 0) return ret;
				if( (state & SPC_HFILL_NRDY) == 0) break;  // fill finished
				if(Timer() - stime > 2.5 ) return -SPC_FILL_TOUT;   // 0.5 for BH600 ,2.5 for BH700
			}
        }
    }

	return 0;  
}

int bh_clear_memory(Spc* spc)
{
	unsigned short meas_page=1;
	int ret;
	
	ret = SPC_fill_memory( -1, -1, meas_page-1, 0);   	//all modules, all blocks

	if (ret > 0)
		ret = test_fill_state(spc);   			//wait for it to finish

	if (ret < 0) {
   		spc_check_error(spc, ret);
		return ret; 						// errors during memory fill
	}
	
	return SPC_SUCCESS;
}

static int MemConfig(Spc* spc, unsigned short mode)
{
	int i, ret, nChans;
	unsigned short meas_page = 1;
	short sync_state[MAX_NO_OF_SPC];
	int noRoutingBits[17] = {0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};
	
	//Before any measurement:-
	//1. BH memory must be configured (-1 do it for all modules)
	
	// How many channels are enabled, 1 or 4?
	nChans = spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y;

	if (nChans > 1)
		nChans = 4;
	
	if (mode == BH_SCOPE_MODE) 
		ret = SPC_configure_memory(-1, spc->_spc_data->adc_resolution, noRoutingBits[nChans], &(spc->mem_info));
	else {  // xy scan
		ret = SPC_set_parameter(spc->_active_module, ADC_RESOLUTION, (float) spc->_spc_data->adc_resolution);
		ret = SPC_set_parameter(spc->_active_module, SCAN_SIZE_X, (float) spc->_spc_data->scan_size_x);
		ret = SPC_set_parameter(spc->_active_module, SCAN_SIZE_Y, (float) spc->_spc_data->scan_size_y);
		ret = SPC_set_parameter(spc->_active_module, SCAN_ROUT_X, (float) spc->_spc_data->scan_rout_x);
		ret = SPC_set_parameter(spc->_active_module, SCAN_ROUT_Y, (float) spc->_spc_data->scan_rout_y);
		ret = SPC_configure_memory(-1, -1, noRoutingBits[nChans], &(spc->mem_info));
	}

	spc_check_error(spc, ret);
		
	//2. Clear BH memory blocks
	//ret=BH_fill_memory(module, block, page, value);

	SPC_set_page( -1, meas_page-1);   // before fill
    
     //3. Measurement page must be set on all modules
	ret = SPC_set_page( -1, meas_page-1);
	spc_check_error(spc, ret);

     //4. Rates should  be cleared, sync state can be checked 
	for( i = 0; i < MAX_NO_OF_SPC; i++) {
  		if(spc->_modules_active[i]) {
    		ret = SPC_clear_rates(i);  // it is needed one time only 
    		ret = SPC_get_sync_state (i, &sync_state[i]);
			spc_check_error(spc, ret);
    	}
  	}
  	return SPC_SUCCESS;
}

int bh_set_operation_mode(Spc* spc, unsigned short mode)
{
	if (mode == BH_SCOPE_MODE) {

		logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC set to oscilloscope mode"); 

		SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, BH_SCOPE_MODE);

		//spc_dim_sys_param_controls(spc, 1);
		SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, spc->_oscilloscope_collect_time);
	}
	else {

		logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC set to xy scan mode"); 

		SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, BH_SCAN_MODE);

		//spc_dim_sys_param_controls(spc, 0);
		SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, spc->_scan_collect_time);
	}

	spc->_spc_data->mode = mode;

	spc_send_sys_params(spc);

	return MemConfig(spc, spc->_spc_data->mode);
}

int bh_set_oscilloscope_mode(Spc* spc)
{
	if(bh_set_operation_mode(spc, BH_SCOPE_MODE) == SPC_ERROR)
		return SPC_ERROR;

	return SPC_SUCCESS;
}


void bh_set_error_handler(Spc* spc, UI_MODULE_ERROR_HANDLER handler)
{
	ui_module_set_error_handler(UIMODULE_CAST(spc), handler, spc);
}


// Returns the amount of memory needed for the chosen parameters
static int spc_get_required_acquisition_memory(Spc* spc, int scan_rout_x, int scan_rout_y, int scan_size_x, int scan_size_y, int adc_resolution)
{
	int max_t = Round(pow(2.0, adc_resolution));

	return (scan_rout_x * scan_rout_y * scan_size_x * scan_size_y * max_t);
}

static int spc_check_acquisition_has_enough_memory(Spc* spc, int scan_rout_x, int scan_rout_y, int scan_size_x, int scan_size_y, int adc_resolution)
{
	// Check whether there is enough memory on the card for creating this image.
	// time_window_range = spc->_time_window_max - spc->_time_window_min;

	// We could have used the time window range here.
	// Ie a smaller time window we could have a bigger scan size but we
	// seem to count the photons in the max possible time window for the adc
	int memory = spc_get_required_acquisition_memory(spc, scan_rout_x, scan_rout_y, scan_size_x, scan_size_y, adc_resolution);

	if(memory == SPC_BOARD_MAX_UNITS) {
		return 0;
	}
	else if(memory < SPC_BOARD_MAX_UNITS) {
		return -1;
	}

	return 1;
}

static void spc_fov(Spc* spc)
{
	unsigned int image_width, image_height;
	int scanner_zoom;
	double fov_um;

	if (spc->_camera == NULL) {
		spc->_umPerPixel = 1.0;
		GCI_ImagingWindow_SetMicronsPerPixelFactor(spc->_spc_window, spc->_umPerPixel);
		return;
	}

	scanner_zoom = scanner_get_zoom(spc->_scanner);

	// Image size in pixels 
	gci_camera_get_size(spc->_camera, &image_width, &image_height);
 
	// Get microns per pixel corrected for binning
	spc->_umPerPixel = gci_camera_get_true_microns_per_pixel(spc->_camera);

	fov_um = image_height * spc->_umPerPixel;
	
	// Pray for square pixels!!
	spc->_umPerPixel = (fov_um /spc->_spc_data->scan_size_x) / ((double) scanner_zoom);
	GCI_ImagingWindow_SetMicronsPerPixelFactor(spc->_spc_window, spc->_umPerPixel);
}	

SPC_PARAMETER display_mem_error_dialog(Spc *spc)
{
	int pnl, ctrl, mem_err_panel;
	char buffer[500];

	find_resource("spc_ui.uir", buffer); 

	mem_err_panel = LoadPanel(0, buffer, SPC_ERR); 

	GCI_MovePanelToDefaultMonitorForDialogs(mem_err_panel);

	if(InstallPopup(mem_err_panel) < 0) {
		
		GCI_MessagePopup("Unknown Error", "Failed to create spc error popup");
		return SPC_PARAM_NONE;
	}
	
	while (1) {
			
		if(GetUserEvent (0, &pnl, &ctrl) < 0) {
		
			GCI_MessagePopup("Unknown Error", "Failed to get event from popup");
			return SPC_PARAM_NONE;
		}

		if (pnl == mem_err_panel) {
			
			if (ctrl == SPC_ERR_ADC_RES)  {
				DiscardPanel(mem_err_panel);  
				return SPC_PARAM_ADC;
			}
		
			if (ctrl == SPC_ERR_SCAN_SIZE)  {
				DiscardPanel(mem_err_panel);  
				return SPC_PARAM_SCAN_SIZE;
			}

			if (ctrl == SPC_ERR_IGNORE)  {
				DiscardPanel(mem_err_panel);  
				return SPC_PARAM_NONE;
			}
		}
	}
	
	return SPC_PARAM_NONE;   
}


void spc_adapt_parameters_for_memory(Spc* spc, int warn_if_required_less_than_real)
{
	int i, channels, adc_resolution,  max_adc_value, mem_check;
	unsigned long scan_x; 
	int scan_size, max_scan_size;
	unsigned short int adc_res;
	SPC_PARAMETER param;

	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_RESOLUTION, &scan_x);
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, &adc_res);

	scan_size = scan_x * scan_x;
	max_scan_size = scan_size;

	// If we have enough memory just return
	mem_check = spc_check_acquisition_has_enough_memory(spc, spc->_spc_data->scan_rout_x, spc->_spc_data->scan_rout_y, scan_x, scan_x, adc_res);
	if(mem_check == 0 || (mem_check < 0 && warn_if_required_less_than_real == 0)) {  // if mem is filled, or mem to spare when we don't want to warn_if_required_less_than_real, proceed
		goto UPDATE_UI;
	}

	// Ok change the variable the user chooses to modify 
	param = display_mem_error_dialog(spc);

	channels = spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y;

	// Ok change the adc resolution
	if(param == SPC_PARAM_ADC) {
		
		int max, max_adc_value_for_us = SPC_BOARD_MAX_UNITS / (scan_size * channels);
		
		for(i=12; i >= 0; i-=2) {
	
			max = (unsigned short) pow(2.0, i);

			if(max <= max_adc_value_for_us)
				break;	
		}

		bh_set_adc_res(spc, i);
	}
	else if(param == SPC_PARAM_SCAN_SIZE) {

		adc_resolution = (unsigned short) spc_get_max_value_for_adc_res(spc);
		max_scan_size = SPC_BOARD_MAX_UNITS / (adc_resolution * channels);

		scan_x = Round(sqrt(max_scan_size));

		if (spc->_spc_data->scan_size_x > scan_x) {
			spc->_spc_data->scan_size_x = scan_x;
			spc->_spc_data->scan_size_y = scan_x;
		}	
	}
	else {

		return;
	}

UPDATE_UI:

	// Don't change the resolution if we are in Park
	if(scan_x > 1)
		scanner_set_resolution(spc->_scanner, scan_x);
	
	// Change time window if neccessary
	max_adc_value = spc_get_max_value_for_adc_res(spc);

	if(spc->_time_window_max > max_adc_value) {
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TO_TW, max_adc_value - 1);
	}

	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, spc->_spc_data->adc_resolution);
	spc_update_params_panel(spc);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_adapt_parameters_for_memory5\n", time_str);
#endif
}

int bh_mode(Spc* spc)
{
	return spc->_spc_data->mode;
}

static int set_mode(Spc* spc, int mode)
{
	int i, force_use=1, ret, work_mode;
	
	//Attempt to set mode for board 0 with force_use true
	for( i =0 ; i < MAX_NO_OF_SPC; i++)
		spc->_modules_active[i] = 0;
	spc->_modules_active[0] = 1;
 			
	ret = SPC_set_mode(mode, force_use, spc->_modules_active);
	if (ret < 0) {
		spc_check_error(spc, ret); 
		return ret;  
	}
	
	work_mode = SPC_get_mode();
	if (work_mode != mode) 
		return SPC_ERROR;
	
	return SPC_SUCCESS;
}

static int BH830_Simulation(Spc* spc)
{
	//If no hardware run in BH 830 simulation mode
	if (set_mode(spc, SPC_SIMUL830) != SPC_SUCCESS) {
		GCI_MessagePopup("BH Error", "Failed to set simulation mode");
		return SPC_ERROR;
	}
	
	return SPC_SUCCESS;
}

static int ForceUse(Spc* spc)
{
	//If the application is stopped or crashes the board is left in a locked state
	//attempt to set hardware mode for board 0 with force_use true
	if (set_mode(spc, SPC_HARD) != SPC_SUCCESS) {
		GCI_MessagePopup("BH Error", "Failed to take control of the B&H board.");
		return SPC_ERROR;
	}
	
	return SPC_SUCCESS;
}

static int BH_Read_eeProm(Spc* spc)
{
	SPC_EEP_Data eep_data;
	SPC_Adjust_Para adjpara;
	
	//Not really necessary as the EEPROM is read during BH_init and the adjust 
	//values are taken into account when the BH module registers are loaded.
	SPC_get_eeprom_data(spc->_active_module, &eep_data);
	SPC_get_adjust_parameters (spc->_active_module, &adjpara);
	
	return SPC_SUCCESS;
}
static int GetModuleInfo(Spc* spc)
{
	int i, err;
	SPCModInfo mod_info[MAX_NO_OF_SPC];
	
	//Get and display information for all modules present.
	spc->_no_modules = 0;
	spc->_no_active_modules = 0;
	
	for( i = 0; i < MAX_NO_OF_SPC; i++){
  		err = SPC_get_module_info(i, (SPCModInfo *)&mod_info[i]);
		spc_check_error(spc, err);
  		if (err) return err;
  		
  		if (mod_info[i].module_type != M_WRONG_TYPE) {
    		spc->_no_modules++;
   		}
  		if(mod_info[i].init == INIT_OK) {
    		spc->_no_active_modules++; 
    		spc->_modules_active[i] = 1;
    		spc->_active_module = i;
    	}
    	else {
      		spc->_modules_active[i] = 0;
		}
  	}
  	
	BH_Read_eeProm(spc);
	
	if (spc->_no_modules < 1) {
		GCI_MessagePopup("Error", "B&H board not found.");
		return SPC_ERROR;
	}
	if (spc->_no_active_modules < 1) { 		
		//Board may be present but locked. Try to commandeer it.
		if (ForceUse(spc) == SPC_ERROR) { 
			GCI_MessagePopup("Error", "B&H board locked by another process.");
			return SPC_ERROR;
		}

		return GetModuleInfo(spc);
	}

  	return SPC_SUCCESS;
}

static void OnScannerResolutionPreChanged(Scanner* scanner, int resolution, void *data)
{
	Spc* spc = (Spc*)data;

	// Need to adjust the left border of the spc card.
	// This is due to the lag (time) the scanners take to become linear.

	int left;

	scanner_get_spc_left_position (spc->_scanner, resolution, &left);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LEFT_BORDER,left);

	spc_send_sys_params(spc);
}

void spc_scanner_value_update(Scanner* scanner, int control, void *data)
{
	int x_shift, y_shift, zoom_index, zoom, scanner_speed, resolution_index, tmp;
	Spc* spc = (Spc*) data;

	spc->_spc_data->scan_size_x = spc->_spc_data->scan_size_y = scanner_get_resolution(scanner);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_X, spc->_spc_data->scan_size_x);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_Y, spc->_spc_data->scan_size_y);

	// We could have used the time window range here.
	// Ie a smaller time window we could have a bigger scan size but we
	// seem to count the photons in the max possible time window for the adc
	// spc_adapt_parameters_for_memory(spc);
	zoom = scanner_get_zoom(scanner);					//x1, x2, x5, x10, x20 or 0 = park
	
	if (zoom == SCANNER_ZOOM_PARK) {
		spc->_spc_data->scan_size_x = 1;
		spc->_spc_data->scan_size_y = 1;
		spc_update_params_panel(spc);
		bh_set_oscilloscope_mode(spc);
	}
	else {

		// Stop the measurement to give the user a chance to
		// set the roi that is now drawn on the window.
		if(control != SCAN_PNL_X_SHIFT && control != SCAN_PNL_Y_SHIFT && control != SCNCALPNL_HYST_OFFSET
			&& control != SCNCALPNL_X_OFFSET && control != SCNCALPNL_Y_OFFSET) {
			spc_stop(spc);
		}

		//line_scan = scanner_get_line_scan(scanner);			//1=line scan, 0=xy scan
		//if (line_scan) {
		//	spc->_scan_pattern = X_LINE_SCAN;
		//	spc->_spc_data->scan_size_y = 1;
		//}

		//spc_update_params_panel(spc);

		SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, BH_SCAN_MODE);
	
		if(bh_set_operation_mode(spc, BH_SCAN_MODE) == SPC_ERROR)
			return;
	}
			
	x_shift = scanner_get_x_shift(scanner);
	y_shift = scanner_get_y_shift(scanner);
	
	//Frame time may be affected
	spc_set_acq_limit(spc);

	scanner_get_zoom_index(spc->_scanner, &zoom_index);
	scanner_get_speed(spc->_scanner, &scanner_speed);
	scanner_get_resolution_index(spc->_scanner, &resolution_index);

	OnScannerResolutionPreChanged(spc->_scanner, spc->_spc_data->scan_size_x, spc);

	// Set the scanner controls on the spc panel to be the same as those on the scanner panel.
	
	
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_ZOOM, &tmp);

	if(zoom_index != tmp) 
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_ZOOM, zoom_index);
		
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_SPEED, &tmp);

	if(scanner_speed != tmp) 
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_SPEED, scanner_speed);
	
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_RESOLUTION, &tmp);

	if(resolution_index != tmp) 
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCANNER_RESOLUTION, resolution_index);
}


static void spc_get_effective_min_max_for_scanner_x_shift(Spc *spc, int *effective_min, int *effective_range)
{
	double range;

	range = (double) (spc->scanner_settings.max_x_shift - spc->scanner_settings.min_x_shift);
	*effective_range = (int) (range / spc->scanner_settings.displayed_image_scanner_zoom);		//	Real zoom when the current scanner zoom is taken into account

	*effective_min = (int) MIN(MAX(spc->scanner_settings.displayed_image_x_shift - ((double) *effective_range / 2.0), 0), 4095);
}

static void spc_get_effective_min_max_for_scanner_y_shift(Spc *spc, int *effective_min, int *effective_range)
{
	double range;

	range = (double) (spc->scanner_settings.max_y_shift - spc->scanner_settings.min_y_shift);
	*effective_range = (int) (range / spc->scanner_settings.displayed_image_scanner_zoom);		//	Real zoom when the current scanner zoom is taken into account

	*effective_min = (int) MIN(MAX(spc->scanner_settings.displayed_image_y_shift - ((double) *effective_range / 2.0), 0), 4095);
}

static void spc_get_display_image_width_and_height(Spc *spc, int *width, int * height)
{
	FIBITMAP *dib = GCI_ImagingWindow_GetDisplayedFIB(spc->_spc_window);

	*width = FreeImage_GetWidth(dib);
	*height = FreeImage_GetHeight(dib);
}

static int spc_set_scanner_shift_for_image_point(Spc *spc, SPC_SCANNER which_scanner, int pt)
{
	int scanner_range_min,  scanner_range_range;
	int x_shift, y_shift;
	int scanner_resolution;
	double center_percentage;
	
	spc_get_display_image_width_and_height(spc, &scanner_resolution, &scanner_resolution);

	center_percentage = (double) pt / scanner_resolution;

	if(which_scanner == SPC_SCANNER_X) {

		spc_get_effective_min_max_for_scanner_x_shift(spc, &scanner_range_min, &scanner_range_range);
		x_shift = (int) ((center_percentage * scanner_range_range) + scanner_range_min);
		spc->scanner_settings.x_shift = (int) x_shift;
	}
	else if(which_scanner == SPC_SCANNER_Y) {

		spc_get_effective_min_max_for_scanner_x_shift(spc, &scanner_range_min, &scanner_range_range);
		y_shift = (int) ((center_percentage * scanner_range_range) + scanner_range_min);
		spc->scanner_settings.y_shift = (int) y_shift;	
	}

	return SPC_SUCCESS;
}

static int spc_setup_scanner_shifts(Spc *spc, Point p1, Point p2)
{

	/*
	Point roi_center;
	int effective_x_shift_min,  effective_y_shift_min, effective_x_shift_range, effective_y_shift_range;
	int x_shift, y_shift;
	int scanner_resolution = scanner_get_resolution(spc->_scanner);
	int roi_size = p2.x - p1.x;	// roi is square so this is used for height as well
	double center_x_percentage, center_y_percentage;
	
	roi_center.x = p1.x + (roi_size / 2);
	roi_center.y = p1.y + (roi_size / 2);

	center_x_percentage = (double) (roi_center.x) / scanner_resolution;
	center_y_percentage = (double) (roi_center.y) / scanner_resolution;

	spc_get_effective_min_max_for_scanner_x_shift(spc, &effective_x_shift_min, &effective_x_shift_range);
	spc_get_effective_min_max_for_scanner_y_shift(spc, &effective_y_shift_min, &effective_y_shift_range);

	x_shift = (int) ((center_x_percentage * effective_x_shift_range) + effective_x_shift_min);
	y_shift = (int) ((center_y_percentage * effective_y_shift_range) + effective_y_shift_min);

	spc->scanner_settings.x_shift = (int) x_shift;
	spc->scanner_settings.y_shift = (int) y_shift;	

	return SPC_SUCCESS;
	*/

	Point roi_center;
	int roi_size = p2.x - p1.x;	// roi is square so this is used for height as well

	roi_center.x = p1.x + (roi_size / 2);
	roi_center.y = p1.y + (roi_size / 2);

	spc_set_scanner_shift_for_image_point(spc, SPC_SCANNER_X, roi_center.x);
	spc_set_scanner_shift_for_image_point(spc, SPC_SCANNER_Y, roi_center.y);

	return SPC_SUCCESS;
}

/*
static RECT spc_get_roi_top_left_from_scanner_shifts(Spc *spc, int roi_size, int x_shift, int y_shift)
{
	Point roi_center;
	RECT roi;
	int effective_x_shift_min,  effective_y_shift_min, effective_x_shift_range, effective_y_shift_range;
	int scanner_resolution = scanner_get_resolution(spc->_scanner);
	
	spc_get_effective_min_max_for_scanner_x_shift(spc, &effective_x_shift_min, &effective_x_shift_range);
	spc_get_effective_min_max_for_scanner_y_shift(spc, &effective_y_shift_min, &effective_y_shift_range);

	roi_center.x = (int)(((double)(x_shift - effective_x_shift_min) / effective_x_shift_range) * scanner_resolution);
	roi_center.y = (int)(((double)(y_shift - effective_y_shift_min) / effective_y_shift_range) * scanner_resolution);

	roi.left = (int) (roi_center.x - roi_size / 2.0);
	roi.top = (int) (roi_center.y - roi_size / 2.0);
	roi.right = (int) (roi.left + roi_size - 1);
	roi.bottom = (int) (roi.top + roi_size - 1);

	// Keep the roi in with the image
	if(roi.left < 0)
		OffsetRect (&roi, -roi.left, 0);

	if(roi.top < 0)
		OffsetRect (&roi, 0, -roi.top);

	if(roi.right > scanner_resolution)
		OffsetRect (&roi, -(roi.right - scanner_resolution), 0);

	if(roi.bottom > scanner_resolution)
		OffsetRect (&roi, -(roi.bottom - scanner_resolution), 0);

	return roi;
}
*/

static AdjustRectForImage(Spc *spc, RECT *roi)
{	
	int max_scanner_resolution_size;

	spc_get_display_image_width_and_height(spc, &max_scanner_resolution_size, &max_scanner_resolution_size);

	max_scanner_resolution_size -= 1;

	// Keep the roi in with the image
	if(roi->left < 0)
		OffsetRect (roi, -roi->left, 0);

	if(roi->top < 0)
		OffsetRect (roi, 0, -roi->top);

	if(roi->right > max_scanner_resolution_size)
		OffsetRect (roi, -(roi->right - max_scanner_resolution_size), 0);

	if(roi->bottom > max_scanner_resolution_size)
		OffsetRect (roi, 0, -(roi->bottom - max_scanner_resolution_size));

}

static void  OnSpcRoiChanged (IcsViewerWindow *window, const Point p1, const Point p2, void* data)
{
	Spc* spc = (Spc*) data;

	spc_setup_scanner_shifts(spc, p1, p2);
}

static void OnSpcRoiResizedOrMoved (IcsViewerWindow *window, void* data)
{
	Spc* spc = (Spc*) data;

	scanner_set_x_shift(spc->_scanner, (int) spc->scanner_settings.x_shift);
	scanner_set_y_shift(spc->_scanner, (int) spc->scanner_settings.y_shift);
}

static void OnSpcCrosshairMoved (IcsViewerWindow *window, const Point p1, const Point p2, void* data)
{
	Spc* spc = (Spc*) data;

	spc_set_scanner_shift_for_image_point(spc, SPC_SCANNER_X, p1.x);
	spc_set_scanner_shift_for_image_point(spc, SPC_SCANNER_Y, p1.y);

	scanner_set_x_shift(spc->_scanner, (int) spc->scanner_settings.x_shift);
	scanner_set_y_shift(spc->_scanner, (int) spc->scanner_settings.y_shift);
}

static void ShowCheckerBoardImage(Spc *spc, const char* text)
{
	// No previous image so we just display a checkerboard pattern.
	int old_zoom;
	int scanner_resolution = scanner_get_resolution(spc->_scanner);
	FIBITMAP *temp_dib = FreeImage_Allocate(scanner_resolution, scanner_resolution, 8, 0, 0, 0);

	if(text != NULL) {
		if(GCI_ConfirmPopup("SPC Image", IDI_INFORMATION, text)) {

			scanner_get_zoom_index(spc->_scanner, &old_zoom);
			spc->_ignore_scanner_signals = 1;
			scanner_set_zoom(spc->_scanner, SCANNER_ZOOM_X1);
			spc_start(spc);
			scanner_set_zoom(spc->_scanner, old_zoom);
			spc->_ignore_scanner_signals = 0;

			return;
		}
	}

	if(temp_dib == NULL) {

		GCI_MessagePopup("Error", "Error creating Freeimage Bitmap");
	}

	FIA_DrawGreyScaleCheckerBoard(temp_dib, 20);

	GCI_ImagingWindow_LoadImage(spc->_spc_window, temp_dib);

	FreeImage_Unload(temp_dib);
}

int spc_set_borders(Spc *spc, unsigned short left, unsigned short top)
{
	spc->_spc_data->scan_borders = top << 16 | left;

	return SPC_SUCCESS;
}


static void DisplayRoiBasedOnScannerOffsets(Spc *spc, int zoom)
{
	double factor;
	int roi_width, roi_height;
	RECT rect;
	Point p1, p2;
	int scanner_resolution;

	spc_get_display_image_width_and_height(spc, &scanner_resolution, &scanner_resolution);
	
	factor = ((double) zoom / ((double) spc->scanner_settings.displayed_image_scanner_zoom));

	roi_width = (int) (scanner_resolution / factor);
	roi_height = (int) (scanner_resolution / factor);

	p1.x = rect.left = (long)(((double) scanner_get_x_shift(spc->_scanner) / (spc->scanner_settings.max_x_shift - spc->scanner_settings.min_x_shift)) * scanner_resolution) - (roi_width/2.0);
	p2.x = rect.right = (long) (rect.left + roi_width - 1);

	p1.y = rect.top = (long)(((double)scanner_get_y_shift(spc->_scanner) / (spc->scanner_settings.max_y_shift - spc->scanner_settings.min_y_shift)) * scanner_resolution) - (roi_height/2.0);
	p2.y = rect.bottom = (long) (rect.top + roi_height - 1);    

	AdjustRectForImage(spc, &rect);

	p1.x = rect.left;
	p1.y = rect.top;
	p2.x = rect.right;
	p2.y = rect.bottom;

	GCI_ImagingWindow_PreventRoiResize(spc->_spc_window);
	GCI_ImagingWindow_SetRoiChangedHandler(spc->_spc_window, OnSpcRoiChanged, spc);   
	GCI_ImagingWindow_SetRoiMoveOrSizeChangeCompletedHandler(spc->_spc_window, OnSpcRoiResizedOrMoved, spc);
	GCI_ImagingWindow_EnableRoiTool(spc->_spc_window);
	GCI_ImagingWindow_SetROIImageRECT(spc->_spc_window, &rect); 
	GCI_ImagingWindow_DimRoiTool(spc->_spc_window, 1);

	spc_setup_scanner_shifts(spc, p1, p2);
}

static void DisplayCrosshairBasedOnScannerOffsets(Spc *spc)
{
	Point p1;
	int scanner_resolution;

	spc_get_display_image_width_and_height(spc, &scanner_resolution, &scanner_resolution);

	p1.x = (long)(((double) scanner_get_x_shift(spc->_scanner) / (spc->scanner_settings.max_x_shift - spc->scanner_settings.min_x_shift)) * scanner_resolution);
	p1.y = (long)(((double)scanner_get_y_shift(spc->_scanner) / (spc->scanner_settings.max_y_shift - spc->scanner_settings.min_y_shift)) * scanner_resolution);
	
	GCI_ImagingWindow_EnableCrossHair(spc->_spc_window);
	GCI_ImagingWindow_PlaceCrossHairAtImagePoint(spc->_spc_window, p1); 
}

static void spc_reset_displayed_image_data(Spc *spc)
{
	spc->scanner_settings.displayed_image_scanner_zoom = spc->roi_overview_image._scanner_zoom;
	spc->scanner_settings.displayed_image_x_shift = spc->roi_overview_image._scanner_shift.x;
	spc->scanner_settings.displayed_image_y_shift = spc->roi_overview_image._scanner_shift.y;
}

static void spc_goto_existing_lower_zoom(Spc *spc)
{
	FIBITMAP *dib = FreeImageIcs_LoadFIBFromIcsFilePath(spc->roi_overview_image._image_path);

	// We have an image with a greater field of view than the required zoom so we use that
	GCI_ImagingWindow_LoadImage(spc->_spc_window, dib);
	FreeImage_Unload(dib);
}

static void OnScannerZoomPreChanged(Scanner* scanner, int old_zoom, int new_zoom, void *data)
{
	double stage_x, stage_y;
	Spc* spc = (Spc*)data;
	XYStage *stage = NULL;
	
	if(spc->_spc_window == NULL)
		return;

	if(spc->_ignore_scanner_signals > 0)
		return;

	stage = microscope_get_stage(spc->_ms);
	
	if(stage != NULL)
		stage_get_xy_position(stage, &stage_x, &stage_y);

	// Maximum field of view for zoom 1 so we don't need a roi displayed.
//	if(new_zoom == 1 || new_zoom == SCANNER_ZOOM_PARK) {
//		if(spc->_spc_window != NULL) {
//			GCI_ImagingWindow_DisableRoiTool(spc->_spc_window);
			//GCI_ImagingWindow_EnableCrossHair(spc->_spc_window);
			//spc_get_scanner_center_point(spc, &(p1.x), &(p1.y));
			//GCI_ImagingWindow_PlaceCrossHairAtImagePoint(spc->_spc_window, p1);
//		}
//	}

	//if(new_zoom <= spc->scanner_settings.displayed_image_scanner_zoom) {

	//	int old_xshift = spc->scanner_settings.displayed_image_x_shift, old_yshift = spc->scanner_settings.displayed_image_y_shift;

		// The new zoom size is less than the old so we 
		// This means is has more field of view
		
		spc_reset_displayed_image_data(spc);


		// Check to see if we have a previously saved acquisition image with a larger field of view than the zoom
		// we want
		if((spc->roi_overview_image._scanner_res != scanner_get_resolution(spc->_scanner)) && (new_zoom != 1)) {

			// As the resolution has changed the saved image has been invalidated.
			ShowCheckerBoardImage(spc, "The cached image at zoom x1 is out of date\n"
										"because the resolution has changed.\n"
		                                "Acquire new image now ?");
		}
		else if ((fabs(spc->roi_overview_image.stage_x - stage_x) > 1.0 ||
			     fabs(spc->roi_overview_image.stage_y - stage_y) > 1.0) && (new_zoom != 1) ) {

			// As the stage position has changed the saved image has been invalidated.
			ShowCheckerBoardImage(spc, "The cached image at zoom x1 is out of date\n"
										"because the stage position has changed.\n"
		                                "Acquire new image now ?");
		}
		else if(new_zoom == SCANNER_ZOOM_PARK) {
			// really want a x1 zoom image for the park cross hair, use it if we have one 
			if (spc->roi_overview_image._scanner_zoom==1)
				spc_goto_existing_lower_zoom(spc);
			else
				ShowCheckerBoardImage(spc, NULL);
		}
		else if(spc->roi_overview_image._scanner_zoom > 0 && spc->roi_overview_image._scanner_zoom <= new_zoom) {
			// display a lower zoom image to put the new_zoom roi on.
			spc_goto_existing_lower_zoom(spc);
		}	
		else {

			// No previous image so we just display a checkerboard pattern.
			ShowCheckerBoardImage(spc, NULL);
		}
//	} 
		if(new_zoom == SCANNER_ZOOM_PARK) {
			GCI_ImagingWindow_DisableRoiTool(spc->_spc_window);	
			GCI_ImagingWindow_DimRoiTool(spc->_spc_window, 0);
			GCI_ImagingWindow_AllowRoiResize(spc->_spc_window);
			DisplayCrosshairBasedOnScannerOffsets(spc);			// NB ONLY WORKS ON ZOOM X1 SAVED IMAGE
		}
		else {
			GCI_ImagingWindow_DisableCrossHair(spc->_spc_window);	
			GCI_ImagingWindow_DimRoiTool(spc->_spc_window, 0);
			GCI_ImagingWindow_AllowRoiResize(spc->_spc_window);
			DisplayRoiBasedOnScannerOffsets(spc, new_zoom);			// NB ONLY WORKS ON ZOOM X1 SAVED IMAGE
		}

		if(new_zoom == 1) {
			GCI_ImagingWindow_DisableRoiTool(spc->_spc_window);
			GCI_ImagingWindow_DimRoiTool(spc->_spc_window, 0);
			GCI_ImagingWindow_AllowRoiResize(spc->_spc_window);
		}

	//|| spc->roi_overview_image._scanner_res >= scanner_get_resolution(spc->_scanner)
}

static int bh_init (Spc* spc)
{
	char spc_ini_filepath[GCI_MAX_PATHNAME_LEN] = "";
	int ret;
	
	spc->_simulation = 0;
	spc->_buffer = NULL;
	spc->_spc_data = NULL;
	spc->_oscilloscope_collect_time = 1.0;
	spc->_scan_collect_time = 0.1f;
	
    //Load panels and install callbacks such that spc is passed in the callback data
    if (bh_init_rates_panel(spc) == SPC_ERROR)
    	return SPC_ERROR;
    
    if (bh_init_params_panel(spc) == SPC_ERROR)
    	return SPC_ERROR;
    
  	spc->_timer = NewAsyncTimer (1.0, -1, 1, cbRateTimer, spc);
	SetAsyncTimerAttribute (spc->_timer, ASYNC_ATTR_ENABLED,  0);
	SetAsyncTimerName(spc->_timer, "Spc");

	if(spc_get_default_bh_ini_file_path(spc, spc_ini_filepath) < 0)
		return SPC_ERROR;

	ret = SPC_init(spc_ini_filepath);
	
	if(-ret == SPC_LICENSE_ERR) {
		
		// No icense can't even run in simulation mode
		return SPC_ERROR; 
	}
	
	else if (-ret == SPC_NO_ACT_MOD) {

		// No hardware found run in simulation
		if (BH830_Simulation(spc) == SPC_ERROR)
			return SPC_ERROR;
		
		spc->_simulation = 1;
	}
	else if(ret < 0) {
		
		if (spc_check_error(spc, ret) == SPC_ERROR) { 
			//Board may be present but locked. Try to commandeer it.
			if (ForceUse(spc) == SPC_ERROR) 
				return SPC_ERROR;
		}
	}
	
	if (GetModuleInfo(spc) == SPC_ERROR) 
		return SPC_ERROR;
	
	ret = SPC_get_module_info(spc->_active_module, &(spc->mod_info));
	spc_check_error(spc, ret);

	if(!ret)
		spc->_mod_info_found = 1;

	spc->_spc_data = (SPCdata *) malloc(sizeof(SPCdata));          
	
	ret = SPC_get_init_status(spc->_active_module);
	spc_check_error(spc, ret);

//	spc_load_default_params(spc);

	// Updated Spc values according to scanner values.
//	spc_scanner_value_update(spc->_scanner, -1, (void *) spc);

	spc->_spc_data->mode = -1;	// Make sure the following call to bh_set_oscilloscope_mode works;

//	ret = bh_set_oscilloscope_mode(spc);
//	spc_check_error(spc, ret);
	
//	if (ret)
//		return -1;

	if(spc->_simulation == 1) {

		SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_SCAN_X, ATTR_CTRL_MODE, VAL_HOT);
		SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_SCAN_Y, ATTR_CTRL_MODE, VAL_HOT);
	}
	
	// Lets allocate all the memory that the spc card has.
	// We have enough memory these days and if we want stage scan integration we will always need the max memory anyhow.
	spc->_buffer = (unsigned short *)malloc(SPC_BOARD_MAX_UNITS * sizeof(unsigned short));

  	return SPC_SUCCESS;
}


int bh_arm(Spc* spc)
{	
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MEAS_LED, 1);
	SPC_enable_sequencer (spc->_active_module, 0);

	return SPC_SUCCESS;
}


int bh_read_xy_results(Spc* spc)
{
	int err, nChans, i;
	unsigned short disp_page=1;
	int idx, j, k, x, y, chan, frame_size;
	unsigned short adc_resolution;
	FILE *fp = NULL;
	int fsize = 0;
	char datadir[GCI_MAX_PATHNAME_LEN]= "", fullpath[GCI_MAX_PATHNAME_LEN] = "";

	//How many channels are enabled?
	//Cannot have more than 8 with SPC 830 board
	nChans = min(8, spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y);
    
	//Read scan data to array
	err = SPC_read_data_page(spc->_active_module, disp_page-1, disp_page-1, spc->_buffer);
 
	spc_check_error(spc, err);

    if (err) {
    	return SPC_ERROR;
    }

	frame_size = spc->mem_info.blocks_per_frame * spc->mem_info.block_length;

    // Need to reduce all counts by a factor of CountIncrement
	if (spc->_spc_data->count_incr > 1) {

   		for (i=0; i<frame_size; i++)
   			spc->_buffer[i] /= spc->_spc_data->count_incr;
   	}		
    	
	if (!spc->_simulation) {

		return SPC_SUCCESS;
	}

	/*
	// Fake some data for testing
	adc_resolution = (unsigned short) pow(2.0, spc->_spc_data->adc_resolution);

	ui_module_get_data_dir(UIMODULE_CAST(spc), datadir);

	sprintf(fullpath, "%s\\spc_demo_3d.bin", datadir);

	if(FileExists(fullpath, &fsize) < 0) {
		GCI_MessagePopup("SPC Error", "Can not find demo spc data file");
		return SPC_ERROR;
	}

	if((fp = fopen(fullpath, "rb"))==NULL) {
		GCI_MessagePopup("SPC Error", "Can not open demo spc data file");
		return SPC_ERROR;
	}
  
	// read the entire array in one step
	if(fread(spc->_buffer, fsize, 1, fp) != 1) {
		GCI_MessagePopup("SPC Error", "Can not read demo spc data file");
		fclose( fp );
		return SPC_ERROR;
	}

	fclose( fp );
	*/

	//Fake some data for testing
	adc_resolution = (unsigned short) spc_get_max_value_for_adc_res(spc);

	y = spc->_spc_data->scan_size_y;
	x = spc->_spc_data->scan_size_x;
	
	for (i=0; i<y/2; i++) {
		for (j=0; j<x/2; j++) {
			for (k = 0; k < adc_resolution; k++) {
				idx = ((i * y + j) * spc->mem_info.block_length) + k;
				for (chan = 0; chan < nChans; chan++)
					spc->_buffer[idx+frame_size*chan] += (unsigned short)(10*exp(-k/(spc->mem_info.block_length/(5+chan))));
			}
		}
	}
	
	for (i=y/2; i<y; i++) {
		for (j=0; j<x/2; j++) {
			for (k = 0; k < adc_resolution; k++) {
				idx = ((i * y + j) * spc->mem_info.block_length) + k;
				for (chan = 0; chan < nChans; chan++)
					spc->_buffer[idx+frame_size*chan] += (unsigned short)(5*exp(-k/(spc->mem_info.block_length/(5+chan))));
			}
		}
	}

	return SPC_SUCCESS;
}

int bh_single_xy_acquisition(Spc* spc, double timeout)
{
	int ret, armed = 1, frames = 1;  // as in the old FLIM we do 1 frame at a time
	unsigned short bh_state=SPC_ARMED | SPC_MEASURE;
	double start_time;
	
    ret = SPC_start_measurement(spc->_active_module);
	spc_check_error(spc, ret);
	
	if (ret<0)
		return SPC_ERROR;
	
	start_time = Timer();

	//In XY mode start scanning at the last possible moment to avoid
	//unecessary exposure of the sample. No harm is done if it's already scanning.
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "AcqStart", GCI_VOID_POINTER, spc);  
	scanner_start_scan(spc->_scanner, frames);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: bh_single_xy_acquisition\n", time_str);
#endif
	while(armed) {
	
#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: bh_single_xy_acquisition loop\n", time_str);
#endif
		if(spc->_simulation == 0) {
			ret = SPC_test_state(spc->_active_module, &bh_state);
			spc_check_error(spc, ret);
		}		

		if (bh_state & SPC_ARMED) {  // 1 - still armed
		
        	if (Timer()-start_time > timeout) {
				logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC timed out"); 
				goto Error;
			}
        		
        	if ( (bh_state & SPC_MEASURE) == 0){
          		// system armed but collection not started because
          		// it is still waiting for Sync signals
          		continue;
          	}
        	
        	//ProcessSystemEvents();
			
			if(spc->_simulation == 1) {
				//simulate collection complete
				bh_state=0;
			}	
      	}
      	else {  //Not armed, collection complete. Stop scanning.
  
		//	scanner_stop_scan(spc->_scanner);
			ret = SPC_stop_measurement(spc->_active_module);
        	armed = 0;
      	}
	}

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: bh_single_xy_acquisition done\n", time_str);
#endif

	return ret;

Error:
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: bh_single_xy_acquisition errored\n", time_str);
#endif

	scanner_stop_scan(spc->_scanner);
	SPC_stop_measurement(spc->_active_module);

	return SPC_ERROR;
}

void print_spc_state(int state)
{
	printf("spc states: ");
	if (state & SPC_ARMED) printf("ARMED ");
	if (state & SPC_OVERFL) printf("OVERFL ");
	if (state & SPC_OVERFLOW) printf("VERFLOW ");
	if (state & SPC_TIME_OVER) printf("TIME_OVER ");
	if (state & SPC_COLTIM_OVER) printf("COLTIM_OVER ");
	if (state & SPC_CMD_STOP) printf("CMD_STOP ");
//	if (state & SPC_REPTIM_OVER) printf("REPTIM_OVER ");
	if (state & SPC_COLTIM_2OVER) printf("COLTIM_2OVER ");
//	if (state & SPC_REPTIM_2OVER) printf("REPTIM_2OVER ");
//	if (state & SPC_SEQ_GAP) printf("SEQ_GAP ");
//	if (state & SPC_FOVFL) printf("FOVFL ");
//	if (state & SPC_FEMPTY) printf("FEMPTY ");
	if (state & SPC_FBRDY) printf("FBRDY ");
	if (state & SPC_SCRDY) printf("SCRDY ");
	if (state & SPC_MEASURE) printf("MEASURE ");
	if (state & SPC_WAIT_TRG) printf("WAIT_TRG ");
	if (state & SPC_HFILL_NRDY) printf("HFILL_NRDY ");
	printf("\n");
}

static int CVICALLBACK update_power_monitor_val(void *callback)
{
	Microscope* microscope = (Microscope*) callback;
#ifdef BUILD_MODULE_LASER_POWER_MONITOR

	if(microscope->_laser_power_monitor != NULL)
		laserpowermonitor_get_and_display (microscope->_laser_power_monitor);
#endif
	return SPC_SUCCESS;   
}

int bh_multiple_xy_acquisition(Spc* spc, int frames, float *actual_collection_time)
{
	int ret, armed = 1;
	unsigned short bh_state=SPC_ARMED | SPC_MEASURE;
	double start_time, progress_time, timeout, collection_time;
	
	if(frames < 1)
		return SPC_SUCCESS;

    ret = SPC_start_measurement(spc->_active_module);
	spc_check_error(spc, ret);
	
	if (ret<0)
		return SPC_ERROR;

	timeout = 2*frames*spc->_frame_time; //should be plenty of time
	
	// collection time for the bh card, just long enough to get into the last frame, card will wait for end of current frame
	collection_time = (frames-1) * spc->_frame_time + 0.1;
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, collection_time);

	// send final spc settings
	spc_send_sys_params(spc);

	scanner_start_scan(spc->_scanner, frames);

	// trigger a measurement of laser power here
	CmtScheduleThreadPoolFunction (gci_thread_pool(), update_power_monitor_val, spc->_ms, NULL);

//	printf("bh_multiple_xy_acquisition\n");
	
	start_time = Timer();
	progress_time = Timer();

	while(armed) {
	
//		printf("bh_multiple_xy_acquisition loop\n");

		if(spc->_simulation == 0) {
			ret = SPC_test_state(spc->_active_module, &bh_state);
			spc_check_error(spc, ret);
		}		

		//printf("test state: armed? %d, overfl? %d, overflow? %d, time over? %d, coltime exp? %d, user stop? %d\n", 
		//	(bh_state & SPC_ARMED>0), (bh_state & SPC_OVERFL>0), (bh_state & SPC_OVERFLOW>0), (bh_state & SPC_TIME_OVER>0), (bh_state & SPC_COLTIM_OVER>0), (bh_state & SPC_CMD_STOP>0));

//		print_spc_state(bh_state);

		if (bh_state & SPC_ARMED) {  // 1 - still armed
		
        	if (Timer()-start_time > timeout) {
				logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC timed out"); 
				//goto Error;
	
//				printf("bh_multiple_xy_acquisition time out\n");
				ret = SPC_stop_measurement(spc->_active_module);
//				printf("SPC_stop_measurement %d\n", ret);
	        	armed = 0;
			}
			
			// if time elapsed since last progress update > frame time, do progress update
			if (Timer()-progress_time > spc->_frame_time){
				double progress;
				GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, &progress);
				SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, progress+1.0);
				progress_time = Timer();
			}

        	if ( (bh_state & SPC_MEASURE) == 0){
          		// system armed but collection not started because
          		// it is still waiting for Sync signals
          		continue;
          	}

			if(spc->_simulation == 1) {
				//simulate collection complete
				bh_state=0;
			}	
      	}
      	else {  //Not armed, collection complete. Stop scanning.
			ret = SPC_stop_measurement(spc->_active_module);
        	armed = 0;
      	}
	}

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: bh_multiple_xy_acquisition done\n", time_str);
#endif

//	SPC_test_state(spc->_active_module, &bh_state);
//	print_spc_state(bh_state);

	// if required get the measured duration of the scan, usually for the first frame, this is then used to calc scan/collection time for the rest of the frames 
	if (actual_collection_time!=NULL) {
		SPC_get_time_from_start(spc->_active_module, actual_collection_time);
		printf("SPC_get_time_from_start: %f - final\n", *actual_collection_time);
	}

	return ret;
/*
Error:
	
	printf("bh_multiple_xy_acquisition errored\n");

	scanner_stop_scan(spc->_scanner);
	SPC_stop_measurement(spc->_active_module);
	
//	SPC_test_state(spc->_active_module, &bh_state);
//	print_spc_state(bh_state);

	return SPC_ERROR;
*/
}

int bh_stop(Spc* spc)
{
	int ret;      
	
	ret = SPC_stop_measurement(spc->_active_module);
	spc_check_error(spc, ret);
	//spc_dim_sys_param_controls(spc, 0);  //Enable all controls

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "AcqStop", GCI_VOID_POINTER, spc);

	return SPC_SUCCESS;
}

float bh_get_tpp(Spc* spc)
{
	float tpp;
	
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_TIME_PER_CHAN, &tpp);		 //time per channel in ns
	
	return tpp;
}


void spc_on_change(Spc* spc)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "SpcChanged", GCI_VOID_POINTER, spc);  
}

////////////////////////////////////////////////////////////////////////////////////////
// Time Windows

/*
static int calcPileupThresholds(Spc* spc, double *fMild, double *fModerate, double *fSevere)
{
	double sync_rate, factor, dwellTime=1.0;
	int nChans;
	
	GetCtrlVal(spc->_pileup_ui_panel, SPC_PILEUP_SEVERE_VAL, fSevere);
	GetCtrlVal(spc->_pileup_ui_panel, SPC_PILEUP_MODERATE_VAL, fModerate);
	GetCtrlVal(spc->_pileup_ui_panel, SPC_PILEUP_MILD_VAL, fMild);
	
	//Threshold = sync rate * panel value * dwell time
	sync_rate = bh_get_sync_rate(spc);

	if (FP_Compare(sync_rate, 0.0) == 0) {

		*fSevere = spc->_max_count;
		*fModerate = spc->_max_count;
		*fMild = spc->_max_count;
	}
	else {
		//GCI_GetDwellTime(&dwellTime);
		nChans = spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y;
		
		if (nChans > 1)
			nChans = 4;
		
		factor = sync_rate * dwellTime/1000000.0 * spc->_current_frame / nChans;		//us to sec
		*fSevere *= factor;		
		*fModerate *= factor;	
		*fMild *= factor;		
	}
	
	return SPC_SUCCESS;
}
*/

static void SpcImageWindowCloseEventHandler( GCIWindow *window, void *callback_data )
{
	Spc* spc = (Spc *) callback_data;
	
	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "SpcImageWindowClose", GCI_VOID_POINTER, spc, GCI_VOID_POINTER, spc->_spc_window); 
	
    spc->_spc_window = NULL;
}

static void SpcImageWindowResizedorMovedEventHandler( GCIWindow *window, void *callback_data )
{
	int id;
	Spc* spc = (Spc *) callback_data;

	if( (id = GCI_ImagingWindow_GetPanelID(window)) != -1) {
	
		ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(spc), id, 1);
	}
}

/*
static void spc_save_3d_metadata(Spc* spc, const char *ics_filepath)
{
	Microscope *ms = microscope_get_microscope();    
	ICS *ics=NULL;
	char extension[10] = "";

	get_file_extension(ics_filepath, extension); 
	
	if(strcmp(extension, ".ics"))
		return;
	
	FreeImageIcs_IcsOpen(&ics, ics_filepath, "rw");  

	FreeImageIcs_IcsDeleteHistory(ics);  

	// add 3d specific stuff
	//dictionary_foreach(spc->_metadata_3d_additions, dictionary_ics_keyval_callback, ics);

	// also add the usual spc metedata
	dictionary_foreach(spc->_metadata, dictionary_ics_keyval_callback, ics);

	FreeImageIcs_IcsClose (ics);   
	
	return;
}
*/

void spc_save_3d_image_from_spc_data (Spc *spc, char *filepath, int prompt) 
{
	unsigned int x,y;
	int ret, dims[3];
	ICS *ics=NULL;
	float timebase, timebins;

	//Data from B&H is in "t x y" order, so that is how we save it.
	x = spc->_spc_data->scan_size_x;
	y = spc->_spc_data->scan_size_y;
	
	dims[0]= spc_get_max_value_for_adc_res(spc);
	dims[1]= x;
	dims[2]= y;
	ret = FreeImageIcs_SaveIcsDataToFile (filepath, spc->_buffer, Ics_uint16, 3, dims);

	FreeImageIcs_IcsOpen(&ics, filepath, "rw");  

	GCI_ImagingWindow_SaveMetaDataToIcsFile(spc->_spc_window, ics);

	//if(prompt)
	//	dictionary_foreach(spc->_scope_metadata, dictionary_ics_keyval_callback, ics);
	//else
	//	dictionary_foreach(spc->_metadata, dictionary_ics_keyval_callback, ics);

	timebase = spc->_spc_data->tac_range/spc->_spc_data->tac_gain * (float)1.0e-9;	//seconds
	timebins = (float)pow(2.0, spc->_spc_data->adc_resolution);
	FreeImageIcs_IcsSetNativeScale(ics, 0, 0.0,  timebase/timebins * (float)1.0e9, "ns");

	FreeImageIcs_IcsSetNativeScale(ics, 1, 0.0,  spc->_umPerPixel, "microns");
	FreeImageIcs_IcsSetNativeScale(ics, 2, 0.0,  spc->_umPerPixel, "microns");

	FreeImageIcs_IcsSetDimOrder(ics, 0, "t", "micro-time");
	FreeImageIcs_IcsSetDimOrder(ics, 1, "x", "x-position");
	FreeImageIcs_IcsSetDimOrder(ics, 2, "y", "y-position");
	
	FreeImageIcs_IcsClose (ics);   
	
	return;
}

static void spc_imagewindow_save_callback (IcsViewerWindow* window, void* callback_data) 
{
	Spc *spc = (Spc*) callback_data;
	int fsize = 0;
	char path[GCI_MAX_PATHNAME_LEN], filepath[GCI_MAX_PATHNAME_LEN];
	
	FIBITMAP *fib = NULL;

	// get the default location from the window
	GCI_ImagingWindow_GetDefaultDirectoryPath(spc->_spc_window, path);

	if (LessLameFileSelectPopup (window->panel_id, path, "*.ics", "*.ics;", "Save Image As", VAL_OK_BUTTON, 0, 0, 1, 1, filepath) <= 0)
		return;

	if(FileExists(filepath, &fsize)) {
		if(GCI_ConfirmPopup("Warning", IDI_WARNING, "File already exists.\nDo you wish to overwrite ?") == 0)
			return;
	}

	spc_save_3d_image_from_spc_data(spc, filepath, 0);
	

	// set this directory as default for all windows in the mscope
	GetDirectoryForFile(filepath, path);
	GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(path);

	return;
}

static int SetupImageWindow(Spc* spc)
{
	int id;

	if(spc->_spc_window == NULL) {
		
		if( (spc->_spc_window = GCI_ImagingWindow_CreateAdvanced2(UIMODULE_GET_NAME(spc), UIMODULE_GET_DESCRIPTION(spc), 300, 300, 500, 500, 0, 1, 0)) == NULL )
			return SPC_ERROR;
			
		GCI_ImagingWindow_Initialise(spc->_spc_window);
		
		GCI_ImagingWindow_SetCloseEventHandler( spc->_spc_window, SpcImageWindowCloseEventHandler, spc);
		
		GCI_ImagingWindow_SetResizedorMovedHandler( spc->_spc_window, SpcImageWindowResizedorMovedEventHandler, spc); 

		GCI_ImagingWindow_SetCrosshairHandler(spc->_spc_window, OnSpcCrosshairMoved, spc); 

		GCI_ImagingWindow_SetSaveFileProviderCallback(spc->_spc_window, spc_imagewindow_save_callback, spc);
	}
	
	GCI_ImagingWindow_SetWindowTitle( spc->_spc_window, UIMODULE_GET_DESCRIPTION(spc) );
	
	GCI_ImagingWindow_SetResizeFitStyle(spc->_spc_window);

	id = GCI_ImagingWindow_GetPanelID(spc->_spc_window); 
	
	//ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(spc), id, 0);

	return SPC_SUCCESS;
}


void spc_update_main_panel_parameters(Spc* spc)
{
	/* Hidden controls
	int which;
	
	SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_DATA_TYPE, VAL_FLOAT);
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_SEL, &which);
	if (which == 1) {//range
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MIN_VALUE, 50.0);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MAX_VALUE, 2000.0);
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_spc_data->tac_range);
	}
	else if (which == 2) {//gain
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_DATA_TYPE, VAL_SHORT_INTEGER);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MIN_VALUE, 1);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MAX_VALUE, 15);
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_spc_data->tac_gain);
	}
	else if (which == 3) {//offset
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MIN_VALUE, 0.0);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MAX_VALUE, 100.0);
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_spc_data->tac_offset);
	}	
	else if (which == 4) {//low limit
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MIN_VALUE, 0.0);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MAX_VALUE, 100.0);
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_spc_data->tac_limit_low);
	}	
	else if (which == 5) {//high limit
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MIN_VALUE, 0.0);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, ATTR_MAX_VALUE, 100.0);
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_spc_data->tac_limit_high);
	}
	*/

	if(spc->_old_adc_resolution != spc->_spc_data->adc_resolution) {
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, spc->_spc_data->adc_resolution);   
		spc->_old_adc_resolution = spc->_spc_data->adc_resolution;
	}

	spc_set_acq_limit(spc);
}

int spc_send_main_panel_parameters(Spc* spc)
{
	//When one of the params on the main panel is changed
	//update the system params panel and send everything

	// Hidden controls
	/*
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_SEL, &which);
	if (which == 1) //range
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_spc_data->tac_range);
	else if (which == 2) //gain
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_spc_data->tac_gain);
	else if (which == 3) //offset
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_spc_data->tac_offset);
	else if (which == 4) //low limit
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_spc_data->tac_limit_low);
	else if (which == 5) //high limit
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_spc_data->tac_limit_high);
    */

	//Add the additional TAC offset required for multi-channel operation
	//noChans = spc->_chans_enabled[0] + spc->_chans_enabled[1] + spc->_chans_enabled[2] + spc->_chans_enabled[3];
	//if (noChans > 1)  {
	//	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MC_TAC_OFFSET, &mc_tac_offset);
	//	spc->_spc_data->tac_offset += mc_tac_offset;
	//}
	
	spc->_spc_data->stop_on_time = 1;

	spc_update_params_panel(spc);
	
	return spc_send_sys_params(spc);
}

void spc_set_acq_limit_adv(Spc* spc, int acq_limit_type, int acq_limit_val)
{
	if(spc->_scanner == NULL)
		return;

	spc->_acq_limit_type = acq_limit_type;

	scanner_get_frame_time(spc->_scanner, &spc->_frame_time);

	if (acq_limit_type == SPC_ACQ_LIMIT_TYPE_FRAMES) {	//frames - show total time
		
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TEXTMSG_12, ATTR_VISIBLE, 1);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_ACQ_TIME, ATTR_VISIBLE, 1);
		spc->_acq_limit_val = acq_limit_val;
	    spc->_scan_time = spc->_acq_limit_val*spc->_frame_time;
		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_TIME, spc->_scan_time);
	}
	else if (acq_limit_type == SPC_ACQ_LIMIT_TYPE_SECONDS) {	//time  - seconds selected
		
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TEXTMSG_12, ATTR_VISIBLE, 0);
		SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_ACQ_TIME, ATTR_VISIBLE, 0);
//		spc->_acq_limit_val = (int) (acq_limit_val/spc->_frame_time);
//		spc->_scan_time =spc->_acq_limit_val*spc->_frame_time;
//		SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, Round(spc->_scan_time));  //time for whole number of frames
	}
	//else {  //counts 
		
	//	SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_TEXTMSG_12, ATTR_VISIBLE, 0);
	//	SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_ACQ_TIME, ATTR_VISIBLE, 0);
	//	spc->_acq_limit_val = acq_limit_val;
	//}	
}

void spc_set_acq_limit(Spc* spc)
{
	// What determines when we stop acquisition?
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, &spc->_acq_limit_type);
	GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, &spc->_acq_limit_val);
	
	spc_set_acq_limit_adv(spc, spc->_acq_limit_type, spc->_acq_limit_val);
}


int spc_initialise (Spc* spc)
{
	int zoom;

	if (bh_init (spc) == SPC_ERROR) {

		logger_log(UIMODULE_LOGGER(spc), LOGGER_ERROR,
			"Error initialising spc board. Make sure the software and hardware is present"); 

		return SPC_ERROR;
	}

	if (spc->_scanner != NULL) {
		scanner_changed_handler_connect(spc->_scanner, spc_scanner_value_update, spc);
		scanner_zoomed_pre_changed_handler_connect(spc->_scanner, OnScannerZoomPreChanged, spc);  
		scanner_resolution_pre_changed_handler_connect(spc->_scanner, OnScannerResolutionPreChanged, spc);
	
		scanner_get_min_max_x_shift (spc->_scanner, &(spc->scanner_settings.min_x_shift), &(spc->scanner_settings.max_x_shift));
		scanner_get_min_max_x_shift (spc->_scanner, &(spc->scanner_settings.min_y_shift), &(spc->scanner_settings.max_y_shift));
	}

	// Load default params from file
	spc_load_default_params(spc);
	

	spc->_spc_data->scan_size_x = spc->_spc_data->scan_size_y = scanner_get_resolution(spc->_scanner);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_X, spc->_spc_data->scan_size_x);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_Y, spc->_spc_data->scan_size_x);

	zoom = scanner_get_zoom(spc->_scanner);

	// Make sure left border is setup correctly. According to scanner values
//	OnScannerResolutionPreChanged(spc->_scanner, spc->_spc_data->scan_size_x, spc);
	spc_scanner_value_update(spc->_scanner, 0, spc);

	if(zoom == 0) {
		bh_set_oscilloscope_mode(spc);
	}
	else {
		if(bh_set_operation_mode(spc, BH_SCAN_MODE) == SPC_ERROR)
			return SPC_ERROR;
	}

	spc_set_acq_limit(spc);

	spc_update_main_panel_parameters(spc);     
	
	spc->_initialised = 1;

  	return SPC_SUCCESS;
}

void spc_set_error_handler(Spc* spc, UI_MODULE_ERROR_HANDLER handler)
{
	ui_module_set_error_handler(UIMODULE_CAST(spc), handler, spc);
}

static void OnSpcPanelDisplayed (UIModule *module, int panel_id, void *data)
{
	Spc *spc = (Spc*) data;
	
	if(spc->_rates_ui_panel == panel_id) {
		enable_rate_count_timer(spc);
	}	
}

Spc* spc_new(Microscope *ms, char *name, char *description, UI_MODULE_ERROR_HANDLER handler, char *data_dir)
{
	int window_handle;
	
	Spc* spc = (Spc*) malloc(sizeof(Spc));
	
	memset(spc, 0, sizeof(Spc));
	
	GciCmtNewLock ("Spc", 0, &(spc->_lock) );  
	
	ui_module_constructor(UIMODULE_CAST(spc), name);
	ui_module_set_description(UIMODULE_CAST(spc), description);
	ui_module_set_error_handler(UIMODULE_CAST(spc), handler, spc);
	ui_module_set_data_dir(UIMODULE_CAST(spc), data_dir);
	
	spc->_ms = ms;
	spc->_camera = microscope_get_camera(spc->_ms); 
	spc->_scanner = microscope_get_scanner(spc->_ms);   
	spc->_filter_set = microscope_get_filter_set(spc->_ms);   

	spc->_mod_info_found = 0;
	spc->_ignore_scanner_signals = 0;
	spc->_initialised = 0;
	spc->_old_adc_resolution = -1.0;
	spc->_frame_time = 1.0;
	spc->_graph_ui_panel = -1;
	spc->_spc_window = NULL;
	spc->oscilloscope_settings._x_data = NULL;
	spc->_max_count = 0;
	spc->_max_scope_count = 3000;
	spc->_scan_max_count = 255;
	spc->_max_scale_for_tau = 5000;
	spc->_umPerPixel = 1.0;
	spc->_time_window_min = 1;
	spc->_acquire_finished = 1;
	spc->_change_palette_for_filter = 0;

	memset(&(spc->roi_overview_image), 0, sizeof(HighestResImage));

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(spc), "SpcChanged", SPC_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(spc), "AcqStart", SPC_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(spc), "AcqStop", SPC_PTR_MARSHALLER);
	
    if ((spc->_graph_ui_panel = ui_module_add_panel(UIMODULE_CAST(spc), "spc_ui.uir", SPC_GRAPH, 0)) < 0)   
		return NULL;   

	 //Load panels and install callbacks such that spc is passed in the callback data
    if (spc_init_main_panel(spc) == SPC_ERROR)
    	return NULL;      
    
	if ( InstallCtrlCallback (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, cbConfigGraph, spc) < 0)
		return NULL;

	if ( InstallCtrlCallback (spc->_graph_ui_panel, SPC_GRAPH_AUTOSCALE, cbConfigGraph, spc) < 0)
		return NULL;

	if ( InstallCtrlCallback (spc->_graph_ui_panel, SPC_GRAPH_OS_CLOSE, OnOscilloscopeClosed, spc) < 0)
		return NULL;
	
	if(ui_module_panel_show_handler_connect (UIMODULE_CAST(spc), OnSpcPanelDisplayed, spc) == UI_MODULE_ERROR)
		return NULL;

	GetPanelAttribute (spc->_main_ui_panel, ATTR_SYSTEM_WINDOW_HANDLE, &(window_handle)); 
	spc->window_hwnd = (HWND) window_handle; 

	spc->uimodule_func_ptr = ui_module_set_window_proc(UIMODULE_CAST(spc), spc->_main_ui_panel, (LONG_PTR) SpcWndProc);	

	if( SetupImageWindow(spc) == SPC_ERROR) {
		send_spc_error_text(spc,"Can not create image window for device %s\n", UIMODULE_GET_DESCRIPTION(spc) );
		return NULL; 
	}

	return spc;
}


int spc_destroy(Spc* spc)
{
	int i, err, mode = SPC_HARD, force_use = 0;
	
	spc_stop(spc);
	
	spc_save_default_params(spc);

	disable_rate_count_timer(spc);
	
	//Free board for use by other applications
	for( i =0 ; i < MAX_NO_OF_SPC; i++)
		spc->_modules_active[i] = 0;

	err = SPC_set_mode(mode, force_use, spc->_modules_active); 
	err = SPC_close(); 
	spc_check_error(spc, err); 

	if (spc->_spc_data != NULL) {
  		free(spc->_spc_data);
  		spc->_spc_data = NULL;
	}

	if (spc->_buffer) {
		free(spc->_buffer);
		spc->_buffer = NULL;
	}

	if(spc->_metadata != NULL)
		dictionary_del(spc->_metadata);

	ui_module_destroy_panel(UIMODULE_CAST(spc), spc->_graph_ui_panel);
	ui_module_destroy_panel(UIMODULE_CAST(spc), spc->_rates_ui_panel);
	ui_module_destroy_panel(UIMODULE_CAST(spc), spc->_params_ui_panel);

	ui_module_destroy(UIMODULE_CAST(spc));

  	CmtDiscardLock (spc->_lock);
  	
  	free(spc);
	
  	return SPC_SUCCESS;
}



void spc_dim_controls(Spc* spc, int dimmed)
{
	if(dimmed)
		ui_module_disable_panel(spc->_main_ui_panel, 8, SPC_MAIN_STOP, SPC_MAIN_ADVANCED, SPC_MAIN_PROGRESS, SPC_MAIN_MEAS_LED, SPC_MAIN_SYNC_LED, SPC_MAIN_STATUS, SPC_MAIN_ACC_TIME, SPC_MAIN_ACC_FRAMES); 
	else
		ui_module_enable_panel(spc->_main_ui_panel,0); 
}


int spc_stop(Spc* spc)
{
	double start_time = Timer();

	if(spc == NULL || spc->_initialised == 0)
		return SPC_SUCCESS;

	spc->_acquire = 0;
	bh_stop(spc);

	while(spc->_acquire_finished == 0 && ((Timer() - start_time) < 20.0)) {

		Delay(0.1);
		ProcessSystemEvents();
	}

	if((Timer() - start_time) > 20.0) {
		logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC stop timed out"); 
	}

	spc_stop_scope(spc);

	return SPC_SUCCESS;
}


static void SetDefaultDisplay(Spc* spc, int scan_size_x, int scan_size_y, float* intensity_array)
{
	int chan, n, *hist, maxi, mini, bins, interval=1; 
	double *temp_array, *x=NULL, min_intensity, max_intensity;
	
	// Set an intelligent value for max count in order to get a nice display
	// i.e. if there a a few very bright pixels we want to ignore them
	
	if (spc->_spc_data->mode  != BH_SCAN_MODE)
		return;  // Do for XY scan only
	
	n = scan_size_x * scan_size_y;
	temp_array = (double *)calloc(n, sizeof(double));
	
	//Get the histogram for the first enabled channel
	for (chan = 0; chan < 4; chan++) {

		if (spc->_chans_enabled[chan]) {
		
			ConvertArrayType (intensity_array, VAL_FLOAT, temp_array, VAL_DOUBLE, n);
			MaxMin1D (temp_array, n, &max_intensity, &maxi, &min_intensity, &mini);
			bins = (int) (max_intensity-min_intensity+1);
			
			x = (double *)calloc(bins, sizeof(double));
			
			if(min_intensity == max_intensity)
				break;

			hist = (int *)calloc(bins, sizeof(int));
			Histogram (temp_array, n, min_intensity, max_intensity, hist, x, bins);
			
			maxi = bins-5;
			if (maxi < 0) return;
			while (hist[maxi] < 5) {
				maxi--;
				if (maxi < 0) return;
			}
			maxi *= interval;
			
			spc->_max_count = (unsigned short)(maxi+min_intensity);
			//SetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, spc->_max_count);
			
			free(x);
			free(hist);
			x = NULL;
			hist = NULL;

			break;
		}
	}

	free(temp_array);
	free(x);
}


static int spc_display_image(Spc* spc, FIBITMAP *dib)
{
	FilterSet fs;
	dictionary *d;

	spc_fov(spc);	
	
	// change window palette if the filter has changed since last display
	if (spc->_change_palette_for_filter == 1){
		if(spc->_spc_window != NULL) {  
			filterset_get_current_filterset(spc->_filter_set, &fs);
			GCI_ImageWindow_SetFalseColourWavelength(spc->_spc_window, fs.emm_min_nm);        
			spc->_change_palette_for_filter = 0;
		}
	}

	// Ok we have a larger acquired image than before so we save it here
	// for display when setting the roi
	if(spc->roi_overview_image._scanner_zoom == 0 || scanner_get_zoom(spc->_scanner) <= spc->roi_overview_image._scanner_zoom) {

		XYStage *stage = NULL;
		char temp_dir[GCI_MAX_PATHNAME_LEN] = "";

		// Temporary image path.
		microscope_get_temp_directory(temp_dir);

		strcpy(spc->roi_overview_image._image_path, temp_dir);
		strcat(spc->roi_overview_image._image_path, "\\highest_zoom_trspc.ics");

		FreeImageIcs_SaveImage(dib, spc->roi_overview_image._image_path, 1);

		spc->roi_overview_image._scanner_zoom = scanner_get_zoom(spc->_scanner);
		spc->roi_overview_image._scanner_res = scanner_get_resolution(spc->_scanner);

		stage = microscope_get_stage(spc->_ms);
		if(stage != NULL)
			stage_get_xy_position(stage, &(spc->roi_overview_image.stage_x), &(spc->roi_overview_image.stage_y));
	}

	GCI_ImagingWindow_LoadImage(spc->_spc_window, dib);
	d = microscope_get_flim_image_metadata (spc->_ms, spc->_spc_window);
	GCI_ImagingWindow_SetMetaData(spc->_spc_window, d);
	GCI_ImagingWindow_Show(spc->_spc_window);

	FreeImage_Unload(dib);

	return SPC_SUCCESS;
}


// Only call from main thread
#pragma optimize("f",on)	// Optimise this function for speed
static int read_and_make_image(Spc* spc)
{
	int max_t;
	register int t, x, y;
	register float *dst_ptr = NULL;
	register float decay_total = 0.0f;
	float total = 0.0;  
	unsigned short *buffer_ptr = spc->_buffer;
	FIBITMAP *fib = NULL;
	int dst_width = spc->_spc_data->scan_size_x;
	int dst_height = spc->_spc_data->scan_size_y;

	GciCmtGetLock(spc->_lock);

	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_STATUS, "Reading data");
	
	if (bh_read_xy_results(spc) == SPC_ERROR)
		goto Error;

	fib = FreeImage_AllocateT(FIT_FLOAT, dst_width, dst_height, 32, 0, 0, 0);

	if(fib == NULL) {

		GCI_MessagePopup("SPC Error", "Failed to create image");
		goto Error;
	}

	max_t = spc_get_max_value_for_adc_res(spc);

	for(y = 0; y < dst_height; y++)
	{
		dst_ptr = (float *) FreeImage_GetScanLine (fib, dst_height - y - 1);

		for(x = 0; x < dst_width; x++)
		{
			decay_total = 0.0f;

			for (t=0; t < spc->_time_window_min; t++) {
				decay_total += (float) (*buffer_ptr++);   // Total photon counts
			}

			// Only include counts in the required time window in displayed image
			for (t = spc->_time_window_min; t <= spc->_time_window_max; t++) {
				
				dst_ptr[x] += ((float) *buffer_ptr);
				
				decay_total += (float) (*buffer_ptr++);
			}

			for(t=spc->_time_window_max+1; t < max_t; t++) {
				decay_total += (float) (*buffer_ptr++);
			}

			total = MAX(total, decay_total);	   // Max photon count in this data block
		}
	}

	spc->_max_count = (int) total;	//for pile-up detection

	if(spc->_spc_window != NULL) {
		GCI_ImagingWindow_DisableRoiTool(spc->_spc_window); 
		GCI_ImagingWindow_DimRoiTool(spc->_spc_window, 0);
		GCI_ImagingWindow_AllowRoiResize(spc->_spc_window);
    }
    
	//You have to display it before changing the linear scale
	spc_display_image(spc, fib);

	//if (spc->_autorange) {

		//SetDefaultDisplay(spc);
		//You have to re-display it after changing the linear scale
		//spc_display_image(spc);
	//}

	GciCmtReleaseLock(spc->_lock);

	return SPC_SUCCESS;

Error:

	if(fib != NULL)
		FreeImage_Unload(fib);

	GciCmtReleaseLock(spc->_lock);

	return SPC_ERROR;
}
#pragma optimize("f",off)

static int spc_pixel_count_limit_reached(Spc* spc)
{
	int chan;

	//Get mean or max count per pixel and compare to our maximum limit.
	for (chan = 0; chan < 4; chan++) {
		if (spc->_chans_enabled[chan]) {
		
			if (spc->_acq_limit_type == 3) {	//Limit max count
					return 1;	//limit reached
			}
			else {	//Limit mean count
					return 1;	//limit reached
			}
		}
	}
	return 0;	//limit not reached
}


static int spc_acquire_image(Spc* spc, AcquireParamData *data)
{
	char msg[100];
	double start_time, last_displayed_time, timeout;
	int repeat_no = 0;

	data->current_frame = 1;
	
	start_time = Timer();
	last_displayed_time = Timer();
	
	spc->_acquire = 1;

	// Clear memory unless we want to accumulate.
	if (data->accumulate == 0) {
		if (bh_clear_memory(spc) != SPC_SUCCESS)
			goto Error;
	}

	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, 0.0);
	//SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_PROGRESS, ATTR_MAX_VALUE, (double) data->limit_val);

	while (1) {

		if (spc->_acquire == 0)	{   //User has aborted
			break;
		}

		if (bh_enable_sequencer(spc, 1) == SPC_ERROR)
			goto Error;
				
		timeout = 2*spc->_frame_time; //should be plenty of time
		
		if(spc->_simulation == 0) {

			if (bh_single_xy_acquisition(spc, timeout) == SPC_ERROR)
				goto Error;
		}
		     
		// Is it time to stop?
		// number_of_frames
		if (data->limit_type == SPC_ACQ_LIMIT_TYPE_FRAMES) {
		
			sprintf(msg, "Frame %d of %d.", data->current_frame, data->limit_val);
   			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_STATUS, msg);

			SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, ((double) data->current_frame / data->limit_val) * 10.0);

			if (data->current_frame == data->limit_val) {
				break;
			}
		}
		else if(data->limit_type == SPC_ACQ_LIMIT_TYPE_SECONDS) {
			
			double seconds_passed = Timer() - start_time;
			
			SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, (seconds_passed / data->limit_val) * 10.0);

			if(seconds_passed >= data->limit_val) {
				break;
			}
		}
		else {
	   		if (spc_pixel_count_limit_reached(spc)) 
	   			break;
		}

		if (data->current_frame >= 1 && data->first_frame_displayed == 0) {   // Always display first frame
		
				SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  

				//SendMessage(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG, 0, 0);

				data->first_frame_displayed = 1;
				last_displayed_time = Timer();
		}

		if (data->should_display && ((Timer()- last_displayed_time) >= data->display_time)) {

				SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  

				//SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG, 0, 0);
				last_displayed_time = Timer();
    	}

		data->current_frame++;
		Delay(0.2);
		//ProcessSystemEvents();
	}

	Delay(0.5);
	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, 0.0);
	
	// Always update on last acquire
	SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  
	
	//SendMessage(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG, 0, 0);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%d: spc_acquire_image done\n", time_str);
#endif

	return SPC_SUCCESS;

Error:

	printf("spc_acquire_image errored\n");

	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, 0);

	return SPC_ERROR;
}

void set_acc_ui (Spc *spc)
{
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACC_FRAMES, spc->_acc_frames);
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACC_TIME, spc->_acc_time);
}

// Acquire function for new scanners. Where a specific number of frames can be set and
// scanned continuously.
static int spc_acquire_image2(Spc* spc, AcquireParamData *data)
{
	double start_time, last_displayed_time;
	int repeat_no = 0, nFrames, display_increment;
	dictionary *d;
	float frame_time;

	data->current_frame = 0;
	
	start_time = Timer();
	last_displayed_time = Timer();
	
	spc->_acquire = 1;

	// Clear memory unless we want to accumulate.
	if (data->accumulate == 0) {
		// reset accumilation counters
		spc->_acc_frames = 0;
		spc->_acc_time = 0.0;
		set_acc_ui(spc);

		if (bh_clear_memory(spc) != SPC_SUCCESS)
			goto Error;
	}

	// to reset the rate counters to try to get reasonable values in the metadata
	SPC_clear_rates(spc->_active_module);
	

	//printf("%s Here1\n", __FUNCTION__);

	if (bh_enable_sequencer(spc, 1) == SPC_ERROR)
		goto Error;
	
	// Assume for now data->limit_type == SPC_ACQ_LIMIT_TYPE_FRAMES
	
	//printf("%s Here2\n", __FUNCTION__);

	if(data->limit_type == SPC_ACQ_LIMIT_TYPE_SECONDS) {
		nFrames = (int)floor(data->limit_val/spc->_frame_time);
	}
	else {
		nFrames = data->limit_val;
	}

	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, 0.0);
	SetCtrlAttribute (spc->_main_ui_panel, SPC_MAIN_PROGRESS, ATTR_MAX_VALUE, (double)nFrames);

	//printf("%s Here3\n", __FUNCTION__);

	if(spc->_simulation == 0) {

		get_time_string(time_str);
		printf("%s: calc frame time %f\n", time_str, spc->_frame_time);

		if (bh_multiple_xy_acquisition(spc, 1, &frame_time) == SPC_ERROR)
			goto Error;
//		spc->_frame_time = (double)frame_time;     // Uncomment this line to use the measured frame time from the SPC board, calculated time is now more accurate (17/03/10)
		printf("%s: meas frame time %f\n", time_str, frame_time);

		// increment accumilation counters
		spc->_acc_frames++;
		spc->_acc_time += spc->_frame_time;
		set_acc_ui(spc);

		// get metadata now so the spc rates are reasonable
		bh_read_rates (spc);
		d = microscope_get_flim_image_metadata (spc->_ms, spc->_spc_window);     
		GCI_ImagingWindow_SetMetaData(spc->_spc_window, d);

		//printf("%s Here4\n", __FUNCTION__);

		// Display the acquired image
		SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL); 

		if(GetLastError() == ERROR_TIMEOUT) {
			logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, __FUNCTION__ " Timed out with SendMessageTimeout"); 
		}

		//printf("%s Here5\n", __FUNCTION__);

		data->current_frame++;
		SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, (double)data->current_frame);

		if(nFrames == 1)
			return SPC_SUCCESS;

		if (data->should_display) {
			display_increment = (int)floor(data->display_time / spc->_frame_time);
			if (display_increment < 1) 
				display_increment = 1;
		}
		else {
			display_increment = nFrames;
		}
		
		while(data->current_frame <= (nFrames - display_increment)) {

			//printf("%s Here6\n", __FUNCTION__);

			if (spc->_acquire == 0)	{   //User has aborted
				break;
			}

			if (bh_multiple_xy_acquisition(spc, display_increment, NULL) == SPC_ERROR)
				goto Error;

			//printf("%s Here7\n", __FUNCTION__);

			// increment accumilation counters
			spc->_acc_frames += display_increment;
			spc->_acc_time += display_increment*spc->_frame_time;
			set_acc_ui(spc);

			// get metadata again, it has changed
			d = microscope_get_flim_image_metadata (spc->_ms, spc->_spc_window);     
			GCI_ImagingWindow_SetMetaData(spc->_spc_window, d);

			// Display the acquired image
			SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
						0, 0, SMTO_ABORTIFHUNG, 2000, NULL); 

			if(GetLastError() == ERROR_TIMEOUT) {
				logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, __FUNCTION__ " Timed out with SendMessageTimeout"); 
			}

			//printf("%s Here8\n", __FUNCTION__);

			data->current_frame+=display_increment;
			SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, (double)data->current_frame);
		}

		if (spc->_acquire){ // make sure user has not aborted
			int last_frames = nFrames - data->current_frame;

			if (bh_multiple_xy_acquisition(spc, last_frames, NULL) == SPC_ERROR)
				goto Error;

			// increment accumilation counters
			spc->_acc_frames += last_frames;
			spc->_acc_time += last_frames*spc->_frame_time;
			set_acc_ui(spc);

			// get metadata again, it has changed
			d = microscope_get_flim_image_metadata (spc->_ms, spc->_spc_window);     
			GCI_ImagingWindow_SetMetaData(spc->_spc_window, d);
		}
	}

	//printf("%s Here9\n", __FUNCTION__);

	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, (double)nFrames);
	
	// Always update on last acquire
	SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  

	if(GetLastError() == ERROR_TIMEOUT) {
		logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, __FUNCTION__ " Timed out with SendMessageTimeout"); 
	}

	//printf("%s Sucess\n", __FUNCTION__);

	return SPC_SUCCESS;

Error:

	printf("spc_acquire_image errored\n");

	SetCtrlVal (spc->_main_ui_panel, SPC_MAIN_PROGRESS, 0);

	return SPC_ERROR;
}

LRESULT CALLBACK SpcWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	Spc *spc = (Spc *) data;

	switch(message)
	{		
    	case SPC_DISPLAY_IMAGE_MSG:
    	{
			if(spc->_acquire == 0 || spc->_acquire_finished == 1)
				return 0;

			if (read_and_make_image(spc) == SPC_ERROR)
				return 0;	

			return 1;
		}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) spc->uimodule_func_ptr,
							hwnd, message, wParam, lParam);
}

int CVICALLBACK spc_start_measurement_thread(void *callback)
{
	AcquireParamData *data = (AcquireParamData*) callback;
	Spc *spc = data->spc;
	char fullpath[GCI_MAX_PATHNAME_LEN] = "";
	int repeat_no = 0;
	double start_time = 0.0;

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: started spc_start_measurement_thread spc->_acquire_finished %d\n", time_str, spc->_acquire_finished);
#endif

	if(spc->_acquire_finished == 1)
		return SPC_SUCCESS;

#ifdef VERBOSE_DEBUG
	printf("spc_start_measurement_thread 1\n");

	printf("spc_start_measurement_thread 2\n");
#endif

	data->first_frame_displayed = 0;

	do {

#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: spc_start_measurement_thread\n", time_str);
#endif
		if(spc->_acquire == 0)
			break;

		spc->_waiting_for_next_repeat = 0;

		data->cycle_start_time = Timer();
	
		start_time = Timer();

	//	if(spc_acquire_image(spc, data) == SPC_ERROR) {
	//		goto Error;
	//	}
		
		// NEW CONTINUOUS SCANNER TEST IS HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if(spc_acquire_image2(spc, data) == SPC_ERROR) {
			goto Error;
		}

#ifdef VERBOSE_DEBUG
		get_time_string(time_str);
		printf("%s: Total time taken %f\n", time_str, Timer() - start_time);
#endif

		if(data->passed_in_filepath == 1) {

			//SendMessage(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG, 0, 0);
			SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  

			if(data->is_3d == 1)
				spc_save_3d_image_from_spc_data (spc, data->output_filename, 0);
		}
		else if (data->autosave) {
		
			sprintf(fullpath, "%s%s%d%s", data->output_dir, data->output_filename, repeat_no++, data->output_filename_ext);

			//SendMessage(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG, 0, 0);
			SendMessageTimeout(spc->window_hwnd, SPC_DISPLAY_IMAGE_MSG,
					0, 0, SMTO_ABORTIFHUNG, 2000, NULL);  

			if (fullpath != NULL) {
				if(data->is_3d == 1)
					spc_save_3d_image_from_spc_data (spc, fullpath, 0);      
			}
		}

		// Wait until the repeat time comes around
		if(data->repeat == 0)
			break;

//		printf("%s: %s Here10\n", time_str, __FUNCTION__);

		while ((Timer() - data->cycle_start_time) < data->repeat_time) {

			//printf("Stuck here1 spc->_acquire_finished %d\n", spc->_acquire_finished);

			spc->_waiting_for_next_repeat = 1;

			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_STATUS, "Waiting to repeat");
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MEAS_LED, 0);     

			//ProcessSystemEvents();

			if (spc->_acquire == 0)	{   // User has aborted				
				goto Error;
			}
		}	

		//printf("Stuck here2 spc->_acquire_finished %d\n", spc->_acquire_finished);
		Delay(0.1);
		//ProcessSystemEvents();
	}
	while(1);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread finished\n", time_str);
#endif

	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_STATUS, "Collection complete");

	bh_enable_sequencer(spc, 0);
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread here1\n", time_str);
#endif

	spc_dim_controls(spc, 0);  //Enable all controls

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("spc_start_measurement_thread here2\n", time_str);
#endif

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "AcqStop", GCI_VOID_POINTER, spc);
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread here3\n", time_str);
#endif

	if(spc->_scanner != NULL)
		scanner_stop_scan(spc->_scanner);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread here4\n", time_str);
#endif

	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MEAS_LED, 0);     
	
	logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC stopped acquisition"); 
	
	spc->_acquire_finished = 1;			
	spc->_acquire = 0;

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread here5\n", time_str);
#endif

	return SPC_SUCCESS;

Error:

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: spc_start_measurement_thread finished\n", time_str);
#endif
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MEAS_LED, 0);  

	bh_enable_sequencer(spc, 0);
	
	spc_dim_controls(spc, 0);		// Un-dim controls
	spc->_acquire_finished = 1;			

#ifdef VERBOSE_DEBUG
	printf("spc_start_measurement_thread here errored _acquire_finished = 1\n");
#endif

	return SPC_ERROR;
}

int spc_start_advanced(Spc* spc, SPC_ACQUISITION_LIMIT_TYPE limit_type, int acq_limit_val, int repeat, float repeat_time, int should_display, double display_time,
					   int accumulate, int autosave, int image_3d, const char *filepath)
{
	AcquireParamData data;

	memset(&data, 0, sizeof(AcquireParamData));

	data.spc = spc;
	data.limit_type = limit_type;
	data.limit_val = acq_limit_val;
	data.repeat = repeat;
	data.repeat_time = repeat_time;
	data.should_display = should_display;
	data.display_time = display_time;
	data.accumulate = accumulate;
	data.autosave = autosave;
	data.is_3d = image_3d;

	if(filepath != NULL) {
		data.passed_in_filepath = 1;
		strncpy(data.output_filename, filepath, GCI_MAX_PATHNAME_LEN - 1);
	}

	return spc_start_advanced_params(spc, data);
}

int spc_check_timewindow_valid(Spc *spc)
{
	int max_t = spc_get_max_value_for_adc_res(spc);

	if( spc->_time_window_min > max_t || spc->_time_window_max > max_t ||
		spc->_time_window_max < 0 || spc->_time_window_min < 0) {
		
		GCI_MessagePopup("Error", "Time window is not within the ADC range allowed.");
		return SPC_ERROR;
	}

	return SPC_SUCCESS;
}

int spc_save_prompt_image(Spc *spc)
{
	dictionary *d = NULL;
	int fsize = 0;
	char filepath[GCI_MAX_PATHNAME_LEN] = "";
	char *default_extensions = "*.ics;";
	char directory[GCI_MAX_PATHNAME_LEN] = "";

	//int ret;
	//spc->_acquire_prompt = 1;
	//spc->_acquire_finished = 0;

	//ret = spc_start_scope(spc, 1);

	//while(spc->_acquire_finished == 0) {

	//	printf("Stuck here spc->_acquire_finished %d\n", spc->_acquire_finished);
	//	ProcessSystemEvents();
	//	Delay(0.1);
	//}

	//scanner_stop_scan(spc->_scanner);

	if (LessLameFileSelectPopup (spc->_main_ui_panel, directory, "*.ics",
				default_extensions, "Save Image As", VAL_OK_BUTTON, 0, 0, 1, 1, filepath) <= 0) {
			//	scanner_set_zoom(spc->_scanner, old_zoom);
				spc->_ignore_scanner_signals = 0;
			return SPC_ERROR;
	}

	if(FileExists(filepath, &fsize)) {
		if(GCI_ConfirmPopup("Warning", IDI_WARNING, "File already exists.\nDo you wish to overwrite ?") == 0)
			return SPC_SUCCESS;
	}

	spc_save_3d_image_from_spc_data (spc, filepath, 1);

	return SPC_SUCCESS;
}

int spc_start_advanced_params(Spc* spc, AcquireParamData data)
{
	double start_time = 0.0;
	char path[GCI_MAX_PATHNAME_LEN] = "";
	static int count = 0;

    if(spc->_spc_window == NULL) {
		if( SetupImageWindow(spc) == SPC_ERROR) {
    		send_spc_error_text(spc,"Can not create image window for device %s\n", UIMODULE_GET_DESCRIPTION(spc) );
    		return SPC_ERROR; 
  		}
	}
	
	spc->_acquire_prompt = 0;

	//spc_hide_graph_ui(spc);
	
	logger_log(UIMODULE_LOGGER(spc), LOGGER_INFORMATIONAL, "SPC starting acquisition");  

	if(!data.accumulate) {
		if (bh_clear_memory(spc) != SPC_SUCCESS)
			return SPC_ERROR;
	}

	if(data.passed_in_filepath == 0) {
		if (data.autosave) {

			microscope_get_user_data_directory(spc->_ms, path);
			if(SimpleFilePrefixSaveDialog(UIMODULE_MAIN_PANEL_ID(spc), path, data.output_dir, data.output_filename, data.output_filename_ext) < 0 )
				return SPC_ERROR;  

			if(strncmp(data.output_filename_ext, ".ics", 4) != 0) {
				GCI_MessagePopup("SPC Error", "Saving time resolved images currently only works with ics files");
				return SPC_ERROR;
			}
		}
	}

	spc_dim_controls(spc, 1);		//Dim controls
	SetCtrlAttribute(spc->_graph_ui_panel, SPC_GRAPH_SAVE_PROMT, ATTR_DIMMED, 1);

	// We are in Park so start the scope to acquire a prompt for the set period of time
	if (scanner_get_zoom(spc->_scanner) == 0) { 
		if (GCI_ConfirmPopup("Acquire Prompt", IDI_INFORMATION, "The Zoom is set to Park.\nAcquire Prompt trace now?"))
			return spc_start_scope(spc, 1);
		else {
			spc->_acquire_finished = 1;
			spc_dim_controls(spc, 0);  //Enable all controls
			return SPC_SUCCESS;
		}
	}

	bh_arm(spc);
	
	SetActivePanel(spc->_main_ui_panel); //So that space bar can start/stop the acquisition

	if(spc_check_timewindow_valid(spc) == SPC_ERROR)
		return SPC_ERROR;

	spc->_acquire = 1;
	spc->_acquire_finished = 0;
	data.current_frame = 0;

	// Save Scanner settings for this acquired image
	spc->scanner_settings.displayed_image_scanner_zoom = scanner_get_zoom(spc->_scanner);
	spc->scanner_settings.displayed_image_x_shift = scanner_get_x_shift(spc->_scanner);
	spc->scanner_settings.displayed_image_y_shift = scanner_get_y_shift(spc->_scanner);

	spc_adapt_parameters_for_memory(spc, 0);

	CmtScheduleThreadPoolFunction (gci_thread_pool(), spc_start_measurement_thread, &data, NULL);

	start_time = Timer();

	while(spc->_acquire_finished == 0) {
		//printf("Stuck here spc->_acquire_finished %d %d\n", spc->_acquire_finished, count++);
		ProcessSystemEvents();
		Delay(0.1);
	}

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: End of spc_start_advanced_params\n", time_str);
#endif

	spc->_acquire = 0;
	spc->_acquire_finished = 1;

	return SPC_SUCCESS;
}

int spc_start(Spc* spc)
{
	int acq_limit_type = 0, acq_limit_val = 1, accumulate = 0, autosave = 0, repeat = 0, should_display = 0;
	float repeat_time = 0.0f;
	double display_time = 0.0;

	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, &acq_limit_type);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, &acq_limit_val);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT, &repeat);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT_TIME, &repeat_time);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_AUTOSAVE, &autosave);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACCUMULATE, &accumulate);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY, &should_display); 
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY_TIME, &display_time); 

	spc_set_acq_limit_adv(spc, acq_limit_type, acq_limit_val);

	return spc_start_advanced(spc, acq_limit_type, acq_limit_val, repeat, repeat_time, should_display, display_time,
					   accumulate, autosave, 1, NULL);
}


int spc_display_main_ui (Spc* spc)
{
	if(spc == NULL)
		return SPC_ERROR;

	// microscope_set_illumination_mode(spc->_ms, MICROSCOPE_LASER_SCANNING);
	
	ui_module_display_panel(UIMODULE_CAST(spc), spc->_main_ui_panel);
	
	spc_set_acq_limit(spc);

	bh_display_rates_ui(spc);

  	return SPC_SUCCESS;
}

int spc_hide_ui (Spc* spc)
{
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCOPE, 0);

	ui_module_hide_all_panels(UIMODULE_CAST(spc));

  	return SPC_SUCCESS;
}

int spc_display_pileup_ui (Spc* spc)
{
	if(spc->_pileup_ui_panel > 0)    
  		ui_module_display_panel(UIMODULE_CAST(spc), spc->_pileup_ui_panel);    
	
  	return SPC_SUCCESS;
}

int spc_hide_pileup_ui (Spc* spc)
{
	if(spc->_pileup_ui_panel > 0)
		ui_module_hide_panel(UIMODULE_CAST(spc), spc->_pileup_ui_panel);
	
  	return SPC_SUCCESS;
}

