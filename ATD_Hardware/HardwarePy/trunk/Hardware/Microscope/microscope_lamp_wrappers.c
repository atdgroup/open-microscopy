#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "lamp.h"

PyObject* microscope_lamp_on(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    Lamp *lamp = microscope_get_lamp(ms);

	if(lamp_on (lamp) == LAMP_ERROR) {
		PyErr_SetString(PyExc_ValueError, "lamp_on returned error.");
		goto err_handler;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_lamp_off(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    Lamp *lamp = microscope_get_lamp(ms);

	if(lamp_off (lamp) == LAMP_ERROR) {
		PyErr_SetString(PyExc_ValueError, "lamp_off returned error.");
		goto err_handler;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_lamp_set_intensity(PyObject *self, PyObject *args)
{
  	double intensity;
	
	Microscope *ms = microscope_get_microscope();
    Lamp *lamp = microscope_get_lamp(ms);

		// Takes 1 doubles
    if(!PyArg_ParseTuple(args, "d", &intensity))
        goto err_handler;

	if(lamp_set_intensity (lamp, intensity) == LAMP_ERROR) {
		PyErr_SetString(PyExc_ValueError, "lamp_off returned error.");
		goto err_handler;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

