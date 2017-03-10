#ifdef BUILD_MODULE_SCANNER

#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "scanner.h"

PyObject* microscope_scanner_set_properties(PyObject *self, PyObject *args)
{
	// Takes number of frames, Resolution, zoom, xshift, yshift, line or xy scan
    int resolution, zoom, xshift, yshift, scan_type;
    Microscope *ms = microscope_get_microscope();
    Scanner *scanner = microscope_get_scanner(ms);
	
	// Takes 5 ints
    if(!PyArg_ParseTuple(args, "iiiii", &resolution,
			&zoom, &xshift, &yshift, &scan_type))
        return NULL;
	
	scanner_set_resolution(scanner, resolution); 
	scanner_set_zoom(scanner, zoom);
	scanner_set_x_shift(scanner, xshift);
	scanner_set_y_shift(scanner, yshift);
	scanner_line_scan(scanner, scan_type);

	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject* microscope_scanner_start(PyObject *self, PyObject *args)
{
	// Takes number of frames, Resolution, zoom, xshift, yshift, line or xy scan
    int frames;
    Microscope *ms = microscope_get_microscope();
    Scanner *scanner = microscope_get_scanner(ms);   
	
	// Takes 1 ints
    if(!PyArg_ParseTuple(args, "ii", &frames))
        return NULL;
	
	scanner_start_scan(scanner, frames);     
	
	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject* microscope_scanner_stop(PyObject *self, PyObject *args)
{
	// Takes number of frames, Resolution, zoom, xshift, yshift, line or xy scan
    Microscope *ms = microscope_get_microscope();
    Scanner *scanner = microscope_get_scanner(ms);       
	
	scanner_stop_scan(scanner);         
	
	Py_INCREF(Py_None);
 	return Py_None;
}

#endif // BUILD_MODULE_SCANNER