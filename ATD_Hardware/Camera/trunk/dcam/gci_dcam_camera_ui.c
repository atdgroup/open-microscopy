#include <userint.h>
#include "gci_dcam_camera.h"
#include "gci_dcam_camera_ui.h"
#include "icsviewer_tools.h"

#include "ImageViewer.h"

static RECT RectToRECT(Rect rect)
{
	RECT rt;
	
	rt.left = rect.left;
	rt.top = rect.top;
	rt.right = rect.left + rect.width;
	rt.bottom = rect.top + rect.height;
	
	return rt;
}

static Rect RECTToRect(RECT rt)
{
	Rect rect;
	
	RectSet (&rect, rt.top, rt.left, rt.bottom - rt.top, rt.right - rt.left);

	return rect;
}

static SUBWINDOW_SIZE GetSelectedSubWindowSize(GciDCamCamera *dcam_camera)
{
	char val[30] = "";
	int ival = 0;
	GciCamera *camera = (GciCamera *) dcam_camera;

	GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, val);
	sscanf(val, "%d", &ival);

	switch(ival)
	{
		case -1:
		{	
			return SW_CUSTOM;
		}

		case 25:
		{	
			return SW_QUARTER_SQUARE;
		}
			
		case 50:
		{
			return SW_HALF_SQUARE;
		}

		case 75:
		{
			return SW_THREE_QUARTER_SQUARE;
		}

		case 100:
		{
			return SW_FULL_SQUARE;
		}

		case 200:
		{
			return SW_FULL_IMAGE;
		}
	}

	return -100;
}

static void GetWidthHeightFromSizePreset(GciDCamCamera *dcam_camera, SUBWINDOW_SIZE size, int *width, int *height)
{
	GciCamera *camera = (GciCamera *) dcam_camera;

	int w = camera->_max_width;
	int h = camera->_max_height;

	w = h = MIN(w, h);

	switch(size)
	{
		case SW_QUARTER_SQUARE:
		{
			w = (int) (w * 0.25);
			h = (int) (h * 0.25);
			break;
		}
			
		case SW_HALF_SQUARE:
		{
			w = (int) (w * 0.5);
			h = (int) (h * 0.5);
			break;
		}

		case SW_THREE_QUARTER_SQUARE:
		{
			w = (int) (w * 0.75);
			h = (int) (h * 0.75);
			break;
		}

		case SW_FULL_SQUARE:
		{
			break;
		}

		case SW_FULL_IMAGE:
		{
			w = camera->_max_width;
			h = camera->_max_height;
			break;
		}
	}

	*width = w;
	*height = h;
}

static void DCAM_DimSubWindowControls(GciDCamCamera *dcam_camera)
{
	int width, height, auto_centre;
	char val[30];
	SUBWINDOW_SIZE size;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, &auto_centre);  
	
	GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, val);
	sscanf(val, "%d %d", &width, &height);
	size = GetSelectedSubWindowSize(dcam_camera);		

	if(size != SW_CUSTOM) {// Pre determined size selected. Dim width and height
		
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SW_APPLY, ATTR_DIMMED, 1); 
		
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_DIMMED, 1);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, ATTR_DIMMED, 1);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, ATTR_DIMMED, 1);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, ATTR_DIMMED, 1);
		
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, ATTR_DIMMED, 0); 
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, 1);   
	}
	else {
		
		// Custom entry
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SW_APPLY, ATTR_DIMMED, 0); 
		
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_DIMMED, 0);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, ATTR_DIMMED, 0);	
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, ATTR_DIMMED, 0);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, ATTR_DIMMED, 0);
		
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, ATTR_DIMMED, 1);
	}
			
	if(auto_centre) {
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, 1); 
	}
	else {
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, 0); 
	}
	
}

int CVICALLBACK DCAM_Camera_onAutoCenter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return 0;
}

