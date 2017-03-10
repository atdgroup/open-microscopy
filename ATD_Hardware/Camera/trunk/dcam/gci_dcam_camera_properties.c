#include "toolbox.h"
#include "gci_dcam_camera.h"
#include "uir_files\gci_camera_ui.h"
#include "gci_dcam_camera_ui.h" 
#include "gci_dcam_camera_settings.h"
#include "gci_dcam_camera_lowlevel.h" 
#include "string_utils.h"
#include "gci_utils.h"

#include "dcamapi.h"
#include "features.h"
#include "dcamapix.h"
#include "dcamprop.h"

const int initial_width = 400, initial_height = 500;

typedef struct
{
    char name[500];
    int iProp;
    
} DCAM_PROPERTY;

static LRESULT CALLBACK PropertiesWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	GciCamera *camera = (GciCamera *) data;
	GciDCamCamera *dcam_camera = (GciDCamCamera *) data;

	switch(message) {
			
		case WM_SIZE:
    	case WM_EXITSIZEMOVE:
    	{
			int left, top, width, height, right, bottom, panel_width, panel_height;

			GetPanelAttribute(dcam_camera->prop_panel_id, ATTR_WIDTH, &panel_width);
			GetPanelAttribute(dcam_camera->prop_panel_id, ATTR_HEIGHT, &panel_height);

			if(panel_width < initial_width)
				SetPanelAttribute(dcam_camera->prop_panel_id, ATTR_WIDTH, initial_width);	

			if(panel_height < initial_height)
				SetPanelAttribute(dcam_camera->prop_panel_id, ATTR_HEIGHT, initial_height);

			ui_module_anchor_control_to_panel_edge(dcam_camera->prop_panel_id, dcam_camera->prop_listbox, 10, 250, 10, 50);
			GetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_listbox, ATTR_LEFT, &left);
			GetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_listbox, ATTR_WIDTH, &width);
			right = left + width;
			ui_module_anchor_control_to_panel_edge(dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, right + 10, 10, 10, 250);
			GetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, ATTR_TOP, &top);
			GetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, ATTR_HEIGHT, &height);
			bottom = top + height;

			ui_module_anchor_control_to_panel_edge(dcam_camera->prop_panel_id, dcam_camera->prop_values_listbox, right + 10, 10, bottom + 10, 50);

			// Move Hide Button    
			ui_module_move_control_pixels_from_bottom(dcam_camera->prop_panel_id, dcam_camera->prop_hide_button_id, 10); 
			ui_module_move_control_pixels_from_right(dcam_camera->prop_panel_id, dcam_camera->prop_hide_button_id, 10);


			break;
    	}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) dcam_camera->old_wndproc,
							hwnd, message, wParam, lParam);
}

static int CVICALLBACK OnHidePropertiesPanel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;

			ui_module_hide_panel(UIMODULE_CAST(dcam_camera), dcam_camera->prop_panel_id);  
			
			break;
		}
	}
	
	return 0;
}


void properties_dialog_destroy(GciDCamCamera *dcam_camera)
{
	// Restore WndProc pointer
	ui_module_restore_cvi_wnd_proc(dcam_camera->prop_panel_id); 

	ui_module_destroy(UIMODULE_CAST(dcam_camera)); 
}

void properties_dialog_show(GciDCamCamera *dcam_camera)    
{																	
	ui_module_display_panel(UIMODULE_CAST(dcam_camera), dcam_camera->prop_panel_id);   
}
    
int value_to_string(GciDCamCamera *dcam_camera, long iProp, double value, char* text, long textsize )
{
	DCAM_PROPERTYATTR	pa;
	DCAM_PROPERTYVALUETEXT	pvt;
	memset( &pvt, 0, sizeof( pvt ) );
	pvt.cbSize	= sizeof( pvt );
	pvt.iProp	= iProp;
	pvt.value	= value;
	pvt.text	= text;
	pvt.textbytes = textsize;

	if( dcam_getpropertyvaluetext(dcam_camera->_hCam, &pvt ) )
		return TRUE;

	memset( &pa, 0, sizeof( pa ) ); 
	pa.cbSize	= sizeof( pa );
	pa.iProp	= iProp;

	dcam_getpropertyattr(dcam_camera->_hCam, &pa );

	if( ( pa.attribute & DCAMPROP_TYPE_MASK ) == DCAMPROP_TYPE_REAL )
	{
		sprintf_s( text, textsize, "%g", value );
	}
	else if( ( pa.attribute & DCAMPROP_TYPE_MASK ) == DCAMPROP_TYPE_LONG )
	{
		sprintf_s( text, textsize, "%d", (long)value );
	}
	else
	{
		assert( ( pa.attribute & DCAMPROP_TYPE_MASK ) == DCAMPROP_TYPE_MODE );
		sprintf_s( text, textsize, "(invalid value; %g)", value );
	}

	return TRUE;
}

