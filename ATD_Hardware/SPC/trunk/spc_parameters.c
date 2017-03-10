#include "spc.h"
#include "spc_ui.h"
#include "string_utils.h"

// This only saves the settings for the hardware
// using b&h api - No extra GCI defined settings
static int bh_save_params(Spc* spc, const char* filepath)
{
	int err;
	
	if(spc->_spc_data == NULL)
		return -1;

	err = SPC_save_parameters_to_inifile(spc->_spc_data, (char*) filepath, NULL, 1);

	spc_check_error(spc, err);

	return err;
}


static int save_gci_spc_settings_to_file(Spc* spc, const char* filepath, const char *flags)
{
	FILE *fd;
	dictionary *d = dictionary_new(20);
	int accumulate = 0, autosave = 0, repeat = 0, display = 0;
	float repeat_time = 0.0f;
	double display_time = 0.0;

	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT, &repeat);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT_TIME, &repeat_time);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACCUMULATE, &accumulate);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_AUTOSAVE, &autosave);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY, &display);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY_TIME, &display_time);

	fd = fopen(filepath, flags);
	
	if (fd==NULL)
		return SPC_ERROR; 
	
	fprintf(fd, "\n");

	dictionary_set(d, "gci_spc", NULL);

	dictionary_setint    (d, "AcqLimitType", spc->_acq_limit_type);
	dictionary_setint    (d, "AcqLimitVal", spc->_acq_limit_val);
	dictionary_setint    (d, "Display", display);
	dictionary_setdouble (d, "DisplayTime", display_time);
	dictionary_setint    (d, "TACselect", spc->_tac_select);
	dictionary_setdouble (d, "TACval", spc->_tac_val);
	
	if (spc->_spc_data != NULL) {
		dictionary_setint(d, "ADCres", spc->_spc_data->adc_resolution);
	}

	dictionary_setdouble (d, "MultiChannelTACoffset", spc->_mc_tac_offset);
	dictionary_setint    (d, "Repeat", repeat);
	dictionary_setdouble (d, "RepeatTime", repeat_time);
	dictionary_setint    (d, "Autosave", autosave);
	dictionary_setint    (d, "Accumulate", accumulate);
	dictionary_setint    (d, "Autorange", spc->_autorange);
	dictionary_setint    (d, "TimeWindowMin", spc->_time_window_min);
	dictionary_setint    (d, "TimeWindowMax", spc->_time_window_max);
	dictionary_setint    (d, "MaxCount", spc->_max_scope_count);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
    return SPC_SUCCESS;
}


int spc_save_settings(Spc* spc, const char *filepath)
{
	// Send the settings and update struct variables befor saving
	spc_send_sys_params(spc);

	bh_save_params(spc, filepath);

	return save_gci_spc_settings_to_file(spc, filepath, "a+");
}

int spc_save_settings_dialog(Spc* spc)
{
	char filepath[GCI_MAX_PATHNAME_LEN] = "";

	if (FileSelectPopup (UIMODULE_GET_DATA_DIR(spc), "spcm.ini", "*.ini", "Save Settings", VAL_SAVE_BUTTON, 0, 1, 1, 1, filepath) <= 0)
		return SPC_SUCCESS;
	
	return spc_save_settings(spc, filepath);
}

int spc_save_default_params(Spc* spc)
{
	char filepath[GCI_MAX_PATHNAME_LEN] = "";

	sprintf(filepath, "%s\\%s", UIMODULE_GET_DATA_DIR(spc), DEFAULT_SPC_FILENAME);

	return spc_save_settings(spc, filepath);
}

static int bh_load_params(Spc* spc, const char* filepath)
{
	int err;
	
	err = SPC_read_parameters_from_inifile(spc->_spc_data, (char *) filepath);
	spc_check_error(spc, err);
	
	if (err)
		return err;


	err = SPC_set_parameters(spc->_active_module, spc->_spc_data);
	spc_check_error(spc, err);
	
	if (err)
		return err;

	//Always read them back again because of interdependancies
	err = SPC_get_parameters(spc->_active_module, spc->_spc_data);
	spc_check_error(spc, err);
	
	if (err < 0)
		return err;

	spc_update_params_panel(spc);
	
	spc_update_main_panel_parameters(spc);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "ParamChanged", GCI_VOID_POINTER, spc);  

	//bh_on_change(spc); //Inform other modules
	
	return SPC_SUCCESS;
}

