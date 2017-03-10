#ifdef BUILD_MODULE_BATCHCOUNTER

#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "ATD_BatchCounter_A1.h"

PyObject* microscope_batchcounter_start(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);

	if(atd_batch_counter_start (bc) == BATCHCOUNTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "batch counter returned error.");
		goto err_handler;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}


PyObject* microscope_batchcounter_reset(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);

	if(atd_batch_counter_reset (bc) == BATCHCOUNTER_ERROR) {
		PyErr_SetString(PyExc_ValueError, "batch counter returned error.");
		goto err_handler;	
	}
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_batchcounter_wait_for_counts(PyObject *self, PyObject *args)
{
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);
	double timeout = 1.0;

	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "d", &timeout))
        return NULL;

	atd_batch_counter_a1_wait_for_counts (bc, timeout);
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}


// Added by MJM 14/01/2010
PyObject* microscope_batchcounter_beamOn(PyObject *self, PyObject *args) {
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);

	atd_batch_counter_a1_set_beam_control_mode(bc, BEAM_ON); // 0 is beam On
	
	Py_INCREF(Py_None);
	
 	return Py_None;
}

PyObject* microscope_batchcounter_beamOff(PyObject *self, PyObject *args) {
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);
	atd_batch_counter_a1_set_beam_control_mode (bc, BEAM_OFF);
	Py_INCREF(Py_None);
	
 	return Py_None;
}

// Added by MJM 02/03/2010
PyObject * microscope_batchcounter_read_counts(PyObject *self, PyObject *args) {
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);
	//atd_batch_counter_a1_set_beam_control_mode (bc, BEAM_OFF);
	int * counts;
	atd_batch_counter_read_number_of_counts(bc, counts);

	return counts;
}

PyObject* microscope_batchcounter_set_counts(PyObject *self, PyObject *args) {
	Microscope *ms = microscope_get_microscope();
    BatchCounterA1 *bc = microscope_get_batchcounter(ms);
	double counts = 1.0;

	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "d", &counts))
        return NULL;
	atd_batch_counter_a1_set_number_of_counts(bc, (int)counts);
	Py_INCREF(Py_None);
	
 	return Py_None;
}


#endif // BUILD_MODULE_BATCHCOUNTER
