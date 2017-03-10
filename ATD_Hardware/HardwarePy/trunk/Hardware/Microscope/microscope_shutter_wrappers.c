#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "shutter.h"

PyObject* microscope_shutter_open(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Shutter *shutter = microscope_get_shutter(ms);
	
    if(shutter_open(shutter) == SHUTTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "shutter_open returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_shutter_close(PyObject *self, PyObject *args)
{
  	Microscope *ms = microscope_get_microscope();
	Shutter *shutter = microscope_get_shutter(ms);
	
    if(shutter_close(shutter) == SHUTTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "shutter_close returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}
 
PyObject* microscope_is_shutter_open(PyObject *self, PyObject *args)
{
	int status;
	
  	Microscope *ms = microscope_get_microscope();
	Shutter *shutter = microscope_get_shutter(ms);
	
    if(shutter_status(shutter, &status) == SHUTTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "shutter_status returned error.");
		return NULL;	
	}
	
	return Py_BuildValue("i", status);
}

// A time <= 0 will be interpreted as infinity
// open_time is is milli seconds  
PyObject* microscope_set_shutter_open_time(PyObject *self, PyObject *args)
{
	double time;
	
  	Microscope *ms = microscope_get_microscope();
	Shutter *shutter = microscope_get_shutter(ms);
	
	// Takes 1 integers
    if(!PyArg_ParseTuple(args, "d", &time))
        return NULL;
	
    if(shutter_set_open_time(shutter, time) == SHUTTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "shutter_set_open_time returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}