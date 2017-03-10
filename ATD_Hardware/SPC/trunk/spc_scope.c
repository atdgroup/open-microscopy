#include "spc.h"
#include "spc_ui.h"
#include "scanner.h"

#include "FreeImageAlgorithms_Utilities.h"

#include <analysis.h>

#define roundf(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))

int __cdecl ConvertFromDoubleToUShort (double *data_in, unsigned short *data_out, long np)
{
	long i;
	double *iptr = data_in;
	unsigned short *optr = (unsigned short *) data_out;

	if (np < 1)	   // daft input 
		return -3;
	
	for (i=0; i<np; i++)
		optr[i] = (unsigned short) (MIN(MAX(0, iptr[i]), USHRT_MAX));
		
	return 0;
}

static double round_float(double x, int decimal_points)
{
	int n = (int)pow(10, decimal_points);
	return roundf(n * x) / n;
}

void spc_set_scope_y_axis_scale(Spc* spc, unsigned short* buffer)
{
	unsigned short max = 0;
	int autoscale = 0;
	int scale = 0;
	int nChans = min(8, spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y);
	int number_of_elements = spc->mem_info.block_length * nChans;

	GetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_AUTOSCALE, &autoscale);

	if(autoscale && buffer != NULL) {

		FIA_FindUShortMax(buffer, number_of_elements, &max);

		spc->_max_scope_count = max;

		while (max > 10) {
			max /= 10;
			scale++;
		}

		SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_YLOOSE_FIT_AUTOSCALING_UNIT, scale);
		SetAxisScalingMode(spc->_graph_ui_panel, SPC_GRAPH_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0.0, ((int)max+1)*pow(10,scale));
	}
	else {
		GetCtrlVal(spc->_graph_ui_panel, SPC_GRAPH_MAX_COUNT, &(spc->_max_scope_count));
		SetAxisScalingMode (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0, spc->_max_scope_count);	
	}
}

int	spc_start_scope(Spc* spc, int acquire_prompt)
{
	int frames=0;	  //continuous
	float saved_collect_time;
	dictionary *d;

	if(spc->_acquire == 1)
		return SPC_SUCCESS;

	spc->_acquire_prompt = acquire_prompt;

	//if (spc->_spc_window != NULL) 
	//	GCI_ImagingWindow_Hide(spc->_spc_window);

	saved_collect_time = spc->_oscilloscope_collect_time;

	// if acquiring the prompt, set a new scope collect time, but save the orig val
	if (acquire_prompt) {
		double double_time;
		int int_time;
		int type;
	
		GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, &type);	
		if (type == SPC_ACQ_LIMIT_TYPE_FRAMES){
			GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_TIME, &double_time);	
			spc->_oscilloscope_collect_time = (float)double_time;
		}
		else {
			GetCtrlVal(spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, &int_time);	
			spc->_oscilloscope_collect_time = (float)int_time;
		}		
	}

	d = microscope_get_flim_image_metadata (spc->_ms, spc->_spc_window);     
	GCI_ImagingWindow_SetMetaData(spc->_spc_window, d);

	//Start oscilloscope mode
	bh_set_oscilloscope_mode(spc);

	SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_XNAME, "ns");
	
	spc_display_graph_ui(spc);

	spc_dim_controls(spc, 1);		//Dim controls
	
	bh_arm(spc);
	
	SetActivePanel(spc->_main_ui_panel); // So that space bar can start/stop the acquisition

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "AcqStart", GCI_VOID_POINTER, spc);
	
	scanner_start_scan(spc->_scanner, frames);

	spc_set_scope_y_axis_scale(spc, spc->_buffer);
	
	//tpp = bh_get_tpp(spc);
	//GetCtrlVal(spc->_params_ui_panel, SPC_PARAM_TAC_TIME_PER_CHAN, &tpp);		 //time per channel in ns
	
	spc_set_scope_x_axis_scale(spc);

	spc_start_oscilloscope(spc);

	spc->_oscilloscope_collect_time = saved_collect_time;
	
	return SPC_SUCCESS;
}

int	spc_stop_scope(Spc* spc)
{
	if(spc->_scanner != NULL)
		scanner_stop_scan(spc->_scanner);

	//spc_dim_sys_param_controls(spc, 0);
	spc_dim_controls(spc, 0);
	spc->_acquire = 0;		//stop

	return SPC_SUCCESS;
}

