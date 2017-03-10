#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "timelapse.h"

#ifdef MICROSCOPE_PYTHON_AUTOMATION

PyObject* microscope_visit_timelapse_points(PyObject *self, PyObject *args)
{
	int err;
	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);
    
	// Takes no arguments	
    err = timelapse_perform_points(tl);
    
	if(err < 0) {
		PyErr_SetString(PyExc_ValueError, "timelapse_perform_points returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
 	return Py_None;
}


PyObject* microscope_abort_timelapse_cycle(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);
 
	tl->active = 0;
	
	Py_INCREF(Py_None);
 	return Py_None;
}


PyObject* microscope_timelapse_has_been_aborted(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);
 
	return Py_BuildValue("i", tl->active);
}

PyObject* microscope_get_all_timelaspe_points(PyObject *self, PyObject *args)
{
	int i, count = 0;
	ListType points;
	TimelapseTableEntry pt; 
	PyObject *all_point_tuple = NULL; 
	PyObject *point_tuple = NULL; 
	
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);

	points = timelapse_get_point_list(tl); 
	
	count = ListNumItems(points);
	
	// No points defined. Return None
	if(count <= 0) {
		
		Py_INCREF(Py_None);
 		return Py_None;	
	}
		
	all_point_tuple = PyTuple_New(count);   	
		
	if (all_point_tuple == NULL) {
    	PyErr_NoMemory();
    	goto err_handler;
  	}
	
	for(i=1; i <= count; i++) {
	
		ListGetItem (points, &pt, i);	
		
		point_tuple = Py_BuildValue("(d,d,d)", pt.centre.x, pt.centre.y, pt.centre.z);
	 
    	if (point_tuple == NULL) {
      		PyErr_NoMemory();
      		goto err_handler;
    	}
	
    	if (PyTuple_SetItem(all_point_tuple, i-1, point_tuple) == -1) {
      		goto err_handler;
    	}
	}
	
	return all_point_tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (point_tuple != NULL && !PySequence_Contains(all_point_tuple, point_tuple)) {
    		Py_DECREF(point_tuple);
  		}
  
  		Py_XDECREF(all_point_tuple);

  	return NULL;
}

PyObject* microscope_get_timelapse_region(PyObject *self, PyObject *args)
{
	PyObject *point_tuple = NULL; 
	TimelapseTableEntry pt; 
	
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);

	timelapse_get_point(tl, tl->current_point, &pt);

	point_tuple = Py_BuildValue("(d,d)", pt.regionSize.x, pt.regionSize.y);
	 
    if (point_tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return point_tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (point_tuple != NULL ) {
    		Py_DECREF(point_tuple);
  		}

  	return NULL;
}

PyObject* microscope_get_timelapse_hasRegion(PyObject *self, PyObject *args)
{
	PyObject *point_tuple = NULL; 
	TimelapseTableEntry pt; 
	
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);

	timelapse_get_point(tl, tl->current_point, &pt);

	return Py_BuildValue("i", pt.hasRegion);
}

PyObject* microscope_get_timelapse_focal_plane(PyObject *self, PyObject *args)
{
	PyObject *point_tuple = NULL; 
	TimelapseTableEntry pt; 
	
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);

	switch (tl->focus_mode)
	{
	case TIMELAPSE_USEGLOBALFOCALPLANE:
		point_tuple = Py_BuildValue("(i,d,d,d)", tl->has_global_focalPlane, tl->global_focalPlane_a, tl->global_focalPlane_b, tl->global_focalPlane_c);
		break;
	case TIMELAPSE_USEINDFOCALPLANE:
		timelapse_get_point(tl, tl->current_point, &pt);
		point_tuple = Py_BuildValue("(i,d,d,d)", pt.hasFocalPlane, pt.focalPlane_a, pt.focalPlane_b, pt.focalPlane_c);
		break;
	case TIMELAPSE_USECONSTFOCALPLANE:
		timelapse_get_point(tl, tl->current_point, &pt);
		point_tuple = Py_BuildValue("(i,d,d,d)", 1, 0.0, 0.0, pt.centre.z);  // return a valid constant focal plane, that can be treated like any other for adding focus offset
		break;
	case TIMELAPSE_USEAUTOEVERY:
		point_tuple = Py_BuildValue("(i,d,d,d)", -1, 0.0, 0.0, 0.0);  // -1 means autofocus should be used
		break;
	}
	 
    if (point_tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return point_tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (point_tuple != NULL ) {
    		Py_DECREF(point_tuple);
  		}

  	return NULL;
}

PyObject* microscope_add_timelapse_point(PyObject *self, PyObject *args) {
	double x=0, y=0, z=-DBL_MAX;
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);
		
	// Takes 3 doubles
    if(!PyArg_ParseTuple(args, "dd|d", &x, &y, &z))
        return NULL;
	
	timelapse_add_point_xyz(tl, x, y, z);
		
	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_get_timelapse_cube_options(PyObject *self, PyObject *args)
{
	PyObject *point_tuple = NULL; 
	TimelapseCubeOptions cube_opt; 
	int pos;
	
  	Microscope *ms = microscope_get_microscope();
    timelapse* tl = microscope_get_timelapse(ms);
		
	// Takes 1 int
    if(!PyArg_ParseTuple(args, "i", &pos))
        return NULL;

	timelapse_get_cube_options(tl, pos, &cube_opt);

	point_tuple = Py_BuildValue("(d,d,d)", cube_opt.exposure, cube_opt.gain, cube_opt.offset);
	 
    if (point_tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return point_tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (point_tuple != NULL ) {
    		Py_DECREF(point_tuple);
  		}

  	return NULL;
}


#endif // MICROSCOPE_PYTHON_AUTOMATION