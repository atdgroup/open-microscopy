#ifndef __ICSVIEWER_PLUGIN_MENU__
#define __ICSVIEWER_PLUGIN_MENU__

#include "icsviewer_window.h"
#include "icsviewer_plugin.h"
#include "gci_menu_utils.h"

typedef struct _MenuItem MenuItem;

typedef void (*PluginMenuCallbackPtr)(ImageWindowPlugin *plugin, MenuItem *menuItem, int dimmed, int checked, void *callback_data);

/*
struct _MenuItem
{
	int menubar_id;
	int menu_id;
	int menu_item_id;
	
	ImageWindowPlugin *plugin;
	
	PluginMenuCallbackPtr eventFunction;
	void *callback_data;
};
*/

int  Plugin_AddMenuItem(ImageWindowPlugin *plugin, char *path,
	int shortCutKey, MenuCallbackPtr eventFunction, void *callback_data);


void Plugin_DimMenuPathItem(ImageWindowPlugin *plugin, char *path);

void Plugin_UnDimMenuPathItem(ImageWindowPlugin *plugin, char *path);

void Plugin_CheckMenuPathItem(ImageWindowPlugin *plugin, char *path);

void Plugin_UnCheckMenuPathItem(ImageWindowPlugin *plugin, char *path);

int Plugin_IsMenuPathItemChecked(ImageWindowPlugin *plugin, char *path);

#define IS_MENU_CHECKED IsMenuItemChecked(menubar, menuItem)

#define CHECK_MENU_ITEM SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 1);

#define UNCHECK_MENU_ITEM SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 0);  

#define DIM_MENU_ITEM SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, 1);  

#define UNDIM_MENU_ITEM SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, 0); 

/*
void Plugin_DimMenuItem(MenuItem *menuItem);

void Plugin_UnDimMenuItem(MenuItem *menuItem);

void Plugin_DimAndUnCheckMenuItem(MenuItem *menuItem);

void Plugin_CheckMenuItem(MenuItem *menuItem);

void Plugin_UnCheckMenuItem(MenuItem *menuItem);
*/



#endif