static int bh_read_osc_results(Spc* spc)
{
	int err, nChans, i, n, number_of_elements;
	unsigned short disp_page=1;
	double *testdata, *gaussNoise = NULL;

	//How many channels are enabled?
	nChans = min(8, spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y);
    
	//Read single trace for each channel- consecutive blocks in frame 1 of page 1
	memset (spc->_buffer, 0, nChans*spc->mem_info.block_length*sizeof(short));   //clear buffer
	
	err = SPC_read_data_frame(spc->_active_module, 0, disp_page-1, spc->_buffer);
    
	spc_check_error(spc, err);
    
	if (err)
		return SPC_ERROR;
    
	if (spc->_simulation) {

		number_of_elements = spc->mem_info.block_length*nChans;

		//Fake some data for testing
		testdata = (double *)calloc(number_of_elements, sizeof(double));
		gaussNoise = (double *)calloc(number_of_elements, sizeof(double));
		
		for (n=0; n<nChans; n++) {
			for (i=spc->mem_info.block_length/10; i<spc->mem_info.block_length; i++)
			{
				testdata[i+n*spc->mem_info.block_length] = exp(-i/(spc->mem_info.block_length/(5+n)));
			}
		}

		LinEv1D (testdata, number_of_elements, 2000.0, 0, testdata);	
		GaussNoise (number_of_elements, 5.0, 0, gaussNoise);
		Add1D (testdata, gaussNoise, number_of_elements, testdata);
		
		ConvertFromDoubleToUShort(testdata, spc->_buffer, number_of_elements);

		free(testdata);
		testdata = NULL;
		free(gaussNoise);
		gaussNoise = NULL;
	}

	return SPC_SUCCESS;
}

void spc_set_scope_x_axis_scale(Spc* spc)
{
	int i, scale=0, rounded_max;
	float x_max = spc->_spc_data->tac_range/(float)spc->_spc_data->tac_gain;
	int n = (int)pow(2.0, spc->_spc_data->adc_resolution);			 //number of points per channel
	float x_temp = x_max;
	
	if (spc->oscilloscope_settings._x_data != NULL) {
		free (spc->oscilloscope_settings._x_data);
		spc->oscilloscope_settings._x_data = NULL;
	}

	spc->oscilloscope_settings._x_data = (double *)calloc(n, sizeof(double));

	// do "nice" scale rounding, this was some rounding by Glenn, but we want to display the timebase you are capturing, surely!
/*	
	while (x_max > 10) {
		x_max /= 10;
		scale ++;
	}

	rounded_max = (int)round_float(x_max, 1)*(int)pow(10.0,scale);

	SetAxisScalingMode (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, VAL_XAXIS, VAL_MANUAL, 0, rounded_max);
	SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING_UNIT, scale);
	SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING, 1);

	for (i=0; i<n; i++)
		spc->oscilloscope_settings._x_data[i] = (double)i * rounded_max/n;   // THIS IS A BUG, SHOULD NOT USE rounded_max HERE!
*/
	// do not round
	while (x_temp > 10) {
		x_temp /= 10;
		scale ++;
	}

	SetAxisScalingMode (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, VAL_XAXIS, VAL_MANUAL, 0, x_max);
	SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING_UNIT, scale);
	SetCtrlAttribute (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, ATTR_XLOOSE_FIT_AUTOSCALING, 1);

	for (i=0; i<n; i++)
		spc->oscilloscope_settings._x_data[i] = (double)i * x_max/n;

}