int CVICALLBACK DCAM_Camera_onSetSizePosition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, auto_centre, left, top, width, height;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int max_width, max_height;
			RECT rt;

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);
			
			gci_camera_get_max_size(camera, &max_width, &max_height);  
			
			SetCtrlIndex(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, 0);
			
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, &auto_centre); 
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH,  &width);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, &height);
			
			DCAM_DimSubWindowControls(dcam_camera);  
		
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, &left);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, &top);
		
			/*
			// Don't allow silly values
			if((left + width) >= camera->_max_width) {
				SetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_MAX_VALUE, camera->_max_width - left);
			};

			//if((top + height) >= camera->_max_height) {
				SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, camera->_max_height - top);
				SetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, ATTR_MAX_VALUE, camera->_max_height - top);
			//}
			*/

			gci_camera_set_size_position(camera, left, top, width, height, auto_centre);
		
			RectSet (&(camera->last_subwindow_rect), top, left, height, width);

			rt = RectToRECT(camera->last_subwindow_rect);
			GCI_ImagingWindow_EnableRoiTool(camera->_camera_window);
			GCI_ImagingWindow_SetROIImageRECT(camera->_camera_window, &rt);		
			GCI_ImagingWindow_DimRoiTool(camera->_camera_window, 1);

			break;
		}
	

	}
	return 0;
}
 

int gci_dcam_set_preset_subwindow(GciCamera *camera, SUBWINDOW_SIZE size)
{
	unsigned char ac;
	int max_width, max_height;
	int binning, binned_width, binned_height;
	int live, left=0, top=0, width, height;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) camera;
	RECT rt;

	gci_camera_get_max_size(camera, &max_width, &max_height);     

	binning = gci_camera_get_binning_mode(camera);

	binned_width = camera->_max_width / binning;
	binned_height = camera->_max_height / binning;

	live = gci_camera_is_live_mode(camera);

	if(live)
		gci_camera_set_snap_mode(camera);

	DCAM_DimSubWindowControls(dcam_camera);  

	if (size != SW_CUSTOM) {

		int l, t, w, h;

		GetWidthHeightFromSizePreset(dcam_camera, size, &width, &height);

		l = (max_width - width) / 2;
		t = (max_height - height) / 2;
		w = width;
		h = height;

		gci_camera_set_size_position(camera, l / binning, t / binning, width / binning, height / binning, 1);

		gci_camera_get_size_position(camera, &left, &top, &width, &height, &ac); 

//		GCI_ImagingWindow_DisableRoiTool(camera->_camera_window);      

		RectSet (&(camera->last_subwindow_rect), top, left, height, width);	 

		// This is relative to the total full image
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, l);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, t);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, w);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, h);
	}
	else {

		// Here we have a custom size

		if(RectEmpty(camera->last_subwindow_rect)) {

			// No previous subwindow set so we set the roi to half the max
			// And in center
			RectSet (&(camera->last_subwindow_rect), max_height / binning / 4, max_width / binning /4,
				max_height / binning / 2, max_width / binning / 2);
		}

//		GCI_ImagingWindow_DisableRoiTool(camera->_camera_window);  

		// Set the window to max values first
		gci_camera_set_size_position(camera, 0, 0, max_width, max_height, 0); 

		gci_camera_snap_image(camera);

		rt = RectToRECT(camera->last_subwindow_rect);
		GCI_ImagingWindow_EnableRoiTool(camera->_camera_window);
		GCI_ImagingWindow_SetROIImageRECT(camera->_camera_window, &rt); 		
//		GCI_ImagingWindow_DimRoiTool(camera->_camera_window, 1);

		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, camera->last_subwindow_rect.left);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, camera->last_subwindow_rect.top);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, camera->last_subwindow_rect.width);
		SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, camera->last_subwindow_rect.height);
	}

	if (live) {

		gci_camera_set_live_mode(camera);
		gci_camera_activate_live_display(camera);
	}
	else {

		gci_camera_snap_image(camera);
	}

	return CAMERA_SUCCESS;
}

