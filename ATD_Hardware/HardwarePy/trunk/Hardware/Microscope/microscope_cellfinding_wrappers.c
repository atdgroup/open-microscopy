#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "cell_finding.h"

static void PyFreeCellFinding(void *ptr)
{
	cell_finder_destroy((cell_finder*) ptr);

    return;
}

PyObject *wrap_new_cellfinding(PyObject *self, PyObject* args)
{
	cell_finder* cf = cell_finder_new();

    // Wrap the pointer as a "PyCObject" and
    // return that object to the interpreter
    return PyCObject_FromVoidPtr(cf, PyFreeCellFinding);
}