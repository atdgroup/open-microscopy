#include "HardWareTypes.h"

#include "gci_dummy_camera.h"
#include "camera\gci_camera.h"
#include "uir_files\gci_camera_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include <utility.h>

#include <userint.h>
#include <ansi_c.h>

#include "FreeImage.h"
#include "FreeImageAlgorithms_IO.h"

#include <userint.h>

int CVICALLBACK OnDummyCameraSetLoadDirectory (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
		 	GciCamera *camera = (GciCamera*) callbackData;
			DummyCamera* om_camera = (DummyCamera*) camera;
			char reg_key[500], buffer[500];
			char name[UIMODULE_NAME_LEN];

			int err, file_size;
			char directory[GCI_MAX_PATHNAME_LEN];

			gci_camera_get_name(camera, name);

			sprintf(reg_key, "software\\GCI\\Microscope\\Cameras\\%s\\", name);

			err = ReadLastSpecifiedLoadDirectory(om_camera, buffer);

			if(DirSelectPopup (buffer, "Fake Image Directory", 1, 1, directory) == 0)
				return 0;
            
			if(!FileExists(directory, &file_size))
				MakeDir(directory);

			RegWriteString (REGKEY_HKCU, reg_key, "LastSpecifiedLoadDir", (const char *) directory);
			
			strcpy(om_camera->om_file_directory, directory);
			om_camera->load_type = DUMMY_CAMERA_IMAGE_LOAD_FROM_DIR;
			om_camera->load_from_directory_count = 1;

			GetAllImageFilesInDirectoryAsList(camera);

			break;
		}
	}
		
	return 0;
}


int CVICALLBACK OnDummyCameraOnExtraPanelClose (int panel, int control, int event,
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
