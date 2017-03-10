#ifndef _REGION_SCAN__
#define _REGION_SCAN__

int RegionScan_signal_hide_handler_connect (void (*handler) (void*), void *callback_data);

int GCI_RegionScan_Init(void);
int GCI_RegionScan_Close(void);
int GCI_RegionScan_DisplayPanel(void);
int GCI_RegionScan_HidePanel(void);
int GCI_RegionScanPanelVisible(void);
int GCI_RegionScan_DisablePanel(int disable);

#endif

 
