#include "microscope_python_wrappers.h"

#include "microscope.h"

PyObject* microscope_sw_autofocus(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
  
	sw_autofocus_autofocus(ms->_sw_af);     
	
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject* microscope_sw_autofocus_abort(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
  
	sw_autofocus_autofocus_abort(ms->_sw_af);     
	
	Py_INCREF(Py_None);
 	return Py_None;
}