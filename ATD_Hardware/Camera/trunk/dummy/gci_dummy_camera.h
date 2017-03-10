#ifndef __GCI_DUMMY_CAMERA__
#define __GCI_DUMMY_CAMERA__

#include "camera\gci_camera.h"

typedef struct _DummyCamera DummyCamera;

typedef enum {DUMMY_CAMERA_IMAGE_LOAD_DEFAULT, DUMMY_CAMERA_IMAGE_LOAD_SPECIFIC_FILE, DUMMY_CAMERA_IMAGE_LOAD_FROM_DIR} DUMMY_CAMERA_IMAGE_LOAD_TYPE;

struct _DummyCamera
{
	GciCamera camera;

	char om_file_directory[GCI_MAX_PATHNAME_LEN];
	char filepath[GCI_MAX_PATHNAME_LEN];
	DUMMY_CAMERA_IMAGE_LOAD_TYPE load_type;
	int  load_from_directory_count;

	ListType directory_load_files;
};

GciCamera* gci_dummy_camera_new(const char *name, const char* description);

void gci_dummy_set_display_file_path(DummyCamera* om_camera, const char *path);
void gci_dummy_set_dummy_file_directory(DummyCamera* om_camera, const char *path);
void gci_dummy_set_dummy_enable_cell_testing(DummyCamera* om_camera, int enable);
void gci_dummy_set_dummy_cell_file_path(DummyCamera* om_camera, const char *path);

int ReadLastSpecifiedLoadDirectory(DummyCamera* om_camera, char *directory);
void GetAllImageFilesInDirectoryAsList(GciCamera* camera);

int CVICALLBACK OnDummyCameraSetLoadDirectory (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnDummyCameraOnExtraPanelClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

#endif
