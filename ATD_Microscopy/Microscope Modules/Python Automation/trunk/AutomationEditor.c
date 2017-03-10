/* Notes:
	
	If a main callback is used by the host project, it is not currently supported. There is no CallMainCallback() fn in cvi!
	Same for any menuDimmer callback.

*/


#include "gci_python_wrappers.h" 

#include "AutomationEditor.h" 
#include "PythonEditor.h"

#ifdef MICROSCOPE_PYTHON_AUTOMATION    
#include "microscope.h"
#endif
	
#include "status.h"

#include "toolbox.h"         

#include "Scintilla.h"
#include "SciLexer.h"

#include "gci_utils.h"
#include "gci_menu_utils.h"

#include <utility.h>
#include <userint.h>

#define blockSize  (128 * 1024)


static char example[] = "#To get help on an object type help(object) ie:\n" 
						"#help(UIController)\n"
						"#help(Panel)\n\n"
						"# EXAMPLE\n"
						"panel = ui.GetPanelByTitle(\"Microscope\")\n"
						"#panel.PressButton(\"Live\")\n"
						"panel.CheckButtonOn(\"Snap\")\n"
						"#EXAMPLE\n";


// Automation editor should be a singleton (one per application)
static AutomationEditor *editor = NULL;

static const char KeyWords[] = "and continue else for "
			"import not raise assert def except from " 
			"in or return break del exec global is pass "
			"try class elif finally if lambda print while";

/// Scintilla Colors structure
typedef struct 
{   int         iItem;
    COLORREF    rgb;
} SScintillaColors ;

#define black   0x00000000
#define white   0x00FFFFFF
#define green   0x00008000
#define red     0x000000FF
#define blue    0x00FF0000
#define yellow  0x00BBFFFF
#define magenta 0x00000FF
#define cyan 	0x00FFFF00

#define PY_WHITESPACE 0
#define PY_COMMENT	1
#define PY_NUMBER	2
#define	PY_STRING 	3
#define PY_SQSTRING 4
#define PY_KEYWORD  5
#define PY_TRIPPLE_QUOTES 6
#define PY_TRIPLE_DOUBLE_QUOTES 7
#define PY_CLASS_NAME_DEF 8
#define PY_FUN_OR_METHED_DEF 9
#define PY_OPERATOR 	10
#define PY_IDENTIFIER	11
#define PY_COMMENT_BLOCKS 12
#define PY_END_OF_LINE_WHEN_STR_NOT_CLOSED 13
#define PY_HIGHLIGTED_IDENTIFIERS 14
#define PY_DECORATORS 15
#define PY_MATCHED_OPERATORS 16


/// Default color scheme
SScintillaColors g_rgbSyntaxCpp[10] =
{
    {   PY_COMMENT,          green },
    {   PY_COMMENT_BLOCKS,      green },
    {   PY_NUMBER,           red },
    {   PY_STRING,           magenta },
    {   PY_SQSTRING,        magenta },
	{   PY_TRIPPLE_QUOTES,        magenta },
	{   PY_TRIPLE_DOUBLE_QUOTES,        magenta },
    {   PY_OPERATOR,             red },
    {   PY_KEYWORD,             blue },
    {   -1,                     0 }
};


static void BlankEditor(AutomationEditor *editor)
{
	SendMessage(editor->hwndScintilla, SCI_CLEARALL, 0, 0);     
	SendMessage(editor->hwndScintilla, EM_EMPTYUNDOBUFFER, 0, 0);     
	SendMessage(editor->hwndScintilla, SCI_SETSAVEPOINT, 0, 0);   
}

static void GetRange(AutomationEditor *editor, int start, int end, char *text) {
	
	TEXTRANGE tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = text;

	SendMessage(editor->hwndScintilla, EM_GETTEXTRANGE, 0, (LPARAM)(&tr));   
}


