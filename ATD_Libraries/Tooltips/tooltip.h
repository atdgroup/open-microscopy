#ifndef __GCI_TOOLTIP__
#define __GCI_TOOLTIP_

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

HWND ToolTip_Create (int panel);

void ToolTip_AddTool(HWND tooltip, RECT rect, char *text);

void ToolTip_AddToolTipForCtrl(HWND tooltip, int panel, int ctrl, char *text);

#endif
