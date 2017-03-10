#include "HardWareTypes.h" 

#include "stage\stage.h"
#include "stage\stage_ui.h"

#include <userint.h>
#include <utility.h>

#include "string_utils.h"
#include "gci_utils.h"

int CVICALLBACK OnStageJoystickToggled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	//Joystick button
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			int status;
			XYStage* stage = (XYStage *) callbackData; 
	
	    	GetCtrlVal(panel, control, &status);
    	
	    	if(status)
	    		stage_set_joystick_on (stage);
	    	else
	    		stage_set_joystick_off (stage);
		}	
		break;
	} 
	
	return 0;
}


int CVICALLBACK OnReinit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			XYStage* stage = (XYStage *) callbackData;
			int full_init;
	
			if(STAGE_VTABLE(stage, hw_init) != NULL) {
    
		    	if( STAGE_VCALL(stage, hw_init)(stage) == STAGE_ERROR ) {
	
					ui_module_send_error(UIMODULE_CAST(stage),  "Stage","Init operation failed for stage");
					return STAGE_ERROR;
		  		}
		  	}
  			
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			full_init = ConfirmPopup("", "OK to initialise XY stage?");
			stage_find_initialise_extents (stage, full_init);
			
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); // just to make sure
		}	
		break;
	}
		
	return 0;
}


int CVICALLBACK OnSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
			stage_display_params_ui(stage);
		}
		break;
	}
	
	return 0;
}


static int StageDisplayInfoPanel(XYStage *stage)
{
	int about_panel, pnl, ctrl;
	char info[500] = "", buffer[500] = "";
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

	if (stage_get_info(stage, info) == STAGE_ERROR ) {
	
		ui_module_send_error(UIMODULE_CAST(stage),  "Stage", "Failed to get stage info");
		return STAGE_ERROR;
	}
	
	find_resource("stage_ui.uir", buffer); 
	about_panel = LoadPanel(0, buffer, ABOUT_PNL); 

	SetCtrlVal(about_panel, ABOUT_PNL_INFO, info);
	
	InstallPopup (about_panel);
	
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		
		if (pnl == about_panel)
			break;
	}
	
	DiscardPanel(about_panel);
	
	return 0;
}


int CVICALLBACK OnAbout (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
		
			StageDisplayInfoPanel(stage);
		}
		break;
	}
	return 0;
}


int CVICALLBACK OnStageJoystickSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double speed;
			XYStage* stage = (XYStage *) callbackData;
	
			GetCtrlVal(stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, &speed);
			stage_set_joystick_speed (stage, speed);
		}
		break;
	}

	return 0;
}


int CVICALLBACK OnStageAdvancedButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
		
			if(!stage->_advanced_view) {
			
				SetPanelAttribute(panel, ATTR_HEIGHT, 440);
				SetCtrlAttribute(panel, control, ATTR_LABEL_TEXT, "Advanced <<");
				stage->_advanced_view = 1;
			}
			else {
			
				SetPanelAttribute(panel, ATTR_HEIGHT, 100);
				SetCtrlAttribute(panel, control, ATTR_LABEL_TEXT, "Advanced >>");
				stage->_advanced_view = 0;
			}
		}
		break;
	}
	
	return 0;
}


int CVICALLBACK OnStageQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_hide_all_panels(UIMODULE_CAST(stage));
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage), "Close", GCI_VOID_POINTER, stage);
		}
		break;
	}
	
	return 0;
}


int CVICALLBACK OnStageAbortInit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
		
			stage_abort_move (stage);
			stage->_init_aborted = 1;
			stage->_abort_move = 1;
		}	
		break;
	}
		
	return 0;
}


// Move handlers

int CVICALLBACK OnStageAbortMove (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			XYStage* stage = (XYStage *) callbackData;
	
			stage_abort_move (stage);
			stage->_abort_move = 1;
		}
		break;
	}
	
	return 0;
}


int CVICALLBACK MoveByX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
	    	double xval; //, t;
		    XYStage* stage = (XYStage *) callbackData; 

	    	//Get the distance to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_REL, &xval);

			//Move stage to required location
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);  
		
//	    	t = Timer();
			stage_rel_move_by (stage, xval, 0.0); 
//			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);
		}
		break;
    }
    	
	return 0;
}


int CVICALLBACK MoveByY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double yval; //, t;
			XYStage* stage = (XYStage *) callbackData; 
    	
	    	//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_REL, &yval);

			//Move stage to required location
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);  
		
//	    	t = Timer();
			stage_rel_move_by (stage, 0.0, yval);
//			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);
		}
		break;
    }
    
	return 0;
}



int CVICALLBACK MoveByXY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double xval, yval; //, t;
			XYStage* stage = (XYStage *) callbackData;
			int joyOn;
	
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyOn);

			//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_REL, &xval);
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_REL, &yval); 

			//Move stage to required location
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE); 
		
//	    	t = Timer();
			stage_rel_move_by (stage, xval, yval);
//			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
			
			if (joyOn) stage_set_joystick_on (stage);	 // the relative move seems to turn the joystick off !!!
		}
		break;
    }
    
	return 0;
}

int CVICALLBACK MoveLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double step;
			XYStage* stage = (XYStage *) callbackData;
			int joyon;

			// the relative move seems to turn the joystick off !!!
			// so let's update the UI, do not want to turn on joystick in this case as these left, right, up, down
			// buttons may be used repeatedly to move around
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyon);
			if (joyon){
				stage_set_joystick_off(stage);
			}

			//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_STEP, &step);

			//Move stage to required location
			stage_rel_move_by (stage, -step, 0.0);
		}
		break;
    }
    
	return 0;
}

