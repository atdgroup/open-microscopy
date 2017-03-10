#include <userint.h>

#include "analyzer.h"
#include "analyzer_ui.h"

int CVICALLBACK OnAnalyserStateChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Analyzer *analyzer = (Analyzer *) callbackData;
	int status;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (analyzer->_main_ui_panel, ANALYZER_STATUS, &status);
			if (status == 1)
				analyzer_in(analyzer);
			else
				analyzer_out(analyzer);
			break;
		}
	return 0;
}

int CVICALLBACK OnAnalyzerClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Analyzer *analyzer = (Analyzer*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			ui_module_hide_all_panels(UIMODULE_CAST(analyzer));
	
			break;
	}
	
	return 0;
}
