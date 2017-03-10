#ifndef __GCI_IMAGE_UTILS
#define __GCI_IMAGE_UTILS

// P Barber
#include "IMAQ_CVI.h"

void Normalise ( IPIImageRef image, IPIImageRef image2 );
int makeCountColTable (int *MyPal, int entry1, int colour1, int entry2, int colour2, int entry3, int colour3);
int createLogColTable (int *ct, int con, int br);
int makeODColTable (int *col, int n, int colour, int contrast);
void makeSeismicColTable (int *Palette, int n);
int makeColTable (int *col, int n, int colour);
void makeRainbowColTable (int *rainbowPalette, int n);
void makeReverseRainbowColTable (int *rainbowPalette, int n);
int adjustColTable (int *col, int n, double contrast, double brightness);
int thresholdColTable(int *col, int n);
int overloadColTable(int *col, int n);
int rescaleImageTo8bits (IPIImageRef image, IPIImageRef image2, float min, float max);
IPIError IPI_ImageBorderSize (IPIImageRef source_image, long Operation, long *border_size);
int formatFloatToString (float n, char *s);
int makeHistogram(float data[], int ndata, int histo[], int nhisto, float low, float high);
void ReverseImage(IPIImageRef input, IPIImageRef output);
void ReverseMask(IPIImageRef input, IPIImageRef output);
int blendMaskWithImage(IPIImageRef mask, IPIImageRef image, IPIImageRef output, int colour, int outline, int hatch, int opacity);
IPIError imageHistogram (IPIImageRef image, IPIImageRef mask_image, int n, float imin, float imax, int *histogram_array, IPIHistoReport *report);
int GCI_convertIndexedColourToRGB (IPIImageRef indexed, int *colTable, int cols, IPIImageRef rgb);

IPIHistoReport Normalise0To1 ( IPIImageRef image, IPIImageRef image2 );
int U16ToU8Image (IPIImageRef in, IPIImageRef out);
int U16ToSGLImage (IPIImageRef in, IPIImageRef out);

int imageRotate90 (IPIImageRef imageIn, IPIImageRef imageOut, int rotateLeft, int flip);

int GetPaletteFromFile(const char *filepath, int *palette);
int SavePaletteToFile(const char *filepath, int *palette);

#endif
