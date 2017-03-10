#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "ZDrive.h"

PyObject* microscope_zdrive_set_pos(PyObject *self, PyObject *args)
{
	// Takes number of frames, Resolution, zoom, xshift, yshift, line or xy scan
	double focus_microns;
    Microscope *ms = microscope_get_microscope();
    Z_Drive *zdrive = microscope_get_master_zdrive(ms);       
	
	// Takes 1 doubles
    if(!PyArg_ParseTuple(args, "d", &focus_microns))
        return NULL;
	
	z_drive_set_position(zdrive, focus_microns);          
	
	Py_INCREF(Py_None);
 	return Py_None;
}