LRESULT CALLBACK EditorWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	AutomationEditor *editor = (AutomationEditor *) data;
	
	switch(message) {
			
		case WM_SIZE:
    	case WM_EXITSIZEMOVE:
    	{
			int width, height, width_of_editors, height_of_editor, height_of_output;
			
			GetPanelAttribute(editor->panel_id, ATTR_WIDTH, &width);
			GetPanelAttribute(editor->panel_id, ATTR_HEIGHT, &height); 
			
			width_of_editors = width - 20;
			height_of_editor = height - 110; 
			
			height_of_output = height -  height_of_editor - 90;
				
			SetWindowPos(editor->hwndScintilla, NULL, 10, 35, width_of_editors, height_of_editor,
				SWP_NOZORDER | SWP_NOACTIVATE);
	  		
			// Move Run Button
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_RUN_BUTTON, ATTR_LEFT, width - 65);
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_RUN_BUTTON, ATTR_TOP, height - 65);  
			
			// Move Exe Button
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_RUN_BUTTON_2, ATTR_LEFT, width - 65);
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_RUN_BUTTON_2, ATTR_TOP, height - 35);  
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_EXELINE, ATTR_TOP, height - 33);  
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_EXELINE, ATTR_WIDTH, width - 110);  
			
			// Move Validate Button
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_VALIDATE_BUTTON, ATTR_LEFT, width - 130);
			SetCtrlAttribute(editor->panel_id, EDIT_PANEL_VALIDATE_BUTTON, ATTR_TOP, height - 65);  
			
			break;
    	}


      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) editor->old_wndproc,
							hwnd, message, wParam, lParam);
}





void SaveFile(AutomationEditor *editor, const char *filepath)
{
	FILE *fp = fopen(filepath, "wb");
	int i;
	
	if (fp) {
		
		char data[blockSize + 1];
		int lengthDoc = SendMessage(editor->hwndScintilla, SCI_GETLENGTH, 0, 0);   
		
		for (i = 0; i < lengthDoc; i += blockSize) {
			
			int grabSize = lengthDoc - i;
			
			if (grabSize > blockSize)
				grabSize = blockSize;
			
			GetRange(editor, i, i + grabSize, data);
			
			fwrite(data, grabSize, 1, fp);
		}
		
		fclose(fp);
		
		SendMessage(editor->hwndScintilla, SCI_SETSAVEPOINT, 0, 0);  
		
	} else {
		
		char msg[MAX_PATH + 100];
		
		strcpy(msg, "Could not save file \"");
		strcat(msg, filepath);
		strcat(msg, "\".");
	
		GCI_MessagePopup("Error", msg);
	}
}

/*
static int load_file(char *fname, unsigned char** result)
{
	int size = 0;
	FILE *f = fopen(fname, "rb");

	if (f == NULL)
	{
		*result = NULL;
		return -1;
	}
 
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	*result = (unsigned char *) malloc(size+1);
	fseek(f, 0, SEEK_SET);
	size = fread(*result, sizeof(unsigned char), size, f);
	
	return size;
}

void RunScript()
{
	int size;
	unsigned char *python_code;
	PyObject *codeobj, *mainobj;
	size = load_file("multiply.pyc", &python_code);

	codeobj = PyMarshal_ReadObjectFromString(python_code+8, size-8);
	mainobj = PyImport_ExecCodeModule("multiply", codeobj);

}
*/

