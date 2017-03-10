#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "OpticalPath.h"

PyObject* microscope_get_opticalpaths(PyObject *self, PyObject *args)
{
	int i, num;

  	PyObject *dict = NULL;
  	PyObject *list = NULL;

  	Microscope *ms = microscope_get_microscope();
	CMDeviceNode *node = NULL;
	
	OpticalPathManager* opm = microscope_get_optical_path_manager(ms);

	num = device_conf_get_num_active_devices(opm->dc);
	
	if(num < 1)
		goto err_handler;   

  	list = PyList_New(num);

  	if (list == NULL) {
    	PyErr_NoMemory();
    	goto err_handler;
  	}

	for(i=0; i < num; i++) {
	
		node = ListGetPtrToItem(opm->dc->in_use_list, i+1);    
	  					  
		dict = Py_BuildValue("{s:s,s:i}",
                 "Name", node->name,
				 "Position", node->position   
				 );
	 
    	if (dict == NULL) {
      		PyErr_NoMemory();
      		goto err_handler;
    	}
	
    	if (PyList_SetItem(list, i, dict) == -1) {
      		goto err_handler;
    	}
	}
 
	return list;

	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (dict != NULL && !PySequence_Contains(list, dict)) {
    		Py_DECREF(dict);
  		}
  
  		Py_XDECREF(list);

  	return NULL;
}

PyObject* microscope_move_opticalpath_to_position(PyObject *self, PyObject *args)
{
	int pos;
	
  	Microscope *ms = microscope_get_microscope();
    OpticalPathManager* opm = microscope_get_optical_path_manager(ms); 

	// Takes 1 integers
    if(!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
	
	
    if(optical_path_move_to_position(opm, pos) == OPTICAL_PATH_MANAGER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "optical_path_move_to_position returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}
