#ifndef __GCI_WINDOWMENU
#define __GCI_WINDOWMENU
int setupWindowMenu (int mbh, int mid);
int WindowMenuDisplayPanel (int panelHandle);
int WindowMenuHidePanel (int panelHandle);
int WindowMenuDiscardPanel (int panelHandle);
int WindowMenuAddPanel (int panelHandle);
int WindowMenuRemovePanel (int panelHandle);
int WindowMenuClear (void);
int WindowMenuChangePanelTitle (int ph, char* title);
#endif
