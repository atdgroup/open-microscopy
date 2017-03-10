#include "microscope_python_wrappers.h"

#include "microscope.h"
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
    char average = 0;

	// Takes 1 optional bool for the average number of frames
    if(!PyArg_ParseTuple(args, "|b", &average))
        return NULL;

	if(average > 1) {
		if(gci_camera_snap_average_image(camera, average) == CAMERA_ERROR) {
			PyErr_SetString(PyExc_ValueError, "gci_camera_snap_average_image returned error.");
			return NULL;	
		}
	}
	else {
		if(gci_camera_snap_image(camera) == CAMERA_ERROR) {
			PyErr_SetString(PyExc_ValueError, "gci_camera_snap_image returned error.");
			return NULL;	
		}
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
    
	// Takes 1 double
    if(!PyArg_ParseTuple(args, "d", &time))
        return NULL;
	
	err = gci_camera_set_exposure_time(camera, time);
		
    if(err == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_set_exposure_time returned error.");
		return NULL;	
	}
	
	return Py_BuildValue("i", err);
}

PyObject* microscope_set_gain(PyObject *self, PyObject *args)
{
    int err;
	double gain;
    Microscope *ms = microscope_get_microscope();
    GciCamera *camera = microscope_get_camera(ms);
    
	// Takes 1 double
    if(!PyArg_ParseTuple(args, "d", &gain))
        return NULL;
	
	err = gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, gain);
		
    if(err == CAMERA_ERROR) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_set_gain returned error.");
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

static void PyFreeFreeImageDib(void *ptr)
{
	if(ptr != NULL)
		FreeImage_Unload((FIBITMAP*) ptr);

    return;
}

PyObject* microscope_get_microns_per_pixel(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	GciCamera *camera = microscope_get_camera(ms);   

	double val = gci_camera_get_true_microns_per_pixel(camera);

	return Py_BuildValue("d", val);
}

PyObject* microscope_get_image(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	GciCamera *camera = microscope_get_camera(ms);   
    FIBITMAP* dib = NULL;
	char average = 0;

	// Takes 1 optional bool for the average number of frames
    if(!PyArg_ParseTuple(args, "|b", &average))
        goto Error;	

	if(average > 1) {
		if((dib = gci_camera_get_image_average_for_frames(camera, average)) == NULL) {
			PyErr_SetString(PyExc_ValueError, "gci_camera_get_image_average_for_frames returned NULL.");
			goto Error;		
		}
	}
	else {
		if((dib = gci_camera_get_image(camera, NULL)) == NULL) {
			PyErr_SetString(PyExc_ValueError, "microscope_save_current_displayed_image returned NULL.");
			goto Error;		
		}
	}

	return PyCObject_FromVoidPtr(dib, PyFreeFreeImageDib);

	Error:

		if (dib!=NULL) 
			FreeImage_Unload(dib);
	
 	return NULL;
}

PyObject* microscope_save_current_displayed_image(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	GciCamera *camera = microscope_get_camera(ms);   
    FIBITMAP* dib = NULL;
	char *filepath = NULL;
	
	// Takes 1 string
    if(!PyArg_ParseTuple(args, "s", &filepath))
        goto Error;	  
	
	if((dib = gci_camera_get_displayed_image(camera)) == NULL) {
		PyErr_SetString(PyExc_ValueError, "microscope_save_current_displayed_image returned NULL.");
		goto Error;		
	}
	
	if(microscope_saveimage(ms, dib, filepath) == MICROSCOPE_ERROR) {
		PyErr_SetString(PyExc_ValueError, "microscope_save_current_displayed_image returned error.");
		goto Error;	
	}	

	if (dib!=NULL) 
		FreeImage_Unload(dib);
	
	Py_INCREF(Py_None);
	return Py_None;

	Error:
		if (dib!=NULL) 
			FreeImage_Unload(dib);
	
 	return NULL;
}

PyObject* microscope_save_widefield_metadata(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	GciCamera *camera = microscope_get_camera(ms);   
	char *filepath = NULL;
	
	// Takes 1 string
    if(!PyArg_ParseTuple(args, "s", &filepath))
        goto Error;	  

    GCI_ImagingWindow_SaveMetaDataToTextFile(camera->_camera_window, filepath);
	
	Py_INCREF(Py_None);
	return Py_None;

	Error:
	
 	return NULL;
}

PyObject* microscope_snap_and_save_image(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	GciCamera *camera = microscope_get_camera(ms);   
    FIBITMAP* dib = NULL;
	char *filepath = NULL, average = 0;
	
	// Takes 1 string, optional average count
    if(!PyArg_ParseTuple(args, "s|b", &filepath, &average))
        goto Error;	  
	
	if(average > 1) {
		if(gci_camera_snap_average_image(camera, average) == CAMERA_ERROR) {
			PyErr_SetString(PyExc_ValueError, "gci_camera_snap_average_image returned error.");
			goto Error;		
		}
	}
	else {
		if(gci_camera_snap_image(camera) == CAMERA_ERROR) {
			PyErr_SetString(PyExc_ValueError, "gci_camera_snap_image returned error.");
			goto Error;		
		}
	}
	
	if((dib = gci_camera_get_displayed_image(camera)) == NULL) {
		PyErr_SetString(PyExc_ValueError, "gci_camera_get_displayed_image returned NULL.");
		goto Error;		
	}
	
	if(microscope_saveimage(ms, dib, filepath) == MICROSCOPE_ERROR) {
		PyErr_SetString(PyExc_ValueError, "microscope_saveimage returned error.");
		goto Error;	
	}	

	if (dib!=NULL) 
		FreeImage_Unload(dib);
	
	Py_INCREF(Py_None);
	return Py_None;

	Error:
		if (dib!=NULL) 
			FreeImage_Unload(dib);
	
 	return NULL;
}