INCLUDE(MAKE_WINDOWS_PATH)

FIND_PATH (LIBRARIES_DIR_PATH UIModule ${MICROSCOPY_TOPLEVEL_SOURCE_DIR}/../../../../ATD_Libraries NO_DEFAULT_PATH)

IF (LIBRARIES_DIR_PATH-NOTFOUND)
    MESSAGE(FATAL_ERROR "Could not find Gci Libraries")
ENDIF (LIBRARIES_DIR_PATH-NOTFOUND)

MAKE_WINDOWS_PATH(LIBRARIES_DIR_PATH)

SET(IMAGEVIEWER_SRCS

		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewer.c
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewer.h
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerBuffer.c
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerBuffer.h
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerMouse.c
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerMouse.h
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerUtils.c
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerUtils.h
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerWndProc.c
		${LIBRARIES_DIR_PATH}/ImageViewer/trunk/ImageViewerWndProc.h
)


INCLUDE_DIRECTORIES(${LIBRARIES_DIR_PATH}/ImageViewer/trunk)
                    
SOURCE_GROUP(ImageViewer FILES ${IMAGEVIEWER_SRCS})