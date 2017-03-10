#ifndef __AUTOMATION_EDITOR__
#define __AUTOMATION_EDITOR__

#include "gci_ui_module.h"

#include <userint.h>
#include <windows.h> 

#include <richedit.h>

typedef struct
{
	UIModule parent;   
	LONG_PTR old_wndproc;
	HWND window_hwnd;

	char tmp_filepath[GCI_MAX_PATHNAME_LEN];
	char script_directory[500];
	
	HINSTANCE 	hInstance;
	HWND 		hwndScintilla; 
	int 		panel_id;
	
} AutomationEditor;


AutomationEditor* Get_AutomationEditor(HINSTANCE hInstance); 

void SaveFile(AutomationEditor *editor, const char *filepath);

void AutomationEditor_OpenFile(AutomationEditor *editor, const char *fileName);

void AutomationEditor_New(AutomationEditor *editor);

void AutomationEditor_SendOutput(const char *output_str);

void AutomationEditor_Display(AutomationEditor *editor);

void AutomationEditor_Destroy(void);
		 
void CVICALLBACK OnNewScriptMenuClicked(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnOpenScriptMenuClicked(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OnSaveScriptMenuClicked(int menubar, int menuItem, void *callbackData, int panel);

#endif