int CVICALLBACK DCAM_Camera_onPresetSubWindow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
			GciCamera *camera = (GciCamera *) dcam_camera;

			SUBWINDOW_SIZE size = GetSelectedSubWindowSize(dcam_camera);		

			gci_camera_set_subwindow(camera, size);	

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK DCAM_onSubwindowApplyClicked    (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{																							   
		case EVENT_COMMIT:{
			
			int live, width, height;
			unsigned char auto_centre;
			GciCamera *camera = (GciCamera *) callbackData; 
			GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;                   
			RECT rt;
			Rect rect;

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			// Get the roi rect from the image window.
			GCI_ImagingWindow_GetROIImageRECT(camera->_camera_window, &rt);      	
			
			printf("GCI_ImagingWindow_GetROIImageRECT tlrb (%d, %d, %d, %d)\n", rt.left, rt.top, rt.right, rt.bottom);

			rt.left = MAX(rt.left, 0);
			rt.top = MAX(rt.top, 0);
			rt.right = MIN(rt.right, (LONG)camera->_max_width);
			rt.bottom = MIN(rt.bottom, (LONG)camera->_max_height);

			if(rt.left < 0 || rt.top < 0)
				return -1;

			if(rt.right <= rt.left || rt.bottom <= rt.top) {

				GCI_MessagePopup("Error", "Non valid rectangle");
				return -1;
			}

			rect = RECTToRect(rt);
			camera->last_subwindow_rect = rect;

			printf("gci_camera_set_size_position tlwh (%d, %d, %d, %d)\n", rect.left, rect.top, rect.width, rect.height);

			gci_camera_set_size_position(camera, rect.left, rect.top,
				rect.width, rect.height, 0);

			// You don't always get the size you asked for. So we get the actual roi set here.
			gci_camera_get_size_position(camera, &rect.left, &rect.top, &width, &height, &auto_centre);

			printf("gci_camera_get_size_position tlwh (%d, %d, %d, %d)\n\n", rect.left, rect.top, width, height);

//			GCI_ImagingWindow_DisableRoiTool(camera->_camera_window);                 
//			GCI_ImagingWindow_DimRoiTool(camera->_camera_window, 0);

			SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, width);
			SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, height);
				
			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}
			
			}break;
		}
	return 0;
}


int CVICALLBACK DCAM_Camera_onBinning (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera;
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
			// When binning changes don't save ast subwindow rect.
			// Things get too complcated.
//			GCI_ImagingWindow_DisableRoiTool(camera->_camera_window);  
			RectSet (&(camera->last_subwindow_rect), 0, 0, 0, 0);

    		if(live)
				gci_camera_set_snap_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BINNING, &(dcam_camera->_binning) );
			gci_camera_set_binning_mode(camera, dcam_camera->_binning);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}
 		

int CVICALLBACK DCAM_Camera_onDataMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live,data_mode;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData; 
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, &data_mode ); 
			camera->_data_mode = (DataMode) data_mode;
			gci_camera_set_data_mode(camera, camera->_data_mode);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}


int CVICALLBACK DCAM_Camera_onPropertyMenuClicked (int menubar, int menuItem, void *callbackData, int panel)
{
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData; 
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	if(dcam_camera_has_properties(dcam_camera))
		properties_dialog_show(dcam_camera);

	return 0;
}
		    
int CVICALLBACK DCAM_Camera_onBlackLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, &(dcam_camera->_black_level) );
			gci_camera_set_blacklevel(camera, CAMERA_ALL_CHANNELS, dcam_camera->_black_level);

			if (!live) gci_camera_snap_image(camera);

			break;
		}
	return 0;
}

static void enable_unsafe_sensitivity(GciDCamCamera *dcam_camera)
{
	GciCamera *camera = (GciCamera *) dcam_camera;

	// We can extend the range control to past the safe preset.
	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY_TITLE, "Sensitivity (Unsafe Mode)");
}

static void disable_unsafe_sensitivity(GciDCamCamera *dcam_camera)
{
	GciCamera *camera = (GciCamera *) dcam_camera;
				
//	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, 0.0);
	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY_TITLE, "Sensitivity");
}