/*
static void RunScript()
{
	int count = 0;
	char filepath[GCI_MAX_PATHNAME_LEN]="", name[500] = "", name_without_ext[500] = "";  
	PyObject *pDict;

	PyObject *pName;

	if(tl->pModule != NULL)
		Py_DECREF(tl->pModule);

	SplitPath (filepath, NULL, NULL, name);

	if(strcmp(name, tl->last_script_filename) != 0) {

		get_file_without_extension(name, name_without_ext);
		pName = PyString_FromString(name_without_ext);

		tl->pModule = PyImport_Import(pName);

		// Hack to make sure module is recompiled 
		if(tl->pModule != NULL) {
			Py_DECREF(tl->pModule);
			tl->pModule = PyImport_ReloadModule(tl->pModule);
		}

		Py_DECREF(pName);
	}
	else {
		
		 if (tl->pModule == NULL) {
			goto FAIL;
		 }

		 tl->pModule = PyImport_ReloadModule(tl->pModule);
	}

	if (tl->pModule == NULL) {
		PyErr_Print();
		strcpy(tl->last_script_filename, "");
		goto FAIL;
	}

	// pDict is a borrowed reference don't dec ref count
	pDict = PyModule_GetDict(tl->pModule);

	tl->on_start_callable = PyDict_GetItemString(pDict, "OnStart");

	if(!PyCallable_Check(tl->on_start_callable)) {
		GCI_MessagePopup("Python Script Error", "OnStart Not callable!");	
		goto FAIL;
	}

	tl->on_abort_callable = PyDict_GetItemString(pDict, "OnAbort");

	if(!PyCallable_Check(tl->on_abort_callable)) {
		GCI_MessagePopup("Python Script Error", "OnAbort Not callable!");	
		goto FAIL;
	}

	tl->on_cycle_start_callable = PyDict_GetItemString(pDict, "OnCycleStart");   

	if(!PyCallable_Check(tl->on_cycle_start_callable)) {
		GCI_MessagePopup("Python Script Error", "OnCycleStart Not callable!");	
		goto FAIL;
	}

    tl->on_point_changed_callable = PyDict_GetItemString(pDict, "OnNewPoint");

	if(!PyCallable_Check(tl->on_point_changed_callable)) {
		GCI_MessagePopup("Python Script Error", "OnNewPoint Not callable!");	
		goto FAIL;
	}

	strcpy(tl->last_script_filename, name);

	return TIMELAPSE_SUCCESS;

FAIL:

	return TIMELAPSE_ERROR;
}
*/

void RunPython (void *callbackData, int execute)
{
	char tmp[500];
	PyObject *pstr,  *main_dict;
	AutomationEditor *editor = (AutomationEditor *) callbackData;		
	static PyObject *main_module = NULL;

	SaveFile(editor, editor->tmp_filepath);
	
	feedback_show("GCI Python", "******************************\n");
		
	// Get a reference to the main module.
	main_module = PyImport_AddModule("__main__");  
	//main_module = PyImport_ReloadModule("__main__");  

	// Make sure module is recompiled 
	//if(main_module != NULL) {
	//	Py_DECREF(main_module);
	//	main_module = PyImport_ReloadModule(main_module);
	//}
	//else {
		
		
	//}

	// Get the main module's dictionary
	// and make a copy of it.
	main_dict = PyModule_GetDict(main_module);
	
	PyRun_SimpleString("from UIControl import UIController, Panel");      

	PyRun_SimpleString("ui = UIController(debug=0)");      

	sprintf(tmp, "ui.execute_mode = %d", execute);
	PyRun_SimpleString(tmp);      

	sprintf(tmp, "execfile( r'%s' )", editor->tmp_filepath);

	pstr = PyRun_String(tmp, Py_file_input, main_dict, main_dict);

	if (pstr == NULL)
		PyErr_Print();
	else
	{
		if (execute) 
			sprintf (tmp, "script ran: %s\n", TimeStr()); 
		else 
			sprintf (tmp, "script validated: %s\n",  TimeStr());
	
		feedback_show("GCI Python", tmp);  
	}
	
	return;	
}

static int CVICALLBACK cbRunPython (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			RunPython(callbackData, 1);
		}
	}
	
	return 0;
}

static int CVICALLBACK cbValidatePython (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			RunPython(callbackData, 0);
		}
	}
	
	return 0;
}

static int CVICALLBACK cbExePythonLine (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			char buffer[256];
			
			PyObject *main_module, *main_dict;
	
			GetCtrlVal(panel, EDIT_PANEL_EXELINE, buffer); 
	
			// Get a reference to the main module.
			main_module = PyImport_AddModule("__main__");  
	
			// Get the main module's dictionary
			// and make a copy of it.
			main_dict = PyModule_GetDict(main_module);
	
			PyRun_SimpleString("from UIControl import UIController, Panel");      

			PyRun_SimpleString("ui = UIController(debug=0)");      

			PyRun_SimpleString(buffer);      
		}
	}
	return 0;
}


