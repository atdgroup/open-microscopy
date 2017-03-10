#include "Imaq_CVI.h"

// setup the draggable box
// provide the panel handle and the canvas control ID,
// a colour for the box and for the background,
// and also the name of a callback function which should be declared thus:
// void myFunction (Rect box);
// this will be called as the box is moved and after the box has been resized,
// box will contain the box coordinates.

int GCI_setupCanvasDrawTools(int panelIn, int controlIn, int toolIn, int boxColour, int bgColour, void (*externalFn)(Rect, void* data), void* callback_data);
int GCI_stopCanvasDrawTools(void);
void GCI_externalReDrawShapeTool (Rect box);
int GCI_getDrawToolsMask(IPIImageRef image, int drawMode);
void GCI_fixShapeAspectRatio (float ratio);
