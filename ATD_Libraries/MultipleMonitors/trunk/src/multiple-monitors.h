#ifndef __MULTIPLE_MONITORS__
#define __MULTIPLE_MONITORS__

// Normally you just include "multimon.h" (like above)
// but one C file needs to define COMPILE_MULTIMON_STUBS
// so the compatibility stubs will be defined
#define COMPILE_MULTIMON_STUBS

#include <ansi_c.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MONITOR_CENTER   0x0001        // center rect to monitor
#define MONITOR_POSITION 0x0004        // position rect on monitor
#define MONITOR_CLIP     0x0000        // clip rect to monitor
#define MONITOR_WORKAREA 0x0002        // use monitor work area
#define MONITOR_AREA     0x0000        // use monitor entire areavoid

int GetMonitorCount(void);

void MoveWindowToOtherWindowMonitor(int panel1_id, int panel2_id, int x, int y, unsigned int flags);

void CenterWindowOnOtherWindowsMonitor(int panel1_id, int panel2_id);

#ifdef __cplusplus
}
#endif

#endif