static int load_gci_spc_ini_file_settings(Spc* spc, const char *filepath)
{
	dictionary* d = NULL;  
	int tmp, file_size = 0;
	double tempf;
	
	if(!FileExists(filepath, &file_size))
		return SPC_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
    	tmp = dictionary_getint(d, "gci_spc:AcqLimitType", -1);
		if(tmp >= 0) {
			spc->_acq_limit_type = tmp;
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, spc->_acq_limit_type);
		}
	
    	tmp = dictionary_getint(d, "gci_spc:AcqLimitVal", -1);
		if(tmp >= 0) {
			spc->_acq_limit_val = tmp;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, spc->_acq_limit_val);
		}
	
    	tmp = dictionary_getint(d, "gci_spc:Display", -1);
		if(tmp >= 0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_DISPLAY, tmp);
 		}
	
	   	tempf = dictionary_getdouble(d, "gci_spc:DisplayTime", -1.0);
		if(tempf >= 0.0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_DISPLAY_TIME, tempf);
		}
	
    	tmp = dictionary_getint(d, "gci_spc:TACselect", -1);
		if(tmp >= 0) {
			spc->_tac_select = tmp;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_SEL, spc->_tac_select);
		}
	
    	tempf = dictionary_getdouble(d, "gci_spc:TACval", -1);
		if(tempf > 0) {
			spc->_tac_val = (float)tempf;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TAC_VAL, spc->_tac_val);
		}
	
    	tmp = dictionary_getint(d, "gci_spc:ADCres", -1);
		if(tmp >= 0) {
			spc->_spc_data->adc_resolution = tmp;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, spc->_spc_data->adc_resolution);
			bh_set_adc_res(spc, tmp) ;   
		}
	
    	tempf = dictionary_getdouble(d, "gci_spc:MultiChannelTACoffset", -1);
		if(tempf > 0) {
			spc->_mc_tac_offset = tempf;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MC_TAC_OFFSET, spc->_mc_tac_offset);
		}

    	tmp = dictionary_getint(d, "gci_spc:Repeat", -1);
		if(tmp >= 0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_REPEAT, tmp);
		}
	
    	tempf = dictionary_getdouble(d, "gci_spcRepeatTime", -1.0);
		if(tempf > 0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_REPEAT_TIME, tempf);
		}

    	tmp = dictionary_getint(d, "gci_spc:Autosave", -1);
		if(tmp >= 0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_AUTOSAVE, tmp);
    	}
		
		tmp = dictionary_getint(d, "gci_spc:Accumulate", -1);
		if(tmp >= 0) {
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACCUMULATE, tmp);
    	}
		
		tmp = dictionary_getint(d, "gci_spc:TimeWindowMin", -1);
		if(tmp >= 0) {
			spc->_time_window_min = tmp;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_FROM_TW, spc->_time_window_min);
    	}
		
		tmp = dictionary_getint(d, "gci_spc:TimeWindowMax", -1);
		if(tmp >= 0) {
			spc->_time_window_max = tmp;	
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TO_TW, spc->_time_window_max);
    	}

		tmp = dictionary_getint(d, "gci_spc:MaxCount", -1);
		if(tmp >= 0) {
			spc->_max_scope_count = tmp;	
			SetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, spc->_max_scope_count);
		}
		
		dictionary_del(d);   
	}
	else
		return SPC_ERROR;
		
    return SPC_SUCCESS;
}

// This function is the only public load api method 
// The user specifies the filepath but two files are loaded.
// One with the b&h settings
// and onew with extra gci settings ie filename_gci_extra.ini
int spc_load_settings(Spc* spc, const char *filepath)
{
	int err = 0;

	err = load_gci_spc_ini_file_settings(spc, filepath);

	bh_load_params(spc, filepath);

	// Send the settings and update struct variables befor saving
	spc_send_sys_params(spc);

	return err;
}