void AutomationEditor_OpenFile(AutomationEditor *editor, const char *filepath)
{
	FILE *fp;
	
	BlankEditor(editor);  
	SendMessage(editor->hwndScintilla, SCI_CANCEL, 0, 0); 
	SendMessage(editor->hwndScintilla, SCI_SETUNDOCOLLECTION, 0, 0); 

	if(CopyFileA(filepath, editor->tmp_filepath, 0) < 0) {
		ui_module_send_error(UIMODULE_CAST(editor), "Automation Error", "%s", "Failed to copy file");
		return;	
	}
	
	fp = fopen(editor->tmp_filepath, "rb");
	
	if (fp) {
		
		//SetTitle();
		char data[blockSize];
		int lenFile = fread(data, 1, sizeof(data), fp);
		
		while (lenFile > 0) {
	
			SendMessage(editor->hwndScintilla, SCI_ADDTEXT, lenFile, (LPARAM)data);    
			
			lenFile = fread(data, 1, sizeof(data), fp);
		}
		
	} else {
		
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		strcat(msg, editor->tmp_filepath);
		strcat(msg, "\".");

		GCI_MessagePopup("Error", msg);
	}
	
	fclose(fp);     
	
	SendMessage(editor->hwndScintilla, SCI_SETUNDOCOLLECTION, 1, 0);   
	SendMessage(editor->hwndScintilla, EM_EMPTYUNDOBUFFER, 0, 0);   
	SendMessage(editor->hwndScintilla, SCI_SETSAVEPOINT, 0, 0);   
	SendMessage(editor->hwndScintilla, SCI_GOTOPOS, 0, 0); 
}


void AutomationEditor_New(AutomationEditor *editor)
{
	BlankEditor(editor); 
	
	SendMessage(editor->hwndScintilla, SCI_ADDTEXT, strlen(example), (LPARAM)example);        
}


int CVICALLBACK OnClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			AutomationEditor_Destroy();
				
			break;
	}
	return 0;	
}

/// Sets a Scintilla style
void SetAStyle(AutomationEditor *editor, int style, COLORREF fore)
{	
	SendMessage(editor->hwndScintilla, SCI_STYLESETFORE, style, fore);
}
	

static void CreateEditorWindow(AutomationEditor *editor)
{
	int i, style_bits_needed;

	editor->hwndScintilla = CreateWindowEx(0,
				"Scintilla","", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
				10,10,500,400, editor->window_hwnd, NULL, editor->hInstance,NULL);

	SendMessage(editor->hwndScintilla, SCI_SETLEXER, SCLEX_PYTHON, 0); 
	  
	style_bits_needed = SendMessage(editor->hwndScintilla, SCI_GETSTYLEBITSNEEDED, 0, 0);
	  
	SendMessage(editor->hwndScintilla, SCI_SETSTYLEBITS, style_bits_needed, 0);

	SendMessage(editor->hwndScintilla, SCI_SETTABWIDTH, 4, 0);      
	  
	SendMessage(editor->hwndScintilla, SCI_SETKEYWORDS, 0, (LPARAM) KeyWords );
	  
	SendMessage(editor->hwndScintilla, SCI_STYLECLEARALL, 0, 0L );    
	  
	AutomationEditor_New(editor);
	
	// Set syntax colors
    for (i = 0; g_rgbSyntaxCpp[ i ].iItem != -1; i++ ) 
        SetAStyle(editor, g_rgbSyntaxCpp[ i ].iItem, g_rgbSyntaxCpp[ i ].rgb );	
}
	

void CVICALLBACK OnSaveScriptMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char filename[GCI_MAX_PATHNAME_LEN];
	
	AutomationEditor *editor = (AutomationEditor *) callbackData;	

	if (FileSelectPopup (editor->script_directory, "*.py", "",
					 "Save Python Script", VAL_SAVE_BUTTON, 0, 1, 1, 1,
					 filename) <= 0 ) {
		return;
	}
	
	SaveFile(editor, filename);
	
	*strrchr(filename, '\\') = 0;
	strcpy(editor->script_directory, filename);
}