int CVICALLBACK MoveRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double step;
			XYStage* stage = (XYStage *) callbackData;
			int joyon;

			// the relative move seems to turn the joystick off !!!
			// so let's update the UI, do not want to turn on joystick in this case as these left, right, up, down
			// buttons may be used repeatedly to move around
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyon);
			if (joyon){
				stage_set_joystick_off(stage);
			}

			//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_STEP, &step);

			//Move stage to required location
			stage_rel_move_by (stage, step, 0.0);
		}
		break;
    }
    
	return 0;
}

int CVICALLBACK MoveUp(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double step;
			XYStage* stage = (XYStage *) callbackData;
			int joyon;

			// the relative move seems to turn the joystick off !!!
			// so let's update the UI, do not want to turn on joystick in this case as these left, right, up, down
			// buttons may be used repeatedly to move around
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyon);
			if (joyon){
				stage_set_joystick_off(stage);
			}

			//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_STEP, &step);

			//Move stage to required location
			stage_rel_move_by (stage, 0.0, -step);
		}
		break;
    }
    
	return 0;
}

int CVICALLBACK MoveDown(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double step;
			XYStage* stage = (XYStage *) callbackData;
			int joyon;

			// the relative move seems to turn the joystick off !!!
			// so let's turn it off, do not want to turn on joystick in this case as these left, right, up, down
			// buttons may be used repeatedly to move around
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyon);
			if (joyon){
				stage_set_joystick_off(stage);
			}

			//Get relative distances to move
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_STEP, &step);

			//Move stage to required location
			stage_rel_move_by (stage, 0.0, step);
		}
		break;
    }
    
	return 0;
}

/*
int CVICALLBACK MoveToX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double xval; //, t;
		    XYStage* stage = (XYStage *) callbackData; 
    
			//Get wanted xcoord
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_ABS, &xval);

			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);       
		
//	    	t = Timer();
			stage_goto_x_position (stage, xval) ;
//			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);   
		}
		break;
	}
	
	return 0;
}


int CVICALLBACK MoveToY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double yval; //, t;
		    XYStage* stage = (XYStage *) callbackData; 
    
			//Get wanted xcoord
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_ABS, &yval);

			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);       
		
//	    	t = Timer();
			stage_goto_y_position (stage, yval) ;
//			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0);   
		}
		break;
	}
	
	return 0;
}
*/


int CVICALLBACK MoveToXY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			double xval, yval; //, t;
		    XYStage* stage = (XYStage *) callbackData; 
			int joyOn;
	
	    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_JOY_ON, &joyOn);
    
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_ABS, &xval);
			GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_ABS, &yval);

			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);  
		
			// Some stage ie LSTEP need manual mode ir joystick off to move to absolute pos ?
			stage_set_joystick_off(stage);

			stage_goto_xy_position(stage, xval, yval);

			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 

			if (joyOn)
				 stage_set_joystick_on (stage);	 // the relative move seems to turn the joystick off !!!
		}
		break;
	}
	
	return 0;
}


int CVICALLBACK MoveTopLeft(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
		
			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   
		
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			if (ConfirmPopup("XY Stage Warning", "Are you sure?")) {
				stage_goto_xy_position (stage, stage->_limits.min_x, stage->_limits.min_y);
			}
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
		}
		break;
	}

	return 0;
}


int CVICALLBACK MoveTopRight(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   
	
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			if (ConfirmPopup("XY Stage Warning", "Are you sure?")) {
				stage_goto_xy_position (stage, stage->_limits.max_x, stage->_limits.min_y);
			}
	
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
		}
		break;
	}

	return 0;
}


int CVICALLBACK MoveBottomLeft(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
 			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   
		
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			if (ConfirmPopup("XY Stage Warning", "Are you sure?")) {
				stage_goto_xy_position (stage, stage->_limits.min_x, stage->_limits.max_y);
			}
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
		}
		break;
	}

	return 0;
}


int CVICALLBACK MoveBottomRight(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			XYStage* stage = (XYStage *) callbackData;

			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   
		
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			if (ConfirmPopup("XY Stage Warning", "Are you sure?")) {
				stage_goto_xy_position (stage, stage->_limits.max_x, stage->_limits.max_y);
			}
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
		}
		break;
	}

	return 0;
}


int CVICALLBACK OnSetXYDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
 			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   
		
			// Some stage ie LSTEP need manual mode ir joystick off to move to absolute pos ?
			stage_set_joystick_off(stage);

			stage_set_current_xy_position_as_xy_datum (stage);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 

			stage_set_joystick_off(stage);
		}
		break;
	}

	return 0;
}


int CVICALLBACK OnGotoXYDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event) 
	{
		case EVENT_COMMIT:
		{		
			XYStage* stage = (XYStage *) callbackData;
	
			ui_module_disable_panel(UIMODULE_MAIN_PANEL_ID(stage), 1, STAGE_PNL_ABORT_MOVE);   

			// Some stage ie LSTEP need manual mode ir joystick off to move to absolute pos ?
			stage_set_joystick_off(stage);

			stage_goto_xy_position (stage, 0.0, 0.0);
			
			stage_set_joystick_on(stage);
		
			ui_module_enable_panel(UIMODULE_MAIN_PANEL_ID(stage), 0); 
		}
		break;
	}

	return 0;
}