static void display_attribute_properties(GciDCamCamera *dcam_camera, DCAM_PROPERTYATTR	pa)
{
	if( pa.attribute != 0 )
	{
		switch( pa.attribute & DCAMPROP_TYPE_MASK )
		{
		    default:					assert( 0 );
		    case DCAMPROP_TYPE_NONE:
            {
                InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "(type_none)", 0);
                break;
            }
		    case DCAMPROP_TYPE_MODE:
            {            
                InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "TYPE_MODE", 0);
                break;
            }
		    case DCAMPROP_TYPE_LONG:
            {
                InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "TYPE_LONG", 0);
                break;
            }
		    case DCAMPROP_TYPE_REAL:
            {
                
                break;
            }
		}

        if(pa.attribute & DCAMPROP_ATTR_WRITABLE)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "WRITABLE", 0);

        if(pa.attribute & DCAMPROP_ATTR_READABLE)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "READABLE", 0);

        if(pa.attribute & DCAMPROP_ATTR_EFFECTIVE)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "EFFECTIVE", 0);

        if(pa.attribute & DCAMPROP_ATTR_DATASTREAM)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "DATASTREAM", 0);

        if(pa.attribute & DCAMPROP_ATTR_ACCESSREADY)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "ACCESSREADY", 0);

        if(pa.attribute & DCAMPROP_ATTR_ACCESSBUSY)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "ACCESSBUSY", 0);

        if(pa.attribute & DCAMPROP_ATTR_HASVIEW)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "HASVIEW", 0);

        if(pa.attribute & DCAMPROP_ATTR_HASCHANNEL)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "HASCHANNEL", 0);

        if(pa.attribute & DCAMPROP_ATTR_VOLATILE)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "VOLATILE", 0);

        if(pa.attribute & DCAMPROP_ATTR_ACTION)
            InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, -1, "ACTION", 0);
	}
}