AutomationEditor* Get_AutomationEditor(HINSTANCE hInstance)
{
	HMODULE hmod;
	int window_handle;

	if(editor != NULL)
		return editor;
		
	editor = (AutomationEditor*) malloc(sizeof(AutomationEditor));      
	
	if(hInstance == NULL)
		return NULL;
	
	editor->hInstance = hInstance;
	
	feedback_new();  
	
	#ifndef MICROSCOPE_PYTHON_AUTOMATION
	
	Py_Initialize();

	Py_InitModule("gci", Gci_Py_Methods);
	
	//Lets append to the python module path  
	//PyRun_SimpleString("sys.path.append(\".")");  

	// Use this as the test if Python is installed
	if (PyRun_SimpleString("import StdOutRedirect")<0)
	{
		free (editor);
		editor = NULL;
		feedback_destroy();
		Py_Finalize();
		return NULL;
	}
	#endif

	// Get temp dir
	if(!GetEnvironmentVariable("Temp", editor->tmp_filepath, 500))
		ui_module_send_error(UIMODULE_CAST(editor), "Automation Error", "%s", "Can not create temporary directory");  

	strcat(editor->tmp_filepath, "\\automation_temp.txt");	
	
	// Make scripts dir
	// Make script Directory
	#ifdef MICROSCOPE_PYTHON_AUTOMATION  
	microscope_get_data_subdirectory(microscope_get_microscope(), "Automation Scripts", editor->script_directory);      
	#else
	GetProjectDir(editor->script_directory);
	#endif
	
	if (!FileExists(editor->script_directory, 0))
		MakeDir (editor->script_directory);   
	
	ui_module_constructor(UIMODULE_CAST(editor), "AutomationEditor");
	
	editor->panel_id = ui_module_add_panel(UIMODULE_CAST(editor), "PythonEditor.uir", EDIT_PANEL, 1);
	
	editor->old_wndproc = ui_module_set_window_proc(UIMODULE_CAST(editor), editor->panel_id, (LONG_PTR) EditorWndProc);     

	GetPanelAttribute (editor->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle); 

	editor->window_hwnd = (HWND) window_handle;

	hmod = LoadLibrary("SciLexer.DLL"); 
	
	if (hmod==NULL)
	{
		MessageBox(editor->window_hwnd,
			"The Scintilla DLL could not be loaded.\nSciLexer.dll",
			"Error loading Scintilla",
			MB_OK | MB_ICONERROR);
	}
	
	CreateEditorWindow(editor);
	
	InstallCtrlCallback(editor->panel_id, EDIT_PANEL_RUN_BUTTON, cbRunPython, editor);

	InstallCtrlCallback(editor->panel_id, EDIT_PANEL_RUN_BUTTON_2, cbExePythonLine, editor);

	InstallCtrlCallback(editor->panel_id, EDIT_PANEL_VALIDATE_BUTTON, cbValidatePython, editor);
	
	InstallCtrlCallback(editor->panel_id, EDIT_PANEL_CLOSE_BUTTON, OnClose, editor);

	InstallMenuCallback(GetPanelMenuBar(editor->panel_id), MENUBAR_FILE_NEW, OnNewScriptMenuClicked, editor);     
	
	InstallMenuCallback(GetPanelMenuBar(editor->panel_id), MENUBAR_FILE_OPEN, OnOpenScriptMenuClicked, editor);     
	
	InstallMenuCallback(GetPanelMenuBar(editor->panel_id), MENUBAR_FILE_SAVE, OnSaveScriptMenuClicked, editor);
	
  	return editor;
}


void AutomationEditor_Display(AutomationEditor *editor)  
{
	ui_module_display_panel(UIMODULE_CAST(editor), editor->panel_id);   
}

void AutomationEditor_Destroy(void)
{
	if (editor==NULL) return;
		
	ui_module_destroy(UIMODULE_CAST(editor));  

	#ifndef MICROSCOPE_PYTHON_AUTOMATION
	Py_Finalize();
	feedback_destroy(); 
	#endif

	free(editor);
	
	editor = NULL;
}

void CVICALLBACK OnOpenScriptMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char fname[GCI_MAX_PATHNAME_LEN]; 
	AutomationEditor *editor = (AutomationEditor *) callbackData;
	
	if (FileSelectPopup (editor->script_directory, "*.py", "",
								 "Load Python Script", VAL_LOAD_BUTTON, 1, 1, 1, 0, fname ) != 1) {
								 
		return;
	}
	
	AutomationEditor_OpenFile(editor, fname);   
	
	*strrchr(fname, '\\') = 0;
	strcpy(editor->script_directory, fname);
}

void CVICALLBACK OnNewScriptMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	AutomationEditor *editor = (AutomationEditor *) callbackData;
	
	AutomationEditor_New (editor);
}
