#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include "pwctrl.h"
#include "password_ui.h"
#include "gci_utils.h"

////////////////////////////////////////////////////////////////////////////
//Popup panel with password verification
//Module of GCI MultiPhotonTimeResolved system
//
//Rosalind Locke - January 2003
////////////////////////////////////////////////////////////////////////////

int GCI_ShowPasswordProtectedPanel(int panelHandle, int parentPanelHandle)
{
	int panelHandlePW, ctrl, pnl, top, left;
	char pword[20];
	char uir_file_path[GCI_MAX_PATHNAME_LEN];
	
	find_resource("password_ui.uir", uir_file_path); 
	
	if ((panelHandlePW = LoadPanel (0, uir_file_path, PANEL_PW)) < 0)
		return -1;
	
	PasswordCtrl_ConvertFromString (panelHandlePW, PANEL_PW_PWSTRING);

	// put near the parent panel 
	GetPanelAttribute (parentPanelHandle, ATTR_LEFT, &left);
	GetPanelAttribute (parentPanelHandle, ATTR_TOP, &top);
	SetPanelPos (panelHandlePW, top+10, left+10);
	
	InstallPopup (panelHandlePW);
	SetActiveCtrl (panelHandlePW, PANEL_PW_PWSTRING);
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl == panelHandlePW && ctrl == PANEL_PW_OK) {
			PasswordCtrl_GetAttribute (panelHandlePW, PANEL_PW_PWSTRING, ATTR_PASSWORD_VAL, pword);
			if (!strcmp( pword, "Feret")) {
				DisplayPanel(panelHandle);
				DiscardPanel(panelHandlePW);
				return 0;
			}
			DiscardPanel(panelHandlePW);
			break;
		}
	}
	return -1;
}

