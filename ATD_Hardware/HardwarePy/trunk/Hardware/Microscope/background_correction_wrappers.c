#include "microscope_python_wrappers.h"

#include "camera\gci_camera.h"

PyObject* microscope_set_camera_live_mode(PyObject *self, PyObject *args)
{
    int mode, err;
    Microscope *ms = microscope_get_microscope();
    GciCamera *camera = microscope_get_camera(ms);
    
	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "i", &mode))
        return NULL;
	
    if(mode)
        err = gci_camera_set_live_mode(camera);
	else
        err = gci_camera_set_snap_mode(camera);
    
	if(err == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_set_live_mode returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
 	return Py_None;
}


PyObject* microscope_snap_image(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
    GciCamera *camera = microscope_get_camera(ms);
    
	if(gci_camera_snap_image(camera) == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_snap_image returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
 	return Py_None;
}


PyObject* microscope_set_exposure(PyObject *self, PyObject *args)
{
    int err;
	double time;
    Microscope *ms = microscope_get_microscope();
    GciCamera *camera = microscope_get_camera(ms);
    
	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "d", &time))
        return NULL;
	
	err = gci_camera_set_exposure_time(camera, time);
		
    if(err == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_set_exposure_time returned error.");
		return NULL;	
	}
	
	return Py_BuildValue("i", err);
}


PyObject* microscope_perform_autoexposure(PyObject *self, PyObject *args)
{
	// The default range to search is 1 to 4000 milliseconds (4 seconds)
    double min_exposure = 1.0, max_exposure = 4000.0;
    Microscope *ms = microscope_get_microscope();
    GciCamera *camera = microscope_get_camera(ms);
    
	// Takes 5 integers
    if(!PyArg_ParseTuple(args, "|dd", &min_exposure, &max_exposure))
        return NULL;
	
    if(gci_camera_autoexposure(camera, min_exposure, max_exposure) == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_autoexposure returned error.");
		return NULL;	
	}
	
	Py_INCREF(Py_None);
 	return Py_None;
}