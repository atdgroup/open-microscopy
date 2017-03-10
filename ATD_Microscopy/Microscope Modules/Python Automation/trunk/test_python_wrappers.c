//==============================================================================
//
// Title:       test_python_wrappers.c
// Purpose:     Project specific python extensions.
//                    Should use:		Py_InitModule("blur", Blur_Py_Methods); PyRun_SimpleString("import blur");      
//                    to make the fns available for scripting with calls like blur.fn()
//
// Created on:  28/08/2007 at 15:39:01 by Paul Barber.
// Copyright:   Gray Cancer Institute. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "test_python_wrappers.h"

static PyObject*
test_py_some_function(PyObject *self, PyObject *args)
{
	char *msg = NULL;
	int ret;

	// Takes 1 string
    if(!PyArg_ParseTuple(args, "s", &msg))
        return NULL;
	
	MessagePopup(msg, "A project specific function.");
	
    Py_INCREF(Py_None);
 	return Py_None;
}


PyMethodDef Test_Py_Methods[] = {

	{"testFunction", test_py_some_function, METH_VARARGS,
     "Demonstrates a project specific python extension."},
	 
    {NULL, NULL, 0, NULL}
};
