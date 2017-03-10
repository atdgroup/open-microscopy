// P. Barber, June 2003.
// code to keep a list of open windows in a menu called window, selecting an item 
// in this menu brings the panel to the front. To help identify multiple windows with the same name, the panel 
// handle is added to the window title, currently in [] brackets.

// Must call setupWindowMenu before anything else using a call like:
//		setupWindowMenu (GetPanelMenuBar(panelHandle), MENUBAR_WINDOW); from the main program.

// When using this menu, use  WindowMenuDisplayPanel() instead of DisplayPanel()
//                            WindowMenuHidePanel()    instead of HidePanel()
//                            WindowMenuDiscardPanel()    instead of DiscardPanel()
//                            WindowMenuChangePanelTitle()    instead of changing the panel title directly (ATTR_TITLE). in case the panel is visible and in the menu.
 
#include <ansi_c.h>
#include <formatio.h>
#include <userint.h>

static int internalMenuBarHandle=-1, internalMenuID=-1, internalSetup=0;

#define MBH internalMenuBarHandle   // menu bar handle of main panel
#define MID internalMenuID          // menu ID of target menu

//********** setupWindowMenu *************************************************************************

int setupWindowMenu (int mbh, int mid)
{
	internalMenuBarHandle = mbh;
	internalMenuID		  = mid;
	internalSetup = 1;
	return (0);
}

//********** findPanelHandleInString *************************************************************************

int findPanelHandleInString (char *panelName)
{
	char *phString, *phStringEnd;
	int ph, e;

	phString    = strrchr (panelName, '[');
	phStringEnd = strrchr (panelName, ']');
	if (phStringEnd == NULL) return(-1);

	phString++;
	e = Scan (phString, "%d", &ph);
	if (e==1) return(ph);
	else return(-1);
}

//********** mcbWindowToFront *************************************************************************

void CVICALLBACK mcbWindowToFront (int menuBar, int menuItem, void *callbackData, int panel)
{
	char miName[256];

	GetMenuBarAttribute (menuBar, menuItem, ATTR_ITEM_NAME, miName);
	SetActivePanel (findPanelHandleInString (miName));
}

//********** checkForItemInMenu *************************************************************************

int checkForItemInMenu (char *title)
{
	int i, n, miID, found=0, error;
	char miName[256];
	
	error = GetMenuBarAttribute (MBH, MID, ATTR_NUM_MENU_ITEMS, &n);
	if (error<0) return(-1);
	GetMenuBarAttribute (MBH, MID, ATTR_FIRST_ITEM_ID, &miID);

	for (i=0; i<n; i++)
	{
		GetMenuBarAttribute (MBH, miID, ATTR_ITEM_NAME, miName);
		if (CompareStrings (miName, 0, title, 0, 1)==0) {found=1; break;}
		GetMenuBarAttribute (MBH, miID, ATTR_NEXT_ITEM_ID, &miID);
	}

	if (found==1) return (miID);
	else return (0);
}

//********** WindowMenuAddPanel *************************************************************************

int WindowMenuAddPanel (int ph)
{
	char panelName[256];
	int mi;
	
	if (internalSetup==0) return(-1);
	if (ph<0) return(-1);
	
	GetPanelAttribute (ph, ATTR_TITLE, panelName);
	// look for panel handle on end of title, if not there add it.
	if (findPanelHandleInString (panelName)<0) 
	{
		sprintf (panelName, "%s     [%d]", panelName, ph);
		SetPanelAttribute (ph, ATTR_TITLE, panelName);
	}
	
	mi = checkForItemInMenu(panelName);
	
	if (mi==0)
	{
		mi = NewMenuItem (MBH, MID, panelName, -1, 0, 0, 0);
		SetMenuBarAttribute (MBH, mi, ATTR_CALLBACK_FUNCTION_POINTER, mcbWindowToFront);
	}

	if (mi>0) return(0);
	else return(mi);
}

//********** WindowMenuRemovePanel *************************************************************************

int WindowMenuRemovePanel (int ph)
{
	char panelName[256];
	int miID;

	if (internalSetup==0) return(-1);
	if (ph<0) return(-1);
	
	GetPanelAttribute (ph, ATTR_TITLE, panelName);
	
	miID = checkForItemInMenu (panelName);
	if (miID>0)
	{
		DiscardMenuItem (MBH, miID);
		return(1);
	}
	else return (miID);   // the error from checkForItemInMenu
}

//********** WindowMenuDisplayPanel *************************************************************************

int WindowMenuDisplayPanel (int ph)
{
	int error;
	
	error =                DisplayPanel       (ph);
	if (internalSetup==0) return(-1);
	if (error ==0) return( WindowMenuAddPanel (ph));
	else return (error);
}

//********** WindowMenuHidePanel *************************************************************************

int WindowMenuHidePanel (int ph)
{
	int error;
	
	error =                HidePanel       (ph);
	if (internalSetup==0) return(-1);
	if (error ==0) return( WindowMenuRemovePanel (ph));
	else return (error);
}

//********** WindowMenuDiscardPanel *************************************************************************

int WindowMenuDiscardPanel (int ph)
{
	WindowMenuRemovePanel (ph);
	return (DiscardPanel  (ph));
}

//********** WindowMenuClear *************************************************************************

int WindowMenuClear (void)
{
	if (internalSetup==0) return(-1);
	return(EmptyMenu (MBH, MID));
}


//********** WindowMenuChangePanelTitle *************************************************************************

int WindowMenuChangePanelTitle (int ph, char* title)
{
	int removed;
	
	removed = WindowMenuRemovePanel (ph);   // remove and re-add it since we are changing title
	if (removed<0) return (-1);
	SetPanelAttribute (ph, ATTR_TITLE, title);
	if (removed==1) WindowMenuAddPanel (ph);
	return(0);
}
