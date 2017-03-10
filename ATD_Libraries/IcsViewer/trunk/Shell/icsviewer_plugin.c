#include "icsviewer_window.h"
#include "toolbox.h"
#include "icsviewer_plugin.h" 
#include "icsviewer_plugin_menu.h"

#include "gci_utils.h"

#include "FreeImageAlgorithms_Utilities.h"

EventData NewEventData(void)
{
	EventData data;
	memset(&data, 0, sizeof(EventData));
	
	return data;
}


ImageWindowPlugin* Plugin_CreatePlugin(IMAGEWINDOW_PLUGIN_CONSTRUCTOR constructor, IcsViewerWindow *window)
{
	ImageWindowPlugin *plugin = (*constructor)(window);

	ListInsertItem (window->program_plugins, &plugin, END_OF_LIST); 

	return plugin; 
}


ImageWindowPlugin* GetPluginPtrInList(IcsViewerWindow *window, int i)
{
	return *((ImageWindowPlugin **) (ListGetPtrToItem (window->program_plugins, i)));
}


void UnDimMenuItems(ImageWindowPlugin *plugin)
{
	int i, size;
	int *menu_item;
	
	if((size = ListNumItems (plugin->menuItemList)) < 1)
		return;
	
	if (size > 100) return; //RJL crashed on getting a huge number here 200906
	
	for(i=1; i <= size; i++) {
	
		menu_item = ListGetPtrToItem (plugin->menuItemList, i);

		SetMenuBarAttribute (MENUBAR_ID(plugin), *menu_item, ATTR_DIMMED, 0); 
	} 
}

			
void DimAndUncheckMenuItems(ImageWindowPlugin *plugin)
{
	int i, size;
	int *menu_item;
	
	if((size = ListNumItems (plugin->menuItemList)) < 1)
		return;
	
	for(i=1; i <= size; i++) {
	
		menu_item = ListGetPtrToItem (plugin->menuItemList, i);

		SetMenuBarAttribute (MENUBAR_ID(plugin), *menu_item, ATTR_DIMMED, 1);
		SetMenuBarAttribute (MENUBAR_ID(plugin), *menu_item, ATTR_CHECKED, 0);
	}
}


void UncheckPluginMenuItems(ImageWindowPlugin *plugin)
{
	int i, size;
	int *menu_item;
	
	if(plugin == NULL)
		return;

	if((size = ListNumItems (plugin->menuItemList)) < 1)
		return;
	
	for(i=1; i <= size; i++) {
	
		menu_item = ListGetPtrToItem (plugin->menuItemList, i);

		SetMenuBarAttribute (MENUBAR_ID(plugin), *menu_item, ATTR_CHECKED, 0); 
	}
}


void DestroyPlugin(ImageWindowPlugin* plugin)
{
	printf("Destroying Plugin %s\n", plugin->name);

	ListDispose(plugin->menuItemList);
	plugin->menuItemList =  0;

	free(plugin->vtable);
	plugin->vtable = NULL;

	free(plugin);
	plugin = NULL;
}

ImageWindowPlugin* Plugin_NewPluginType(IcsViewerWindow *window, char *name, size_t size)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) malloc (size);
	
	memset(plugin, 0, sizeof(ImageWindowPlugin));	// Set plugin memebers entries to NULL   
	
	plugin->vtable = (PluginOperations*) malloc(sizeof(PluginOperations));
	memset(plugin->vtable, 0, sizeof(PluginOperations)); // Set vtable entries to NULL   

	plugin->window = window;
	strcpy(plugin->name, name);
	plugin->type = PLUGIN;
	
	plugin->menuItemList = ListCreate (sizeof(int));
	
	return plugin;
}


int Is16BitImage(FIBITMAP* dib)
{
	if(dib == NULL)
		return 0;
		
	if(FreeImage_GetBPP(dib) == 16)
		return 1;
		
	return 0;
}

int IsFloatingPointImage(FIBITMAP* dib)
{
	int type =  FreeImage_GetImageType(dib);
	
	if(dib == NULL)
		return 0;
		
	if(type == FIT_FLOAT && type == FIT_DOUBLE)
		return 1;
		
	return 0;
}
 

int IsStandardTypeImage(FIBITMAP* dib)
{
	int type =  FreeImage_GetImageType(dib); 
	
	if(type == FIT_BITMAP)
		return 1;
	
	return 0;
}

int Plugin_IsImageGreyScale(ImageWindowPlugin *plugin) 
{
	if(plugin->window->panel_dib != NULL && FIA_IsGreyScale(plugin->window->panel_dib))
		return 1;
		
	return 0;
}


void DisableAllPlugins(IcsViewerWindow *window)
{
	int i;
	ImageWindowPlugin *plugin = NULL; 
	
	for(i=1; i <= window->number_of_plugins; i++) { 
	
		plugin = GetPluginPtrInList(window, i); 
		
		Plugin_DimAndUncheckMenuItems (plugin);	
	} 
}

void DestroyAllPlugins(IcsViewerWindow *window)
{
	int i;
	ImageWindowPlugin *plugin = NULL; 
	
	EventData data1 = NewEventData();     
	EventData data2 = NewEventData(); 
	
	SEND_EVENT(window, on_destroy, data1, data2, "on_destroy");    

	for(i=1; i <= window->number_of_plugins; i++) { 
	
		plugin = GetPluginPtrInList(window, i); 
		
		DestroyPlugin(plugin);
	} 
}