static int spc_display_osc_results(Spc* spc)
{
	int i=0, j, nChans, chansEnabled[4];
	unsigned short disp_page=1;
	int plotColour[8] = {VAL_RED, VAL_DK_GREEN, VAL_BLUE, VAL_YELLOW, VAL_DK_BLUE, VAL_DK_RED, VAL_GREEN, VAL_DK_YELLOW};

	// How many channels are enabled?
	nChans = min(8, spc->_spc_data->scan_rout_x * spc->_spc_data->scan_rout_y);

	if (bh_read_osc_results(spc) == SPC_ERROR)
		return SPC_ERROR;
	
	//We plot chans 1,2,3 in red, green, blue always
	for (i=0; i<4; i++) 
		chansEnabled[i] = spc->_chans_enabled[i];
			
	DeleteGraphPlot (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, -1, VAL_DELAYED_DRAW);

	if (nChans > 1) {

		for (j=0; j<4; j++) {
			if (chansEnabled[j])
				PlotXY (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, spc->oscilloscope_settings._x_data, &spc->_buffer[j*spc->mem_info.block_length],
					spc->mem_info.block_length, VAL_DOUBLE, VAL_UNSIGNED_SHORT_INTEGER, VAL_FAT_LINE,
					VAL_NO_POINT, VAL_SOLID, 1, plotColour[j]);
		}
	}
	else  {
		for (j=0; j<4; j++) {
			if (chansEnabled[j])
				break;
		}

		PlotXY (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, spc->oscilloscope_settings._x_data, spc->_buffer,
				spc->mem_info.block_length, VAL_DOUBLE, VAL_UNSIGNED_SHORT_INTEGER, VAL_FAT_LINE,
				VAL_NO_POINT, VAL_SOLID, 1, plotColour[j]);
	}

	ProcessDrawEvents();

	return SPC_SUCCESS;
}

int spc_display_graph_ui (Spc* spc)
{
	if(spc->_graph_ui_panel > 0)        
		ui_module_display_panel(UIMODULE_CAST(spc), spc->_graph_ui_panel);          
	
  	return SPC_SUCCESS;
}

int spc_hide_graph_ui (Spc* spc)
{
	if(spc->_graph_ui_panel > 0)
		ui_module_hide_panel(UIMODULE_CAST(spc), spc->_graph_ui_panel);         
	
  	return SPC_SUCCESS;
}

static int bh_single_osc_acquisition(Spc* spc)
{
	int ret, armed = 1;
	unsigned short bh_state;
	
    ret = SPC_start_measurement(spc->_active_module);
	spc_check_error(spc, ret);

	//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(spc), "AcqStart", GCI_VOID_POINTER, spc);  

	while(armed) {

		ret = SPC_test_state(spc->_active_module, &bh_state);
		spc_check_error(spc, ret);

		if(spc->_simulation) {
		//simulate collection complete
			bh_state=0;	
		}

		if (bh_state & SPC_ARMED) {  // 1 - still armed
		
        	if ( (bh_state & SPC_MEASURE) == 0){
          		// system armed but collection not started because
          		// it is still waiting for Sync signals
          		continue;
          	}
        	
        	ProcessSystemEvents();
      	}
      	else {  //Not armed, collection complete
   			bh_read_osc_results(spc);
			ret = SPC_stop_measurement(spc->_active_module);
        	armed = 0;
      	}
	}
	return ret;
}


int CVICALLBACK thread_scope_update(void *callback)
{
	Spc* spc = (Spc *) callback;
  
	while (spc->_acquire) {

		// Clear memory unless we want to accumulate. Never accumulate in oscilloscope mode

		
		if ((bh_clear_memory(spc)) != SPC_SUCCESS) {

			// Stop Oscilloscope mode
			spc_stop(spc);
		
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_SCOPE, 0);
			SetCtrlVal(spc->_main_ui_panel, SPC_MAIN_MEAS_LED, 0);

			spc->_acquire_finished = 1;

			return MICROSCOPE_ERROR;
		}

		bh_single_osc_acquisition(spc);
		spc_set_scope_y_axis_scale(spc, spc->_buffer);
		spc_display_osc_results(spc);

		Delay(0.1);

		if (spc->_acquire_prompt){  // if acquiring prompt just do once 
			spc->_acquire = 0;
			spc_dim_controls(spc, 0);
			scanner_stop_scan(spc->_scanner);	
		}
   	}

	//DeleteGraphPlot (spc->_graph_ui_panel, SPC_GRAPH_GRAPH, -1, VAL_IMMEDIATE_DRAW);
	ProcessDrawEvents();

	spc->_acquire_finished = 1;

	// If we were in Park then on stop un grey the save prompt button
	if(scanner_get_zoom(spc->_scanner) == 0) {
		SetCtrlAttribute(spc->_graph_ui_panel, SPC_GRAPH_SAVE_PROMT, ATTR_DIMMED, 0);
	}

	return MICROSCOPE_SUCCESS;
}


int spc_start_oscilloscope(Spc* spc)
{
	int ret = SPC_SUCCESS, thread_id = 0;
	
	spc->_acquire = 1;

	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, thread_scope_update, spc, &thread_id);  
		
	return ret;
}