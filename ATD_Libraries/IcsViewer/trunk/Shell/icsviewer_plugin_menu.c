#include "toolbox.h"
#include "icsviewer_plugin_menu.h"
#include "icsviewer_window.h"
#include <utility.h>

#include "gci_menu_utils.h"

int Plugin_AddMenuItem(ImageWindowPlugin *plugin, char *path, 
	int shortCutKey, MenuCallbackPtr eventFunction, void *callback_data)
{
	int menu_bar_id, menu_item_id;
	
	menu_bar_id = MENUBAR_ID(plugin);
	
	menu_item_id = CreateMenuItemPath(menu_bar_id, path, -1, shortCutKey, eventFunction, callback_data);    
	
	ListInsertItem(plugin->menuItemList, &menu_item_id, END_OF_LIST);      
	
	SetMenuBarAttribute (menu_bar_id, menu_item_id, ATTR_DIMMED, 1); 
	SetMenuBarAttribute (menu_bar_id, menu_item_id, ATTR_CHECKED, 0);
	
	return menu_item_id;
}


void Plugin_DimMenuPathItem(ImageWindowPlugin *plugin, char *path)
{
	if(plugin == NULL)
		return;

	DimMenuPathItem(MENUBAR_ID(plugin), path);    
}


void Plugin_UnDimMenuPathItem(ImageWindowPlugin *plugin, char *path)
{
	if(plugin == NULL)
		return;

	UnDimMenuPathItem(MENUBAR_ID(plugin), path);   
}


void Plugin_CheckMenuPathItem(ImageWindowPlugin *plugin, char *path)
{
	if(plugin == NULL)
		return;

	CheckMenuPathItem(MENUBAR_ID(plugin), path); 
}


void Plugin_UnCheckMenuPathItem(ImageWindowPlugin *plugin, char *path)
{
	if(plugin == NULL)
		return;

	UnCheckMenuPathItem(MENUBAR_ID(plugin), path);   
}


int Plugin_IsMenuPathItemChecked(ImageWindowPlugin *plugin, char *path)
{
	if(plugin == NULL)
		return 0;

	return IsMenuPathItemChecked(MENUBAR_ID(plugin), path); 
}