static should_property_be_greyed(GciDCamCamera *dcam_camera, DCAM_PROPERTY	prop)
{
	DCAM_PROPERTYATTR	pa;
	memset( &pa, 0, sizeof( pa ) ); 
	pa.cbSize	= sizeof( pa );
	pa.iProp	= prop.iProp;

	dcam_getpropertyattr(dcam_camera->_hCam, &pa);

	if( ( pa.attribute & DCAMPROP_ATTR_WRITABLE ) == 0 )
	{
		return TRUE;
	}
	else
	{
		DWORD status;

		dcam_getstatus(dcam_camera->_hCam, &status);

		if( status == DCAM_STATUS_UNSTABLE
		 ||	status == DCAM_STATUS_STABLE 
		 || status == DCAM_STATUS_READY && ( pa.attribute & DCAMPROP_ATTR_ACCESSREADY )
		 || status == DCAM_STATUS_BUSY  && ( pa.attribute & DCAMPROP_ATTR_ACCESSBUSY ) )
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

int create_properties(GciDCamCamera *dcam_camera)
{
	long	iProp = 0;
    long	iItem = 0;
    DCAM_PROPERTY prop;
    
    ClearListCtrl(dcam_camera->prop_panel_id, dcam_camera->prop_listbox);
    ListClear(dcam_camera->_properties);
    
	if( dcam_getnextpropertyid(dcam_camera->_hCam, &iProp, 0) )
	{	
		dcam_camera->has_properties = 1;

		do
		{
			dcam_getpropertyname(dcam_camera->_hCam, iProp, prop.name, sizeof(prop.name));
         
            prop.iProp = iProp;

            ListInsertItem(dcam_camera->_properties, &prop, END_OF_LIST);

            //if(should_property_be_greyed(DCAM_PROPERTY	prop)) {
                // sprintf(buffer, "\033fg000000\033bgFFA500%s", node->error);
		
		        //InsertListItem (tm->_panel_id, PROP_PNL_PROP_LIST, 0, buffer, 0);
            //}
			
            
		} while( dcam_getnextpropertyid(dcam_camera->_hCam, &iProp, 0) );
	}

	return CAMERA_SUCCESS;
}
    
    
void update_values(GciDCamCamera *dcam_camera)
{
    DCAM_PROPERTY *prop = NULL;
	int i;
    long	iProp = 0;
    double	value;
    char	text[64] = "";
    
    for(i=1; i <= ListNumItems(dcam_camera->_properties); i++) {
    
		prop = ListGetPtrToItem(dcam_camera->_properties, i);
        
        iProp = prop->iProp;
		
        if( !dcam_getpropertyvalue(dcam_camera->_hCam, iProp, &value ) )
		{
			strcpy_s( text, sizeof( text ), "(invalid)" );
		}
		else
		{
			value_to_string(dcam_camera, iProp, value, text, sizeof(text));
		}
            
		InsertListItem (dcam_camera->prop_panel_id, dcam_camera->prop_listbox, 0, text, 0);
	}         
}


/*
void CDlgDcamProperty::edit_property_of( long index )
{
	m_bChangingEditbox = TRUE;

	m_editprop.indexOnListview	= index;
	m_editprop.attribute = 0;

	long	iPropBase = 0;
	if( m_hdcam != NULL && index >= 0 )
	{
		ASSERT( index < m_listview.GetItemCount() );

		long	i = (long)m_listview.GetItemData( m_editprop.indexOnListview );
		ASSERT( 0 <= i && i < m_arrayIDPROP.GetSize() );

		iPropBase = m_arrayIDPROP.GetAt( i );
		long	iProp = iPropBase + m_idpropoffset;

		DCAM_PROPERTYATTR	pa;
		memset( &pa, 0, sizeof( pa ) );
		pa.cbSize	= sizeof( pa );
		pa.iProp	= iProp;

		CString	strValue;

		if( ! dcam_getpropertyattr( m_hdcam, &pa ) )
		{
			ASSERT( m_idpropoffset != 0 );
			m_editprop.attribute	= 0;
		}
		else
		{
			m_editprop.attribute	= pa.attribute;
			m_editprop.viewcount	= pa.nMaxView;
			m_editprop.channelcount	= pa.nMaxChannel;

			if( m_editprop.viewcount <= 1 && m_editprop.channelcount <= 1 )
			{
				m_idpropoffset = 0;	// all
				m_iViewCh = 0;
			}

			double	value;
			VERIFY( dcam_getpropertyvalue( m_hdcam, iProp + m_idpropoffset, &value ) );

			if( m_bUseListboxAlways )
			{
				m_lbValues.ResetContent();
				fill_propertyvaluetext_into_listbox( m_lbValues, m_hdcam, iProp, value );
			}
			else
			{
				switch( m_editprop.attribute & DCAMPROP_TYPE_MASK )
				{
				case DCAMPROP_TYPE_NONE:	break;
				case DCAMPROP_TYPE_MODE:
					m_lbValues.ResetContent();
					fill_propertyvaluetext_into_listbox( m_lbValues, m_hdcam, iProp, value );
					break;

				case DCAMPROP_TYPE_LONG:
					m_txtMin.SetWindowText( get_string_as_long( pa.valuemin ) );
					m_txtMax.SetWindowText( get_string_as_long( pa.valuemax ) );

					m_fRatioSlider = 1;
					m_fStepSlider = pa.valuestep;
					m_sliderValue.SetRange( -(long)pa.valuemax, -(long)pa.valuemin );
					m_sliderValue.SetPos( -(long)value );

					strValue = get_string_as_long( value );
				//	m_spinValue.
					break;
				case DCAMPROP_TYPE_REAL:
					m_txtMin.SetWindowText( get_string_as_real( pa.valuemin ) );
					m_txtMax.SetWindowText( get_string_as_real( pa.valuemax ) );

					if( pa.valuestep > 0 )
					{
						if( ( pa.valuemax - pa.valuemin ) / pa.valuestep >= 65536 )
							m_fRatioSlider = ceil( ( pa.valuemax - pa.valuemin ) / pa.valuestep / 65536 ) * pa.valuestep;
						else
							m_fRatioSlider = pa.valuestep;
					}
					else
					{
						if( ( pa.valuemax - pa.valuemin ) >= 65536 )
							m_fRatioSlider = ceil( ( pa.valuemax - pa.valuemin ) / 65536 );
						else
							m_fRatioSlider = 1;
					}

					m_sliderValue.SetRange( -(long)( pa.valuemax / m_fRatioSlider ), -(long)( pa.valuemin / m_fRatioSlider ) );
					m_sliderValue.SetPos( -(long)( value / m_fRatioSlider ) );

					strValue = get_string_as_real( value );
					break;
				}
			}
		}

		if( ! strValue.IsEmpty() && GetFocus() != &m_ebValue )
		{
			m_ebValue.SetWindowText( strValue );
			m_listview.SetItemText( m_editprop.indexOnListview, INDEX_PROPERTY_VALUE, strValue );
		}
	}

	if( m_editprop.idprop != iPropBase )
	{
		m_editprop.idprop = iPropBase;

		update_controls();
	}

	m_bChangingEditbox = FALSE;
}
*/

int CVICALLBACK OnDCamPropertyTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			GciDCamCamera *dcam_camera = (GciDCamCamera *) callbackData;

            update_values(dcam_camera);
	     
            break;    
        }
    }
    
    return 0;
}

