#include <userint.h>
#include <ansi_c.h>
#include "gci_menu_utils.h"

int FindMenuItemIdFromNameInMenu(int menubar_id, int menu_id, char *name, int recurse)
{
	int id, menuitem_id, submenu_id, number_of_menu_items;  
	char temp_name[50]; 
	
	GetMenuBarAttribute (menubar_id, menu_id, ATTR_NUM_MENU_ITEMS, &number_of_menu_items);
	
	// Check Menu Items
	GetMenuBarAttribute (menubar_id, menu_id, ATTR_FIRST_ITEM_ID, &menuitem_id); 
	
	// If the item passed is the man menu just return the id.
	GetMenuBarAttribute (menubar_id, menu_id, ATTR_MENU_NAME , temp_name);  
	
	if(strcmp(temp_name, name) == 0)
		return menu_id;
	
	while(menuitem_id > 0) {
		
		GetMenuBarAttribute (menubar_id, menuitem_id, ATTR_ITEM_NAME , temp_name);
		
		if(strcmp(temp_name, name) == 0)
			return menuitem_id;
	
		if(recurse) {
		
			GetMenuBarAttribute (menubar_id, menuitem_id, ATTR_SUBMENU_ID, &submenu_id);  	
		
			if(submenu_id) {
				if((id = FindMenuItemIdFromNameInMenu(menubar_id, submenu_id, name, recurse)) > 0)
					return id;
			}
		}
		
		GetMenuBarAttribute (menubar_id, menuitem_id, ATTR_NEXT_ITEM_ID, &menuitem_id);
	}
	
	return -1;
}


int GetIdForTopLevelMenu(int menubar_id, char *name) 
{
	int menu_id;
	char temp_name[30];
	
	// Plugins add a category or menu on the menubar
	// We need to check if this menu exist before add the plugin menu ITEM to it.
	GetMenuBarAttribute (menubar_id, 0, ATTR_FIRST_MENU_ID, &menu_id);
	
	while(menu_id > 0)
	{
		GetMenuBarAttribute (menubar_id, menu_id, ATTR_MENU_NAME, temp_name);
		
		// Menu exists.
		if(strcmp(temp_name, name) == 0)
			return menu_id;
	
		GetMenuBarAttribute (menubar_id, menu_id, ATTR_NEXT_MENU_ID, &menu_id); 
	}

	return -1;
}


static void GetMenuPathFirstandLastEntry(char *path, char *first_entry, char *last_entry)
{
	char *p;
	char str[500];
	
	strcpy(str, path);

	p = strtok (str, "/");
  
  	strcpy(first_entry, p);      
  
	while (p != NULL) {
		strcpy(last_entry, p); 
		p = strtok (NULL, "//");
	}
}


int CreateMenuItemPath(int menubar_id, char *path, int beforeMenuItemID,
	int shortCutKey, MenuCallbackPtr eventFunction, void *callbackData)
{
	char *p;
	char str[500];
	char menu[500], menuitem[500];
	int first_menu_id = -1, submenu_id = -1, last_submenu_id = -1, id = -1;
	
	// Get the first toplevel menu and the last (menuitem)
	// Everything in between is a submenu
	GetMenuPathFirstandLastEntry(path, menu, menuitem);              
	
	strcpy(str, path);
    
  	if((first_menu_id = GetIdForTopLevelMenu(menubar_id, menu)) < 0)
		first_menu_id = NewMenu (menubar_id, menu, -1);
  
  	last_submenu_id = first_menu_id;
  
  	p = strtok (str, "//");
    p = strtok (NULL, "//"); 
  
	while (p != NULL && (strcmp(menuitem, p) != 0) )
	{
		if((submenu_id = FindMenuItemIdFromNameInMenu(menubar_id, last_submenu_id, p, 0)) < 0) {     
	
			submenu_id = NewMenuItem (menubar_id, last_submenu_id,
				p, beforeMenuItemID, 0, NULL, NULL);
				
			last_submenu_id = NewSubMenu(menubar_id, submenu_id);  
		}
		else {
		
			GetMenuBarAttribute (menubar_id, submenu_id, ATTR_SUBMENU_ID, &last_submenu_id);  
		}
	
		p = strtok (NULL, "//");
	}
	
	
	// If the menuitem already exists return the existing id
	if((id = FindMenuItemIdFromNameInMenu(menubar_id, last_submenu_id, menuitem, 0)) > 0)
		return id;
	
	id = NewMenuItem (menubar_id, last_submenu_id,
					menuitem, beforeMenuItemID, shortCutKey, eventFunction, callbackData);
	
	return id;
}


int GetLastItemIdFromPath(int menubar_id, char *path, int *id)
{
	int menu_id;
	char str[500], first_entry[500], last_entry[500];

	strcpy(str, path);

	GetMenuPathFirstandLastEntry(str, first_entry, last_entry);  
	
	menu_id = GetIdForTopLevelMenu(menubar_id, first_entry);
	
	if(menu_id < 0)
		return -1;
	
	*id = FindMenuItemIdFromNameInMenu(menubar_id, menu_id, last_entry, 1);
	
	if(*id < 0)
		return -2;
	
	return *id;
}


void UnDimMenuPathItem(int menubar_id, char *path)
{
	int id;
	
	GetLastItemIdFromPath(menubar_id, path, &id);   
	
	SetMenuBarAttribute (menubar_id, id, ATTR_DIMMED, 0);   		
}


void DimMenuPathItem(int menubar_id, char *path)
{
	int id;
	
	GetLastItemIdFromPath(menubar_id, path, &id);   
	
	SetMenuBarAttribute (menubar_id, id, ATTR_DIMMED, 1); 	
}


void CheckMenuPathItem(int menubar_id, char *path)
{
	int err, id;
	
	err = GetLastItemIdFromPath(menubar_id, path, &id);
	
	if(err < 0 )
		return;
	
	if(id > -1)  
		SetMenuBarAttribute (menubar_id, id, ATTR_CHECKED, 1);
	
}

void CheckOrUncheckMenuPathItem(int menubar_id, char *path, int checked)
{
	int err, id;
	
	err = GetLastItemIdFromPath(menubar_id, path, &id);
	
	if(err < 0 )
		return;
	
	if(id > -1)  
		SetMenuBarAttribute (menubar_id, id, ATTR_CHECKED, checked);
	
}

int IsMenuItemChecked(int menubar_id, int menu_item)
{
	int checked;
	
	GetMenuBarAttribute (menubar_id, menu_item, ATTR_CHECKED, &checked);
			
	if(checked)
		return 1;

	return 0;
}

int IsMenuPathItemChecked(int menubar_id, char *path)
{
	int id, checked;
	
	if(GetLastItemIdFromPath(menubar_id, path, &id) > 0 ) {
		if(id > -1) { 
			GetMenuBarAttribute (menubar_id, id, ATTR_CHECKED, &checked);
			
			if(checked)
				return 1;
		}
	}
	
	return 0;
}

void UnCheckMenuPathItem(int menubar_id, char *path)
{
	int id;
	
	if(GetLastItemIdFromPath(menubar_id, path, &id) > 0 ) {
		if(id > -1)
			SetMenuBarAttribute (menubar_id, id, ATTR_CHECKED, 0);
	}
}
