#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "BeamScanner.h"

atd_beam_scanner_set_mode(BeamScanner *bs, BEAM_SCANNER_MODE mode)
atd_beam_scanner_set_start_mode(BeamScanner *bs)
static int atd_beam_scanner_set_stop_mode(BeamScanner *bs)
static int atd_beam_scanner_set_pattern_mode(BeamScanner *bs, BEAM_SCANNER_PATTERN pattern,
											 BEAM_SCANNER_PATTERN_SCALE_FACTOR scale)
static int atd_beam_scanner_set_batch_count_mode(BeamScanner *bs)
static int atd_beam_scanner_read_status(BeamScanner *bs, BYTE *status_bytes)
atd_beam_scanner_get_number_of_loaded_points(BeamScanner *bs)
atd_beam_scanner_wait_for_block_completion(BeamScanner *bs)
atd_beam_scanner_get_counts(BeamScanner *bs)
atd_beam_scanner_load_data_from_bitmap(BeamScanner *bs, FIBITMAP *dib)

PyObject* microscope_get_cubes(PyObject *self, PyObject *args)
{
	int i, num;

  	PyObject *dict = NULL;
  	PyObject *list = NULL;

  	Microscope *ms = microscope_get_microscope();
    FluoCubeManager* cm = microscope_get_cube_manager(ms);
	FluoCube* cubes = NULL;
	
	cube_manager_get_number_of_cubes(cm, &num);  
	
  	list = PyList_New(num);

  	if (list == NULL) {
    	PyErr_NoMemory();
    	goto err_handler;
  	}
	
	cubes = cube_manager_get_active_cubes(cm);
	
	for (i = 0; i < num; i++) {
	  					  
		dict = Py_BuildValue("{s:s,s:i,s:i,s:i,s:i,s:i}",
                 "Name", cubes[i].name,
				 "Position", i + 1,   
				 "Exc-Min-NM", cubes[i].exc_min_nm,
				 "Exc-Max-NM", cubes[i].exc_max_nm,
				 "Dichroic-NM", cubes[i].dichroic_nm,
				 "Emm-Min-NM", cubes[i].emm_min_nm,
				 "Emm-Max-NM", cubes[i].emm_max_nm
				 );
	 
    	if (dict == NULL) {
      		PyErr_NoMemory();
      		goto err_handler;
    	}
	
    	if (PyList_SetItem(list, i, dict) == -1) {
      		goto err_handler;
    	}
	}
  
	free(cubes);
  
	return list;

	err_handler:
  
		free(cubes);  
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (dict != NULL && !PySequence_Contains(list, dict)) {
    		Py_DECREF(dict);
  		}
  
  		Py_XDECREF(list);

  	return NULL;
}



PyObject* microscope_move_cube_to_position(PyObject *self, PyObject *args)
{
	int pos;
	
  	Microscope *ms = microscope_get_microscope();
    FluoCubeManager* cm = microscope_get_cube_manager(ms);

	// Takes 1 integers
    if(!PyArg_ParseTuple(args, "i", &pos))
        return NULL;
	
    if(cube_manager_move_to_position(cm, pos) == CUBE_MANAGER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "cube_manager_move_to_position returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_wait_for_cube(PyObject *self, PyObject *args)
{
  	double delay, timeout;
	
	Microscope *ms = microscope_get_microscope();
    FluoCubeManager* cm = microscope_get_cube_manager(ms);

	// Takes 2 doubles
    if(!PyArg_ParseTuple(args, "dd", &delay, &timeout))
        return NULL;

	if(cube_manager_wait_for_stop_moving (cm, timeout) == CUBE_MANAGER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "cube_manager_wait_for_stop_moving returned error.");
		goto err_handler;	
	}
	
	Delay(delay);
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}