int dcam_camera_has_properties(GciDCamCamera *dcam_camera)
{
	return dcam_camera->has_properties;
}


void properties_dialog_new(GciDCamCamera *dcam_camera)
{
	dcam_camera->has_properties = 0;

	dcam_camera->prop_panel_id =  NewPanel (0, "DCam Camera Properties", 100, 100, initial_width, initial_height);

    dcam_camera->prop_listbox = NewCtrl(dcam_camera->prop_panel_id, CTRL_TREE, "", 10, 10);
	dcam_camera->prop_attr_textbox = NewCtrl(dcam_camera->prop_panel_id, CTRL_TEXT_BOX, "", 10, 10);
	dcam_camera->prop_values_listbox = NewCtrl(dcam_camera->prop_panel_id, CTRL_TREE, "", 10, 10);

	dcam_camera->prop_hide_button_id = NewCtrl(dcam_camera->prop_panel_id, CTRL_SQUARE_COMMAND_BUTTON, "Close", 430, 150);    

	SetPanelAttribute(dcam_camera->prop_panel_id, ATTR_CONFORM_TO_SYSTEM, 1);
	
	// Set text box to indicator
	SetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, ATTR_NO_EDIT_TEXT, 1);	
	SetCtrlAttribute (dcam_camera->prop_panel_id, dcam_camera->prop_attr_textbox, ATTR_SCROLL_BARS, VAL_BOTH_SCROLL_BARS);
	
	SetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_hide_button_id, ATTR_CMD_BUTTON_COLOR, MakeColor(30, 87, 174));
	SetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_hide_button_id, ATTR_LABEL_COLOR, VAL_WHITE);   
	
	InstallCtrlCallback(dcam_camera->prop_panel_id, dcam_camera->prop_hide_button_id, OnHidePropertiesPanel, dcam_camera);
	
	dcam_camera->old_wndproc = ui_module_set_window_proc(UIMODULE_CAST(dcam_camera), dcam_camera->prop_panel_id, (LONG_PTR) PropertiesWndProc);   

    dcam_camera->prop_timer = NewCtrl(dcam_camera->prop_panel_id, CTRL_TIMER, "", 0, 0);
		
	if ( InstallCtrlCallback (dcam_camera->prop_panel_id, dcam_camera->prop_timer, OnDCamPropertyTimerTick, dcam_camera) < 0)
        return;	
		
	SetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_timer, ATTR_INTERVAL, 5.0);  
	SetCtrlAttribute(dcam_camera->prop_panel_id, dcam_camera->prop_timer, ATTR_ENABLED, 0);

    create_properties(dcam_camera);
    update_values(dcam_camera);
}
