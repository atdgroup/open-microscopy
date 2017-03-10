#include "spc.h"
#include "spc_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "file_prefix_dialog.h"

#include <userint.h>
#include <utility.h>

static int CVICALLBACK OnStageScanSetROI (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Spc *spc = (Spc *) callbackData; 

			region_of_interest_panel_display(spc->ss_acquire.roi, UIMODULE_MAIN_PANEL_ID(spc));
            
            break;
		}
	}
    
	return 0;
}


static void OnCustomRegionOfInterestSelected (double left, double top, double width, double height, void *data)
{
	Spc *spc = (Spc *) data;

	spc->ss_acquire.frame_roi_left   = left;
	spc->ss_acquire.frame_roi_top    = top;
	spc->ss_acquire.frame_roi_width  = width;
	spc->ss_acquire.frame_roi_height = height;
	spc->ss_acquire.roi_set = 1;      
}

int spc_setup_stagescan(Spc *spc)
{
    spc->ss_acquire.roi = microscope_get_region_of_interest(spc->_ms);
    region_of_interest_selected_handler(spc->ss_acquire.roi, OnCustomRegionOfInterestSelected, spc) ;
    region_of_interest_panel_init(spc->ss_acquire.roi);
    
    return SPC_SUCCESS;
}