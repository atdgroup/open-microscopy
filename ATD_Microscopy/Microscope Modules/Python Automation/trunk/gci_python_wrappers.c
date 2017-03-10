#include "gci_python_wrappers.h"

#include "gci_utils.h"
#include "gci_menu_utils.h"

#include "toolbox.h"

/************************* CallMenuCallback **********************************************/

static int CallMenuCallback(int panel, int menuItem)
{
	int menubar = GetPanelMenuBar(panel);
	void *callback=NULL, *data=NULL;
	MenuCallbackPtr callback_fun;
	
	if(menubar < 0)
		return -1;

	// Get the ptr to the callback
	GetMenuBarAttribute(menubar, menuItem, ATTR_CALLBACK_FUNCTION_POINTER, &callback);
	
	// Get the callback data
	GetMenuBarAttribute(menubar, menuItem, ATTR_CALLBACK_DATA, &data);
	
	callback_fun = (MenuCallbackPtr) callback;
	
	// Call the ptr
	if(callback_fun != NULL)
		(*callback_fun)(menubar, menuItem, data, panel);
	
	return 0;
}

/************************* gci_py_callctrlcallback **********************************************/

static PyObject* gci_py_callctrlcallback(PyObject *self, PyObject *args)
{
	int panel, ctrl, event, eventData1, eventData2, ret, err;
	void *callback = NULL;

	// Takes 5 integers
    if(!PyArg_ParseTuple(args, "iiiii", &panel, &ctrl, &event, &eventData1, &eventData2))
        return NULL;
	
	GetCtrlAttribute(panel, ctrl, ATTR_CALLBACK_FUNCTION_POINTER, &callback);  
	
	if(callback == NULL)
		return Py_BuildValue("i", 0); 
	
	err = CallCtrlCallback (panel, ctrl, event, eventData1, eventData2, &ret);
		
	if(err < 0) {
		
		PyErr_SetString(PyExc_ValueError, "CallCtrlCallback returned error.");
		return NULL;	
	}
	
    return Py_BuildValue("i", ret);
}

/************************* gci_py_get_menu_id_from_path **********************************************/

static PyObject* gci_py_get_menu_id_from_path(PyObject *self, PyObject *args)
{
	int menubar_id, err, id;
	char *path = NULL;
	
	// Takes 1 integers, 1 string
    if(!PyArg_ParseTuple(args, "is", &menubar_id, &path))
        return NULL;
	
	err = GetLastItemIdFromPath (menubar_id, path, &id);
		
	if(err < 0) {
		
		if(err == -1)
			PyErr_SetString(PyExc_ValueError, "GetIdForTopLevelMenu returned error.");
		else
			PyErr_SetString(PyExc_ValueError, "FindMenuItemIdFromNameInMenu.");  
			
		return NULL;	
	}
	
    return Py_BuildValue("i", id);
}

/************************* gci_py_call_menu_callback **********************************************/

static PyObject* gci_py_call_menu_callback(PyObject *self, PyObject *args)
{
	int panel_id, menu_item_id, err;
	
	// Takes 2 integers
    if(!PyArg_ParseTuple(args, "ii", &panel_id, &menu_item_id))
        return NULL;
	
	err = CallMenuCallback(panel_id, menu_item_id);
		
	if(err == -1) {
		
		PyErr_SetString(PyExc_ValueError, "CallMenuCallback returned error. No such menubar_id");
		return NULL;	
	}
	
    return Py_BuildValue("i", err);
}

/************************* gci_py_set_panel_focus **********************************************/

static PyObject* gci_py_set_panel_focus(PyObject *self, PyObject *args)
{
	int panel_id, err, old_panel_id;
	
	// Takes 1 integer
    if(!PyArg_ParseTuple(args, "i", &panel_id))
        return NULL;
	
	old_panel_id = GetActivePanel();
	err = SetActivePanel(panel_id);
	if (!err)
	{
		CallPanelCallback (old_panel_id, EVENT_LOST_FOCUS, panel_id, 0, 0);
		CallPanelCallback (panel_id, EVENT_GOT_FOCUS, old_panel_id, 0, 0);
	}
	
	if(err == -1) {
		
		PyErr_SetString(PyExc_ValueError, "CallMenuCallback returned error. No such menubar_id");
		return NULL;	
	}
	
    return Py_BuildValue("i", err);
}

/************************* gci_py_show_status **********************************************/

static PyObject* gci_py_show_status(PyObject *self, PyObject *args)
{
	char *title = NULL;     
	char *msg = NULL;     

	// Takes 2 integers
    if(!PyArg_ParseTuple(args, "ss", &title, &msg))
        return NULL;
	
	feedback_show("GCI Python", msg);
	
    Py_INCREF(Py_None);
 	return Py_None;
}


/************************* Gci_Py_Methods **********************************************/

PyMethodDef Gci_Py_Methods[] = {
	
    {"callcallback", gci_py_callctrlcallback, METH_VARARGS,
     "Calls the callback associated with a cvi control."},
	 
	{"get_menu_id_from_path", gci_py_get_menu_id_from_path, METH_VARARGS,
     "Returns a menuid from a path like File//Open."},
	
	{"call_menu_callback", gci_py_call_menu_callback, METH_VARARGS,
     "Calls a menu callback."},
	
	{"set_focus", gci_py_set_panel_focus, METH_VARARGS,
     "Sets ui focus to a panel."},
	 
	{"ShowStatus", gci_py_show_status, METH_VARARGS,
     "Prints text in a dialog box."},
	 
    {NULL, NULL, 0, NULL}
};
