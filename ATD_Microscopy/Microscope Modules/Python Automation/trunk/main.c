#include <userint.h>
#include "Test.h"
#include "AutomationEditor.h"
#include "status.h"
#include <ansi_c.h>
#include "test_python_wrappers.h" 

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	int test_panel_id, test_panel_id2;
	
	AutomationEditor* editor = Get_AutomationEditor(hInstance); 
	
	AutomationEditor_Display(editor);
	
	Py_InitModule("test", Test_Py_Methods);
	PyRun_SimpleString("import test");      
	
	AutomationEditor_OpenFile(editor, "Test.py");     
	
	test_panel_id = LoadPanel(0, "Test.uir", TEST_PNL);
	test_panel_id2 = LoadPanel(test_panel_id, "Test.uir", TEST_PNL2);  
	
	InstallCtrlCallback(test_panel_id, TEST_PNL_NUMERIC, OnNumericChanged, editor);     
	
	DisplayPanel(test_panel_id);
	DisplayPanel(test_panel_id2); 
	
	RunUserInterface();        
	
  	return 0;
}



int CVICALLBACK OnNumericChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	AutomationEditor *editor = (AutomationEditor *) callbackData;  
	char buffer[500];
	
	switch (event)
	{
		case EVENT_COMMIT:

			feedback_show("Python", "OnNumericChanged Called\n");
			
			break;
	}
	return 0;
}



int CVICALLBACK OnOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			feedback_show("Python", "OnOkClicked Called\n"); 
				
			break;
	}
	return 0;
}

int CVICALLBACK OnRingChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			feedback_show("Python", "OnRingChanged Called\n");
			
			break;
	}
	return 0;
}

int CVICALLBACK OnListChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			feedback_show("Python", "OnListChanged Called\n"); 
				
			break;
	}
	return 0;
}

void CVICALLBACK OnFileOpen (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	feedback_show("Python", "OnFileOpen Called\n");  
}

void CVICALLBACK OnViewClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	feedback_show("Python", "OnViewClicked Called\n");  
}

int CVICALLBACK OnToggleButtonChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			feedback_show("Python", "On Toggle button toggled\n");  
			
			break;
	}
	return 0;
}