int spc_load_settings_dialog(Spc* spc)
{
	char filepath[GCI_MAX_PATHNAME_LEN] = "";

	if (FileSelectPopup (UIMODULE_GET_DATA_DIR(spc), "spcm.ini", "*.ini", "Load Settings", VAL_LOAD_BUTTON, 0, 1, 1, 0, filepath) != 1)
		return SPC_SUCCESS;
	
	return spc_load_settings(spc, filepath);
}

int spc_load_default_params(Spc* spc)
{
	char filepath[GCI_MAX_PATHNAME_LEN] = "";

	sprintf(filepath, "%s\\%s", UIMODULE_GET_DATA_DIR(spc), DEFAULT_SPC_FILENAME);

	return spc_load_settings(spc, filepath);
}

int spc_get_default_bh_ini_file_path(Spc* spc, char *filepath)
{
	int file_size = 0;
	char filename[GCI_MAX_PATHNAME_LEN] = "";

	sprintf(filepath, "%s\\%s", UIMODULE_GET_DATA_DIR(spc), DEFAULT_SPC_FILENAME);

	return FileExists(filepath, &file_size);
}

void spc_update_params_panel(Spc* spc)
{
	unsigned short X_pol, Y_pol, pix_clock_pol, topBorder, leftBorder;

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, spc->_spc_data->mode);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OFLO_CTRL, spc->_spc_data->stop_on_ovfl);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TRIGGER, spc->_spc_data->trigger);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, spc->_spc_data->collect_time);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DEAD_TIME, spc->_spc_data->dead_time_comp);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_LIM_LOW, spc->_spc_data->cfd_limit_low);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_LIM_HIGH, spc->_spc_data->cfd_limit_high);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_ZC, spc->_spc_data->cfd_zc_level);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_HOLDOFF, spc->_spc_data->cfd_holdoff);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ADC_RES, spc->_spc_data->adc_resolution);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DTHER, spc->_spc_data->dither_range);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COUNT_INC, spc->_spc_data->count_incr);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DELAY, spc->_spc_data->ext_latch_delay);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_X, spc->_spc_data->scan_rout_x);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_Y, spc->_spc_data->scan_rout_y);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_X, spc->_spc_data->scan_size_x);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_Y, spc->_spc_data->scan_size_y);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_RANGE, spc->_spc_data->tac_range);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_GAIN, spc->_spc_data->tac_gain);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_OFFSET, spc->_spc_data->tac_offset);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_LIM_LOW, spc->_spc_data->tac_limit_low);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_LIM_HIGH, spc->_spc_data->tac_limit_high);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_TIME_PER_CHAN, spc->_spc_data->tac_range/(float)spc->_spc_data->tac_gain);

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_ZC_LEVEL, spc->_spc_data->sync_zc_level);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_HOLDOFF, spc->_spc_data->sync_holdoff);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_THRESH, spc->_spc_data->sync_threshold);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_FREQ_DIV, spc->_spc_data->sync_freq_div);
	
	X_pol = spc->_spc_data->scan_polarity & 1;	// bit 0	
	Y_pol = (spc->_spc_data->scan_polarity & 2) >> 1;	// bit 1	
	pix_clock_pol = (spc->_spc_data->scan_polarity & 4) >> 2;	// bit 2	

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_X_POLARITY, X_pol);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_Y_POLARITY, Y_pol);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_POLARITY, pix_clock_pol);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LINE_COMPRESSION, spc->_spc_data->line_compression);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_DIVIDER, spc->_spc_data->ext_pixclk_div);

	leftBorder = spc->_spc_data->scan_borders & 0x0000ffff;
	topBorder = (spc->_spc_data->scan_borders & 0xffff0000) >> 16;

	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LEFT_BORDER, leftBorder);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TOP_BORDER, topBorder);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK, spc->_spc_data->pixel_clock);

	spc_set_scope_x_axis_scale(spc);
	
	ProcessDrawEvents();
}

