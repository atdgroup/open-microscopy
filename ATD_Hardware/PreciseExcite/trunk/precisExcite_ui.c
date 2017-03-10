#include "precisExcite.h"
#include "precisExcite_ui.h"

#pragma region Callbacks

int CVICALLBACK OnConnect (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	char address[50];
	unsigned int port;
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:
		GetCtrlVal(precise_excite->_setup_panel, SETUP_ADDRESS, address);
		GetCtrlVal(precise_excite->_setup_panel, SETUP_PORT, &port);
		if (preciseExcite_connect(precise_excite, address, port) == 0)
			SetCtrlVal(precise_excite->_setup_panel, SETUP_STATUS, "Connected");

		break;
	}
	return 0;
}

#pragma endregion 


int CVICALLBACK OnSetupQuit (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:

		ui_module_hide_panel(UIMODULE_CAST(precise_excite), precise_excite->_setup_panel);

		break;
	}

	return 0;
}

int CVICALLBACK OnPrecisExciteClose (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:

		ui_module_hide_all_panels(UIMODULE_CAST(precise_excite));

		break;
	}
	return 0;
}

int CVICALLBACK OnPrecisExciteSetup (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:

		ui_module_display_panel(UIMODULE_CAST(precise_excite), precise_excite->_setup_panel);

		break;
	}
	return 0;
}

int CVICALLBACK OnSetIntensityVal (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	int val;
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:
		GetCtrlVal(panel, control, &val);
		if (control == PE_MAIN_VIOLET_VAL) {
			preciseExcite_set_intensity(precise_excite, PE_VIOLET, val);
		}
		else if (control == PE_MAIN_BLUE_VAL) {
			preciseExcite_set_intensity(precise_excite, PE_BLUE, val);
		}
		else if (control == PE_MAIN_GREEN_VAL) {
			preciseExcite_set_intensity(precise_excite, PE_GREEN, val);
		}

		break;
	}

	return 0;
}

int CVICALLBACK OnSetChannelOn (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int val;
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:
		GetCtrlVal(panel, control, &val);
		if (control == PE_MAIN_VIOLET_ON) {
			preciseExcite_set_channel_on_off(precise_excite, PE_VIOLET, val);
		}
		else if (control == PE_MAIN_BLUE_ON) {
			preciseExcite_set_channel_on_off(precise_excite, PE_BLUE, val);
		}
		if (control == PE_MAIN_GREEN_ON) {
			preciseExcite_set_channel_on_off(precise_excite, PE_GREEN, val);
		}

		break;
	}
	return 0;
}

int CVICALLBACK OnPulse (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	double pulse_width;
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:

		if (control == PE_MAIN_VIOLET_PULSE) {
			GetCtrlVal(precise_excite->_main_ui_panel, PE_MAIN_VIOLET_PULSE_WIDTH, &pulse_width);
			pulse_width /= 1000.0;	//seconds
			//preciseExcite_pulse_channel_fudge(precise_excite, PE_VIOLET, pulse_width);
			preciseExcite_pulse_channel(precise_excite, PE_VIOLET, pulse_width);
		}
		else if (control == PE_MAIN_BLUE_PULSE) {
			GetCtrlVal(precise_excite->_main_ui_panel, PE_MAIN_BLUE_PULSE_WIDTH, &pulse_width);
			pulse_width /= 1000.0;	//seconds
			preciseExcite_pulse_channel_fudge(precise_excite, PE_BLUE, pulse_width);
		}
		else if (control == PE_MAIN_GREEN_PULSE) {
			GetCtrlVal(precise_excite->_main_ui_panel, PE_MAIN_GREEN_PULSE_WIDTH, &pulse_width);
			pulse_width /= 1000.0;	//seconds
			preciseExcite_pulse_channel_fudge(precise_excite, PE_GREEN, pulse_width);
		}

		break;
	}
	return 0;
}

int CVICALLBACK OnArm (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	precisExcite* precise_excite = (precisExcite*) callbackData;

	switch (event)
	{
	case EVENT_COMMIT:
		GetCtrlVal(precise_excite->_setup_panel, SETUP_TRIGGER, &precise_excite->_trigger_edge);

		if (control == PE_MAIN_VIOLET_ARM) 
			preciseExcite_arm_channel(precise_excite, PE_VIOLET);
		else if (control == PE_MAIN_BLUE_ARM) 
			preciseExcite_arm_channel(precise_excite, PE_BLUE);
		else if (control == PE_MAIN_GREEN_ARM) 
			preciseExcite_arm_channel(precise_excite, PE_GREEN);

		break;
	}
	return 0;
}
