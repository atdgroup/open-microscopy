#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "RegionScan.h"

static void PyFreeRegionScan(void *ptr)
{
	region_scan_destroy((region_scan*) ptr);

    return;
}

PyObject *wrap_new_regionscan(PyObject *self, PyObject* args)
{
	region_scan* rs = region_scan_new();

    // Wrap the pointer as a "PyCObject" and
    // return that object to the interpreter
    return PyCObject_FromVoidPtr(rs, PyFreeRegionScan);

	// use existing region scan, do not supply destroy fn ptr
//	return PyCObject_FromVoidPtr(microscope_get_region_scan(microscope_get_microscope()), NULL);
}

PyObject *regionscan_wrap_start(PyObject *self, PyObject* args)
{
	char *output_dir = NULL, *filename_prefix = NULL, *filename_ext = NULL;
	int action;
	void *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, int, string, string, string
    if(!PyArg_ParseTuple(args, "Oisss", &rs_obj, &action, &output_dir, &filename_prefix, &filename_ext)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = PyCObject_AsVoidPtr(rs_obj);

    regionscan_start((region_scan *)rs, action, output_dir, filename_prefix, filename_ext);

	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_set_roi(PyObject *self, PyObject* args)
{
	double left, top, width, height;
	void *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, double, double, double, double 
    if(!PyArg_ParseTuple(args, "Odddd", &rs_obj, &left, &top, &width, &height)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = PyCObject_AsVoidPtr(rs_obj);

	regionscan_set_roi((region_scan *)rs, left, top, width, height);
 
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_set_focal_plane(PyObject *self, PyObject* args)
{
	double focalPlane_a, focalPlane_b, focalPlane_c;
	int hasFocalPlane; 
	region_scan *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, int, double, double, double
    if(!PyArg_ParseTuple(args, "Oiddd", &rs_obj, &hasFocalPlane, &focalPlane_a, &focalPlane_b, &focalPlane_c)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = (region_scan *)PyCObject_AsVoidPtr(rs_obj);

	if (hasFocalPlane<0) {  // signal for autofocus at each point
		regionscan_set_perform_swautofocus_every_point(rs, 1);
	}
	else {  // transfer these values
		rs->roi->focal_plane_valid = hasFocalPlane;
		rs->roi->focal_point_a = focalPlane_a;
		rs->roi->focal_point_b = focalPlane_b;
		rs->roi->focal_point_c = focalPlane_c;
		rs->roi->focal_point_c_original = focalPlane_c;
	}
	
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_set_focal_offset(PyObject *self, PyObject* args)
{
	double val;
	region_scan *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, int
    if(!PyArg_ParseTuple(args, "Od", &rs_obj, &val)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = (region_scan *)PyCObject_AsVoidPtr(rs_obj);

	// restore the original focal value and add new offset so that offsets are not cummulative
	rs->roi->focal_point_c = rs->roi->focal_point_c_original + val;

	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_set_focal_offset_from_xyz(PyObject *self, PyObject* args)
{
	double x, y, z, old_z;
	region_scan *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, int
    if(!PyArg_ParseTuple(args, "Oddd", &rs_obj, &x, &y, &z)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = (region_scan *)PyCObject_AsVoidPtr(rs_obj);

	// get the current z for the x,y location
	region_of_interest_z_for_xy(rs->roi, x, y, &old_z);

	// add the offset to the current c parameter
	rs->roi->focal_point_c += z - old_z;
	rs->roi->focal_point_c_original = rs->roi->focal_point_c;

	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_set_perform_swautofocus(PyObject *self, PyObject* args)
{
	int val;
	void *rs;
	PyObject *rs_obj = NULL;

	// Takes Object, int
    if(!PyArg_ParseTuple(args, "Oi", &rs_obj, &val)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = PyCObject_AsVoidPtr(rs_obj);

	regionscan_set_perform_swautofocus_every_point((region_scan *)rs, val);
 
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_stop(PyObject *self, PyObject* args)
{
	void *rs;
	PyObject *rs_obj = NULL;

	// Takes Object
    if(!PyArg_ParseTuple(args, "O", &rs_obj)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = PyCObject_AsVoidPtr(rs_obj);

	regionscan_stop((region_scan *)rs);
 
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *regionscan_wrap_hide(PyObject *self, PyObject* args)
{
	void *rs;
	PyObject *rs_obj = NULL;

	// Takes Object
    if(!PyArg_ParseTuple(args, "O", &rs_obj)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    rs = PyCObject_AsVoidPtr(rs_obj);

	region_scan_hide((region_scan *)rs);
 
	Py_INCREF(Py_None);
 	return Py_None;
}