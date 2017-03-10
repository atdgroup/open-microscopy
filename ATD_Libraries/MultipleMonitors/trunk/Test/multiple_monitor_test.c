
#include <windows.h>
#include <multimon.h>

#include "multiple_monitor_test2.h"

#include <utility.h>
#include <userint.h>



static int first_panel, second_panel;
static POINT p1;
static HMONITOR monitor;

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)  	
{
	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;	// out of memory        

	first_panel = NewPanel (0, "Panel 1", 200, 200, 200, 200); 	
	second_panel = NewPanel (0, "Panel 1", 200, 400, 200, 200);
	
	DisplayPanel(first_panel);
	DisplayPanel(second_panel);
	
	MoveWindowToOtherWindow (first_panel, second_panel, 0, 0,
							 MONITOR_CENTER | MONITOR_WORKAREA);
							 
	p1.x = 200;
	p1.y = 400;
	
	monitor = GetMonitorForPoint( p1 ); 
	
							 
	Test();
							 
	RunUserInterface(); 
	
	return 0;
}
