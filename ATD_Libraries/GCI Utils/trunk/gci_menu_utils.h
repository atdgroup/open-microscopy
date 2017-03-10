#ifndef __GCI_MENU_UTILS__
#define __GCI_MENU_UTILS__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

int FindMenuItemIdFromNameInMenu(int menubar_id, int menu_id, char *name, int recurse);

int CreateMenuItemPath(int menubar_id, char *path, int beforeMenuItemID,
	int shortCutKey, MenuCallbackPtr eventFunction, void *callbackData);

int GetLastItemIdFromPath(int menubar_id, char *path, int *id);

void DimMenuPathItem(int menubar_id, char *path);

void UnDimMenuPathItem(int menubar_id, char *path);

void CheckMenuPathItem(int menubar_id, char *path);

void CheckOrUncheckMenuPathItem(int menubar_id, char *path, int checked);

void UnCheckMenuPathItem(int menubar_id, char *path); 

int IsMenuItemChecked(int menubar_id, int menu_item);

int IsMenuPathItemChecked(int menubar_id, char *path);

#endif
