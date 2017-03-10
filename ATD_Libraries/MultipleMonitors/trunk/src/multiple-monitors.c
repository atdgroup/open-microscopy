#include "multiple-monitors.h"

#include <windows.h>
#include <multimon.h>
#include <userint.h>

#include "assert.h"   

#define GCI_MONITOR_ERROR -1
#define GCI_MONITOR_SUCCESS 0

static int IsMonitor( const HMONITOR monitor )
{
	if ( monitor == NULL )
		return GCI_MONITOR_ERROR;

	return GCI_MONITOR_SUCCESS;
}

static void GetMonitorRect( HMONITOR monitor, LPRECT lprc )
{
	MONITORINFO mi;
    RECT        rc;

	assert(IsMonitor(monitor) == GCI_MONITOR_SUCCESS);

	mi.cbSize = sizeof( mi );
	
	GetMonitorInfo( monitor, &mi );
	
	rc = mi.rcMonitor;

	SetRect( lprc, rc.left, rc.top, rc.right, rc.bottom );
}


static int IsRectOnMonitor( HMONITOR monitor, const LPRECT lprc )
{
	RECT rect;

	if( IsMonitor(monitor) == GCI_MONITOR_ERROR )
		return GCI_MONITOR_ERROR;
	
	GetMonitorRect( monitor, &rect );

	return IntersectRect( &rect, &rect, lprc );
}


static int IsPointOnMonitor( HMONITOR monitor, const POINT pt )
{
	RECT rect;

	if( IsMonitor(monitor) == GCI_MONITOR_ERROR)
		return GCI_MONITOR_ERROR;
	
	GetMonitorRect( monitor, &rect );

	return PtInRect( &rect, pt );
}


static HMONITOR GetMonitorForPoint( const POINT pt )
{
	HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);

	if(monitor == NULL)
		return NULL;

	return monitor;
}


static HMONITOR GetMonitorForWindow( HWND hwnd )
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);

	if(monitor == NULL)
		return NULL;

	return monitor;
}


static HMONITOR GetMonitorForLabWindow( int panel_id )
{
	int panel_handle;
	
	GetPanelAttribute (panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &panel_handle);

	return GetMonitorForWindow( (HWND) panel_handle );
}

                            

//
//  ClipOrCenterRectToMonitor
//
//  The most common problem apps have when running on a
//  multimonitor system is that they "clip" or "pin" windows
//  based on the SM_CXSCREEN and SM_CYSCREEN system metrics.
//  Because of app compatibility reasons these system metrics
//  return the size of the primary monitor.
//
//  This shows how you use the new Win32 multimonitor APIs
//  to do the same thing.
//
static void ClipOrCenterRectToMonitor(HMONITOR hMonitor, LPRECT prc, int x, int y, UINT flags)
{
    MONITORINFO mi;
    RECT        rc;
    int         w = prc->right  - prc->left;
    int         h = prc->bottom - prc->top;

    //
    // get the work area or entire monitor rect.
    //
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    if (flags & MONITOR_WORKAREA)
        rc = mi.rcWork;
    else
        rc = mi.rcMonitor;

    //
    // center or clip the passed rect to the monitor rect
    //
    if (flags & MONITOR_CENTER)
    {
        prc->left   = rc.left + (rc.right  - rc.left - w) / 2;
        prc->top    = rc.top  + (rc.bottom - rc.top  - h) / 2;
        prc->right  = prc->left + w;
        prc->bottom = prc->top  + h;
    }
    else if(flags & MONITOR_POSITION)
    {
    	prc->left   = max(rc.left, min(rc.right-w,  x + rc.left));
        prc->top    = max(rc.top,  min(rc.bottom-h, y + rc.top));
        prc->right  = prc->left + w;
        prc->bottom = prc->top  + h;
    }
    else
    {
        prc->left   = max(rc.left, min(rc.right-w,  prc->left));
        prc->top    = max(rc.top,  min(rc.bottom-h, prc->top));
        prc->right  = prc->left + w;
        prc->bottom = prc->top  + h;
    }
}

static void MoveWindowToMonitor(HMONITOR hMonitor, HWND hwnd, int x, int y, unsigned int flags)
{
    RECT rc;
    GetWindowRect(hwnd, &rc);
    
    ClipOrCenterRectToMonitor(hMonitor, &rc, x, y, flags);
    
    SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static void MoveHwndWindowToOtherWindowsMonitor(HWND hwnd1, HWND hwnd2, int x, int y, UINT flags)
{
    RECT rc;
    HMONITOR monitor;

    monitor = GetMonitorForWindow(hwnd1);

	if(monitor == NULL)
		return;

    GetWindowRect(hwnd2, &rc);
    ClipOrCenterRectToMonitor(monitor, &rc, x, y, flags);
    SetWindowPos(hwnd2, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

int GetMonitorCount(void)
{ 
	return GetSystemMetrics(SM_CMONITORS);
}

void CenterWindowOnOtherWindowsMonitor(int panel1_id, int panel2_id)
{
	MoveWindowToOtherWindowMonitor(panel1_id, panel2_id, 0, 0, MONITOR_CENTER);
}

void MoveWindowToOtherWindowMonitor(int first_panel_id, int second_panel_id, int x, int y, unsigned int flags)
{
	int first_window_handle, second_window_handle;
	
	GetPanelAttribute (first_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &first_window_handle);
	GetPanelAttribute (second_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &second_window_handle);

    MoveHwndWindowToOtherWindowsMonitor( (HWND) first_window_handle, (HWND) second_window_handle, x, y, flags);

	return;
}