int CVICALLBACK DCAM_Camera_onSensitivty (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, ctrl_pressed = 0;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	int pnl, xCoordinate, yCoordinate, leftButtonDown, rightButtonDown, keyModifiers;
			int is_ctrl_pressed = 0;

	GetGlobalMouseState (&pnl, &xCoordinate, &yCoordinate,
				&leftButtonDown, &rightButtonDown, &keyModifiers);

	ctrl_pressed = VAL_MENUKEY_MODIFIER & keyModifiers;

	switch (event)
	{
		case EVENT_LEFT_CLICK:
		{
			if(ctrl_pressed) {
				enable_unsafe_sensitivity(dcam_camera);
			}
			else {
				// The keyboard modifiewer is not present
				// we will not allow the 
				disable_unsafe_sensitivity(dcam_camera);
			}
		}
		break;

		case EVENT_COMMIT:
		{
			static 
			double gain = 0.0;

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, &(dcam_camera->_sensitivity) );

			if(ctrl_pressed)
			{
				// Ok we got the secret key press.
				double gain = 0.0;	
				
				// If the gain is above the preset we can now extend the sensitivity
				gci_camera_get_gain(camera, CAMERA_CHANNEL1, &gain);

				if(dcam_camera->_sensitivity > 0 && gain < (double) dcam_camera->_gain_safe_preset_for_sensitivity) {
					GCI_MessagePopup("Warning", "Increase the gain in preference to the sensitivity");
				}
			}
			else 
			{
				if (dcam_camera->_sensitivity > dcam_camera->_max_safe_sensitivity) {
					
					// secret key not pressed, do not let sens to go too high
					dcam_camera->_sensitivity = dcam_camera->_max_safe_sensitivity;
				}
			}

			live = gci_camera_is_live_mode(camera);
    
			gci_camera_set_sensitivity(camera, CAMERA_ALL_CHANNELS, dcam_camera->_sensitivity);

			if (!live) gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK DCAM_Camera_onCCDMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val = 0;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_CCDMODE, &val);
			gci_camera_set_ccd_mode(camera, val);

			if (!live)
				gci_camera_snap_image(camera);
			else {
				gci_camera_set_live_mode(camera);
				gci_camera_activate_live_display(camera);
			}

			break;
		}
	return 0;
}





int CVICALLBACK DCAM_Camera_onEnableSubtract (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val=0, live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_ENABLESUBTRACT, &val);
			gci_dcam_camera_enable_subtract(camera, val);

			if (!live)
				gci_camera_snap_image(camera);
			else {
				gci_camera_set_live_mode(camera);
				gci_camera_activate_live_display(camera);
			}

			break;
		}
	return 0;
}

int CVICALLBACK DCAM_Camera_onAcquireSubtractRef (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val=0, live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);

			gci_camera_set_snap_mode(camera);

			gci_dcam_camera_acquire_subtract_reference(camera);

			if (!live)
				gci_camera_snap_image(camera);
			else {
				gci_camera_set_live_mode(camera);
				gci_camera_activate_live_display(camera);
			}

			break;
		}
	return 0;
}

int CVICALLBACK DCAM_Camera_onSetSubtractOffset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val=0, live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SUBTRACTOFFSET, &val);
			gci_dcam_camera_setoffset(camera, val);

			if (!live)
				gci_camera_snap_image(camera);
			else {
				gci_camera_set_live_mode(camera);
				gci_camera_activate_live_display(camera);
			}

			break;
		}
	return 0;
}




int CVICALLBACK DCAM_Camera_onLightMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;
	GciCamera *camera = (GciCamera *) dcam_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, &(dcam_camera->_light_mode) );
			gci_camera_set_light_mode(camera, dcam_camera->_light_mode);

			if (!live) gci_camera_snap_image(camera);

			break;
		}
	return 0;
}

int CVICALLBACK DCAM_Camera_onExtrasQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GciCamera *camera = (GciCamera *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			gci_camera_hide_extra_ui(camera);

			break;
		}
	return 0;
}

int CVICALLBACK DCAM_Camera_SetPhotonMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	
	GciCamera *camera = (GciCamera *) callbackData;  
	
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int was_live = gci_camera_is_live_mode(camera);

			GetCtrlVal(panel, control, &val);
			
			gci_dcam_camera_set_photon_mode(camera, val);
			
			if (!was_live)
				gci_camera_snap_image(camera);

			break;
		}
	}
	return 0;
}

int CVICALLBACK DCAM_Camera_TimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			double temp;
			GciCamera *camera = (GciCamera *) callbackData;    
			
			temp = gci_dcam_camera_get_temperature(camera);       
				
			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DEGS_C, temp);            
			
			break;
		}
	}
	
	return 0;
}



