#include "microscope_python_wrappers.h"

#include "microscope.h"
#include "CubeSlider.h"
#include "file_prefix_dialog.h" 
#include "BasicWin32Window.h"

PyObject* microscope_get_user_data_dir(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
	char path[GCI_MAX_PATHNAME_LEN];
	PyObject *out_string = NULL;   
	
	microscope_get_user_data_directory(ms, path);
	
	out_string = Py_BuildValue("s", path);
	 
    if (out_string == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return out_string;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (out_string != NULL ) {
    		Py_DECREF(out_string);
  		}
		
		return NULL;    
}


PyObject* microscope_file_sequence_dialog(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
    char *default_dir = NULL;
	char output_dir[GCI_MAX_PATHNAME_LEN], output_filename[GCI_MAX_PATHNAME_LEN];
	PyObject *tuple = NULL;   
	
	// Takes 1 string
    if(!PyArg_ParseTuple(args, "s", &default_dir)) {
        goto err_handler;  
	}
	
	if(FilePrefixSaveDialog(0, default_dir, output_dir, output_filename) < 0) {
		Py_INCREF(Py_None);
 		return Py_None;  	
	}	

	tuple = Py_BuildValue("(s,s)", output_dir, output_filename);
	 
    if (tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (tuple != NULL ) {
    		Py_DECREF(tuple);
  		}
		
		return NULL;    
}


PyObject* microscope_simple_file_sequence_dialog(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
    int err;
	char *default_dir = NULL;
	char output_dir[GCI_MAX_PATHNAME_LEN], output_filename[GCI_MAX_PATHNAME_LEN], output_ext[20];
	PyObject *tuple = NULL;   
	
	// Takes 1 string
    if(!PyArg_ParseTuple(args, "s", &default_dir)) {
        goto err_handler;  
	}
	
	err = SimpleFilePrefixSaveDialog(0, default_dir, output_dir, output_filename, output_ext);

	// User pressed cancel return None object
	if(err == -1) {
		Py_INCREF(Py_None);
 		return Py_None;  
	}

	if(err <= -2){
		PyErr_SetString(PyExc_ValueError, "SimpleFilePrefixSaveDialog returned error.");
		goto err_handler;   	
	}	

	tuple = Py_BuildValue("(s,s,s)", output_dir, output_filename, output_ext);
	 
    if (tuple == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return tuple;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (tuple != NULL ) {
    		Py_DECREF(tuple);
  		}
		
		return NULL;    
}

PyObject* microscope_parse_file_sequence_value(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
    int sequence_num = 0;
	char *file_string = NULL;
	char output_filename[GCI_MAX_PATHNAME_LEN];
	PyObject *out_string = NULL;   
	
	output_filename[0] = 0;

	// Takes 1 string, 1 int
    if(!PyArg_ParseTuple(args, "si", &file_string, &sequence_num)) {
        goto err_handler;  
	}
	
	FilePrefixParseString(file_string, sequence_num, output_filename);  
		
	out_string = Py_BuildValue("s", output_filename);
	 
    if (out_string == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return out_string;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (out_string != NULL ) {
    		Py_DECREF(out_string);
  		}
		
		return NULL;    
}

void insert_text_into_filename(char* filename, char* output, char *text)
{
	// PRB Jan 2010
	char ext[16];
	char *p=NULL;

	if (filename==NULL) return;
	if (text==NULL) return;
	if (output==NULL) return;

	strcpy(output, filename);

	// store the extension
	p = strrchr(output, '.');
	if (p!=NULL){
		strcpy(ext, p);
	
		// truncate the file name at the '.'
		p[0] = 0;
	}

	// append the text
	strcat(output, "_");
	strcat(output, text);

	// reform the filename
	if (p!=NULL)
		strcat(output, ext);
}

PyObject* microscope_insert_cube_into_filename(PyObject *self, PyObject *args)
{
    Microscope *ms = microscope_get_microscope();
    int cube_num = 0;
	char *file_string = NULL;
	char cubename[256]="";
	char output_filename[GCI_MAX_PATHNAME_LEN];
	PyObject *out_string = NULL;   
	
	// Takes 1 string, 1 int
    if(!PyArg_ParseTuple(args, "si", &file_string, &cube_num)) {
        goto err_handler;  
	}

	// get the name of the required cube from the mscope
	device_conf_get_device_name_for_pos((ModuleDeviceConfigurator *)(microscope_get_cube_manager(ms)->dc), cubename, cube_num);

	// insert the name of the cube into the filename
	insert_text_into_filename (file_string, output_filename, cubename);
		
	out_string = Py_BuildValue("s", output_filename);
	 
    if (out_string == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return out_string;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (out_string != NULL ) {
    		Py_DECREF(out_string);
  		}
		
		return NULL;    
}

PyObject* microscope_insert_text_into_filename(PyObject *self, PyObject *args)
{
	char *file_string = NULL, *text_string = NULL;
	char output_filename[GCI_MAX_PATHNAME_LEN];
	PyObject *out_string = NULL;   

	// Takes 2 strings
    if(!PyArg_ParseTuple(args, "ss", &file_string, &text_string)) {
        goto err_handler;  
	}

	// insert the text into the filename
	insert_text_into_filename (file_string, output_filename, text_string);
	
	out_string = Py_BuildValue("s", output_filename);
	 
    if (out_string == NULL) {
      	PyErr_NoMemory();
      	goto err_handler;
    }
	
	return out_string;
	
	err_handler:
  
  		assert(PyErr_Occurred()); // An exception must already be set. 

  		if (out_string != NULL ) {
    		Py_DECREF(out_string);
  		}
		
		return NULL;    
}


/*
PyObject* microscope_display_win32_window(PyObject *self, PyObject *args)
{
    char *title = NULL;
	int top, left, width, height;
	FIBITMAP *dib;
	PyObject *dib_obj = NULL;

	// Takes Object, int, string, string, string
    if(!PyArg_ParseTuple(args, "siiiiO", &title, &left, &top, &width, &height, &dib_obj)) {
        return NULL;
	}

	// Convert the PyCObject to a void pointer:
    dib = (FIBITMAP*) PyCObject_AsVoidPtr(dib_obj);

    BasicWin32Window(title, left, top, width, height, dib);

	FreeImage_Unload(dib);

	Py_INCREF(Py_None);
 	return Py_None;

}
*/