void bh_set_adc_res(Spc* spc, int adc_resolution)
{
	int new_adc_range = (int) Round(pow(2.0, adc_resolution));

	// Set range of time window spin controls
	SetCtrlAttribute(spc->_main_ui_panel, SPC_MAIN_FROM_TW, ATTR_MAX_VALUE, MAX(new_adc_range - 2, 0));
	SetCtrlAttribute(spc->_main_ui_panel, SPC_MAIN_TO_TW, ATTR_MAX_VALUE, MAX(new_adc_range - 1, 0));

	// If the adc has been changed to a larger value
	// We scale up the time window the same amount.
	if(adc_resolution > spc->_spc_data->adc_resolution) {

		int tw_min, tw_max;
		int old_adc_range = spc_get_max_value_for_adc_res(spc);

		double low_factor = (double) spc->_time_window_min / old_adc_range;
		double high_factor = (double) spc->_time_window_max / old_adc_range;

		tw_min = (int) (low_factor * new_adc_range);
		tw_max = (int) (high_factor * new_adc_range);

		if(tw_min > tw_max) {
			GCI_MessagePopup("Spc Error", "Time window minimum value must not be greater than the maximum");
			return;
		}

		spc->_time_window_min = tw_min;
		spc->_time_window_max = tw_max;
	}
	else {

		spc->_time_window_min = 0;
		spc->_time_window_max = new_adc_range - 1;		
	}
				
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_FROM_TW, spc->_time_window_min);
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_TO_TW, spc->_time_window_max);

	spc->_spc_data->adc_resolution = adc_resolution;
	SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ADC_RES, adc_resolution);
	SetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ADC_RES, adc_resolution);
}

int bh_enable_sequencer(Spc* spc, int enable)
{
	return SPC_enable_sequencer (spc->_active_module, enable);    
}

int spc_send_sys_params(Spc* spc)
{
	int err, temp;
	unsigned short X_pol, Y_pol, pix_clock_pol, topBorder, leftBorder;
	
	if(spc->_spc_data == NULL)
		return SPC_ERROR;

	// Send all params from System Params panel to board
	// Get all the system parameters from the panels
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OP_MODE, &spc->_spc_data->mode);
	spc->_spc_data->stop_on_time = 1;
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_OFLO_CTRL, &spc->_spc_data->stop_on_ovfl);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TRIGGER, &spc->_spc_data->trigger);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, &spc->_spc_data->collect_time);
	spc->_spc_data->display_time=1.0;
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DEAD_TIME, &temp);
	spc->_spc_data->dead_time_comp = (unsigned short)temp;
	
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_LIM_LOW, &spc->_spc_data->cfd_limit_low);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_LIM_HIGH, &spc->_spc_data->cfd_limit_high);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_ZC, &spc->_spc_data->cfd_zc_level);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_CFD_HOLDOFF, &spc->_spc_data->cfd_holdoff);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ADC_RES, &spc->_spc_data->adc_resolution);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DTHER, &spc->_spc_data->dither_range);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_COUNT_INC, &spc->_spc_data->count_incr);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_DELAY, &spc->_spc_data->ext_latch_delay);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_X, &spc->_spc_data->scan_rout_x);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_ROUTING_Y, &spc->_spc_data->scan_rout_y);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_X, &spc->_spc_data->scan_size_x);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SCAN_Y, &spc->_spc_data->scan_size_y);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_RANGE, &spc->_spc_data->tac_range);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_GAIN, &spc->_spc_data->tac_gain);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_OFFSET, &spc->_spc_data->tac_offset);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_LIM_LOW, &spc->_spc_data->tac_limit_low);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_LIM_HIGH, &spc->_spc_data->tac_limit_high);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_ZC_LEVEL, &spc->_spc_data->sync_zc_level);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_HOLDOFF, &spc->_spc_data->sync_holdoff);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_THRESH, &spc->_spc_data->sync_threshold);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_SYNC_FREQ_DIV, &spc->_spc_data->sync_freq_div);
	
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_X_POLARITY, &X_pol);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_Y_POLARITY, &Y_pol);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_POLARITY, &pix_clock_pol);
	spc->_spc_data->scan_polarity = pix_clock_pol<<2 | Y_pol<<1 | X_pol; 
	
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LINE_COMPRESSION, &spc->_spc_data->line_compression);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_DIVIDER, &spc->_spc_data->ext_pixclk_div);
	
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_LEFT_BORDER, &leftBorder);
	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TOP_BORDER, &topBorder);

	spc_set_borders(spc, leftBorder, topBorder);

	GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK, &spc->_spc_data->pixel_clock);
	
	err = SPC_set_parameters(spc->_active_module, spc->_spc_data);
	
	spc_check_error(spc, err);
	
	if (err)
		return err;
	
	//Always read them back again because of interdependancies
	err = SPC_get_parameters(spc->_active_module, spc->_spc_data);
	spc_check_error(spc, err);
	
	if (err < 0)
		return err;

	spc_update_params_panel(spc);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "ParamChanged", GCI_VOID_POINTER, spc);  

	return SPC_SUCCESS;
}

