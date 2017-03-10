#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "stage\stage.h"

PyObject* microscope_get_stage_position(PyObject *self, PyObject *args)
{
	double x=0, y=0, z=0;
	PyObject *point_tuple = NULL; 
	
  	Microscope *ms = microscope_get_microscope();
    XYStage* stage = microscope_get_stage(ms);
	Z_Drive* z_drive = microscope_get_master_zdrive(ms);  
	
	if(stage_get_xy_position (stage, &x, &y) == STAGE_ERROR) {
		PyErr_SetString(PyExc_ValueError, "stage_get_xy_position returned error.");
		goto err_handler;	
	}
	
	// set z drive position
	if(z_drive != NULL) {
		if(z_drive_get_position(z_drive, &z) == Z_DRIVE_ERROR)
			goto err_handler;		
	}
	
	point_tuple = Py_BuildValue("(d,d,d)", x, y, z);
	 
    if (point_tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return point_tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  		if (point_tuple != NULL ) {
    		Py_DECREF(point_tuple);
  		}

  	return NULL;
}


PyObject* microscope_set_stage_position(PyObject *self, PyObject *args)
{
	double x=0, y=0, z=-DBL_MAX;

  	Microscope *ms = microscope_get_microscope();
    XYStage* stage = microscope_get_stage(ms);
	Z_Drive* z_drive = microscope_get_master_zdrive(ms);
		
	// Takes 3 doubles
    if(!PyArg_ParseTuple(args, "dd|d", &x, &y, &z))
        return NULL;
	
	if(z_drive_is_part_of_stage(z_drive) && z != -DBL_MAX) {
//	if(STAGE_VTABLE(stage, goto_xyz_position) != NULL && z != -DBL_MAX) {
		// Stage has a xyz move implemented
		if(stage_async_goto_xyz_position (stage, x, y, z) == STAGE_ERROR) {
			PyErr_SetString(PyExc_ValueError, "stage_set_xyz_position returned error.");
			goto err_handler;	
		}
		if(z_drive != NULL && z != -DBL_MAX)
			z_drive_update_current_position(z_drive, z);
	}
	else {
		if(stage_async_goto_xy_position (stage, x, y) == STAGE_ERROR) {
			PyErr_SetString(PyExc_ValueError, "stage_set_xy_position returned error.");
			goto err_handler;	
		}
		
		// set z drive position
		if(z_drive != NULL && z != -DBL_MAX) {
			if(z_drive_set_position(z_drive, z) == Z_DRIVE_ERROR)
				goto err_handler;		
		}
	}

	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_set_stage_z_position(PyObject *self, PyObject *args)
{
	double z=0.0, current_z = 0.0;

  	Microscope *ms = microscope_get_microscope();
	Z_Drive* z_drive = microscope_get_master_zdrive(ms);
		
	// Takes 3 doubles
    if(!PyArg_ParseTuple(args, "d", &z))
        return NULL;
	
	// set z drive position
	if(z_drive != NULL) {

		if(z_drive_set_position(z_drive, z) == Z_DRIVE_ERROR)
			goto err_handler;		
	}
		
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_set_stage_z_rel_position(PyObject *self, PyObject *args)
{
	double z=0.0, current_z = 0.0;

  	Microscope *ms = microscope_get_microscope();
	Z_Drive* z_drive = microscope_get_master_zdrive(ms);
		
	// Takes 3 doubles
    if(!PyArg_ParseTuple(args, "d", &z))
        return NULL;
	
	// set z drive position
	if(z_drive != NULL) {

		if(z_drive_get_position(z_drive, &current_z) == Z_DRIVE_ERROR)
			goto err_handler;		

		if(z_drive_set_position(z_drive, current_z + z) == Z_DRIVE_ERROR)
			goto err_handler;		
	}
		
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}

PyObject* microscope_wait_for_stage(PyObject *self, PyObject *args)
{
  	double delay, timeout;
    char time_str[256];
	
	Microscope *ms = microscope_get_microscope();
    XYStage* stage = microscope_get_stage(ms);

	// Takes 2 doubles
    if(!PyArg_ParseTuple(args, "dd", &delay, &timeout))
        return NULL;

	stage_set_timeout(stage, timeout);
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Started microscope_wait_for_stage timeout: %f\n", time_str, timeout);
#endif

	if(stage_wait_for_stop_moving (stage) == STAGE_ERROR) {
		PyErr_SetString(PyExc_ValueError, "stage_wait_for_stop_moving returned error.");
		goto err_handler;	
	}
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Finished microscope_wait_for_stage 1", time_str);
#endif

	Delay(delay);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf(" 2 %s\n", time_str);
#endif
	
	Py_INCREF(Py_None);
	
 	return Py_None;
	
	err_handler:
  
  		assert(PyErr_Occurred()); /* An exception must already be set. */

  	return NULL;
}
