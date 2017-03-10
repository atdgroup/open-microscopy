#include "microscope_python_wrappers.h"

#include "microscope.h"

PyObject *background_correction_enable_wrap(PyObject *self, PyObject* args)
{
	int enable;
	Microscope *ms = microscope_get_microscope();
    ref_images* bc = microscope_get_background_correction(ms);

	// Takes Object, int, string, string, string
    if(!PyArg_ParseTuple(args, "i", &enable)) {
        return NULL;
	}

	if(enable)
		ref_images_enable(bc);
	else
		ref_images_disable(bc);

	Py_INCREF(Py_None);
 	return Py_None;
}

PyObject *background_correction_can_process_wrap(PyObject *self, PyObject* args)
{
	int can_process = 0;
	Microscope *ms = microscope_get_microscope();
    ref_images* bc = microscope_get_background_correction(ms);

	can_process = ref_images_can_process(bc);

	return Py_BuildValue("i", can_process);
}