void spc_dim_sys_param_controls(Spc* spc, int dimmed)
{
	// The SPC software seems to disable the whole params panel when 
	// appropriate. We also have xy scan parameters on the params panel,
	// but I think its best to play safe and dimmed all controls when scanning
	if(dimmed)
		ui_module_disable_panel(spc->_params_ui_panel, 3, SPC_PARAM_QUIT, SPC_PARAM_SAVE, SPC_PARAM_LOAD); 
	else
		ui_module_enable_panel(spc->_params_ui_panel, 0); 
}

void bh_set_sys_param_controls_mode(Spc* spc, int mode)
{
//	SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_OP_MODE, ATTR_CTRL_MODE, mode); //0=indicator, 1=hot
	//SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_ADC_RES, ATTR_CTRL_MODE, mode);
//	SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_ROUTING_X, ATTR_CTRL_MODE, mode);
	//SetCtrlAttribute (spc->_params_ui_panel, SPC_PARAM_ROUTING_Y, ATTR_CTRL_MODE, mode);
}

///////////////////////////////////////////////////////////////////////////////
int bh_init_params_panel(Spc* spc)
{
	if ((spc->_params_ui_panel = ui_module_add_panel(UIMODULE_CAST(spc), "spc_ui.uir", SPC_PARAM, 0)) < 0)
		return SPC_ERROR;   

    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_LINE_COMPRESSION, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_POLARITY, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_Y_POLARITY, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_X_POLARITY, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TOP_BORDER, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_LEFT_BORDER, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_PIX_CLOCK_DIVIDER, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_QUIT, cbQuitSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SAVE, cbSaveSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_LOAD, cbLoadSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_OP_MODE, cbSysParamOpMode, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_DEAD_TIME, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_COUNT_INC, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SCAN_Y, cbCheckScanSize, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SCAN_X, cbCheckScanSize, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_ROUTING_Y, cbCheckScanSize, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_ROUTING_X, cbCheckScanSize, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_DELAY, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_OFLO_CTRL, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_CFD_HOLDOFF, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_CFD_ZC, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_CFD_LIM_HIGH, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_CFD_LIM_LOW, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TAC_LIM_HIGH, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SYNC_THRESH, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SYNC_HOLDOFF, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SYNC_ZC_LEVEL, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TAC_LIM_LOW, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TAC_GAIN, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TAC_OFFSET, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TAC_RANGE, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_COLLECT_TIME, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_SYNC_FREQ_DIV, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_DTHER, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_MEM_OFFSET, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_ADC_RES, cbCheckScanSize, spc) < 0)
		return SPC_ERROR;
  	
    if ( InstallCtrlCallback (spc->_params_ui_panel, SPC_PARAM_TRIGGER, cbSendSysParams, spc) < 0)
		return SPC_ERROR;
  	
	return SPC_SUCCESS;
}

void load_settings_from_main_panel(Spc* spc)
{
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, &spc->_acq_limit_type);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, &spc->_acq_limit_val);
	//GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_TAC_SEL, &spc->_tac_select);
	//GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_TAC_VAL, &spc->_tac_val);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ADC_RES, &spc->_spc_data->adc_resolution);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_MC_TAC_OFFSET, &spc->_mc_tac_offset);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_FROM_TW, &spc->_time_window_min);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_TO_TW, &spc->_time_window_max);
	GetCtrlVal (spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &spc->_max_scope_count);
}
