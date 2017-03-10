#ifndef __CAMERA_CALLBACKS__
#define __CAMERA_CALLBACKS__

int CVICALLBACK Camera_onSnap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onLive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onExposure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onAutoExposure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onReinit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onAverage (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onSaveSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onLoadSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onSetDefaults (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onExtras (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onQuit(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onInfo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onAverageFramesChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK Camera_onSaveImages (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

#endif
