#ifdef BUILD_MODULE_SPC

#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "spc.h"
#include "spc_ui.h"

PyObject* spc_wrap_clear_board_memory(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);

	bh_clear_memory(spc);

	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_spc_start(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);
	char *filepath = NULL;
	int limit_type, limit_val, repeat, should_display, accumulate, autosave, is_3d;
	double repeat_time, display_time;

	// Takes 1 string
	// Params
	// SPC_ACQUISITION_LIMIT_TYPE limit_type
	// int limit_val
	// int repeat
	// float repeat_time
	// int should_display
	// double display_time
	// int accumulate
	// int autosave
	// int 3d_data save or 2d
	// char* filepath
    if(!PyArg_ParseTuple(args, "iiididiis", &limit_type, &limit_val, &repeat, &repeat_time,
		&should_display, &display_time, &accumulate, &autosave, &is_3d, &filepath))
        goto err_handler;   	  
	
	spc_start_advanced(spc, limit_type, limit_val, repeat, (float) repeat_time, should_display, display_time,
					   accumulate, autosave, is_3d, filepath);

	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
	
		assert(PyErr_Occurred()); // An exception must already be set. 

		return NULL;  	
}

PyObject* microscope_spc_stop(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);
	
	spc_stop(spc);

	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* spc_wrap_acquire_and_save_to_filename_using_ui_values(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);
	char *filepath = NULL;
	int limit_type, limit_val, repeat, should_display, accumulate, autosave = 0, is_3d;
	float repeat_time;
	double display_time;

	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_TYPE, &limit_type);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACQ_LIMIT_VAL, &limit_val);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT, &repeat);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_REPEAT_TIME, &repeat_time);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_ACCUMULATE, &accumulate);
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY, &should_display); 
	GetCtrlVal (spc->_main_ui_panel, SPC_MAIN_DISPLAY_TIME, &display_time); 

    if(!PyArg_ParseTuple(args, "iz", &is_3d, &filepath))
        goto err_handler;   	  
	
	spc_start_advanced(spc, limit_type, limit_val, repeat, repeat_time, should_display, display_time,
					   accumulate, autosave, is_3d, filepath);

	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
	
		assert(PyErr_Occurred()); // An exception must already be set. 

		return NULL;  	
}

PyObject* microscope_spc_set_adc_res(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);
	
	int res, index;
	
	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "i", &res))
        return NULL;
	
	
	switch (res)
	{
		case 4096:
		{
			index = 12;	
			break;   
		}
	
		case 1024:
		{
			index = 10;	     
			break;   
		}
		
		case 256:
		{
			index = 8;	     	
			break;   
		}
		
		case 64:
		{
			index = 6;	     	
			break;   
		}
		
		case 16:
		{
			index = 4;	     	
			break;   
		}
		
		case 4:
		{
			index = 2;	     
			break;   
		}
		
		case 1:
		{
			index = 0;	     
			break;
		}
	}
	
	//bh_set_adc_res(spc, index);
	//bh_check_scan_size(spc, ADC_RES);

	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_spc_set_acq_limit(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Spc* spc = microscope_get_spc(ms);
	int acq_limit_type = 0, acq_limit_val = 0;
	
	// Takes 2 ints
    if(!PyArg_ParseTuple(args, "ii", &acq_limit_type, &acq_limit_val))
        return NULL;
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}

#endif // BUILD_MODULE_SPC