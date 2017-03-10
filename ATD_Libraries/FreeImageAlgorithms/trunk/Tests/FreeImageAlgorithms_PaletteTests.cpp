#include "CuTest.h"

#include "Constants.h"
#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Arithmetic.h"

#include "FreeImageAlgorithms_Testing.h"

static void
TestFIA_SetFalseColourPalette_ForColour(CuTest* tc)
{
	FIBITMAP *dib1 = NULL, *dib2 = NULL;
	FREE_IMAGE_TYPE type;
	int bpp, err;
    
    const char *file = TEST_DATA_DIR "fly.bmp";
	dib1 = FIA_LoadFIBFromFile(file);

	CuAssertTrue(tc, dib1 != NULL);

	bpp = FreeImage_GetBPP(dib1);
	type = FreeImage_GetImageType(dib1);

	CuAssertTrue(tc, bpp == 8);
	CuAssertTrue(tc, type == FIT_BITMAP);

	FIA_SetFalseColourPalette_ForColour(dib1, FIA_RGBQUAD(255,0,0));
	err = FIA_SaveFIBToFile (dib1, TEST_DATA_OUTPUT_DIR "/Palette/SetFalseColourPalette_ForColour-Test.bmp", BIT24);

	CuAssertTrue(tc, err == FIA_SUCCESS);

	FreeImage_Unload(dib1);
}



static void
TestFIA_Add8BitImageToColourImageTest(CuTest* tc)
{
	FREE_IMAGE_TYPE type;
	int bpp, width, height, err;
 
    const char *file1 = TEST_DATA_DIR "fly_top_right_gs.jpg";
	const char *file2 = TEST_DATA_DIR "red-kneed-tarantula_topleft_gs.jpg";
	const char *file3 = TEST_DATA_DIR "gregarious-desert-locusts-bottom_left_gs.jpg";
	const char *composite_file = TEST_DATA_DIR "fibres.jpg";

	FIBITMAP *dib1 = FIA_LoadFIBFromFile(file1);
	FIBITMAP *dib2 = FIA_LoadFIBFromFile(file2);
	FIBITMAP *dib3 = FIA_LoadFIBFromFile(file3);
	FIBITMAP *composite_dib = FIA_LoadFIBFromFile(composite_file);

	CuAssertTrue(tc, dib1 != NULL);
	CuAssertTrue(tc, dib2 != NULL);
	CuAssertTrue(tc, dib3 != NULL);
	CuAssertTrue(tc, composite_dib != NULL);

	FIA_InPlaceConvertTo24Bit(&composite_dib);

	width = FreeImage_GetWidth(dib1);
	height = FreeImage_GetHeight(dib1);

	FIA_SetFalseColourPalette_ForColour(dib1, FIA_RGBQUAD(255,0,0));
	FIA_Add8BitImageToColourImage(composite_dib, dib1, GREY_LEVEL_ADD_AVERAGE);

	FIA_SetFalseColourPalette_ForColour(dib2, FIA_RGBQUAD(0,255,0));
	FIA_Add8BitImageToColourImage(composite_dib, dib2, GREY_LEVEL_ADD_AVERAGE);

	FIA_SetFalseColourPalette_ForColour(dib3, FIA_RGBQUAD(255,0,153));
	FIA_Add8BitImageToColourImage(composite_dib, dib3, GREY_LEVEL_ADD_AVERAGE);

	err = FIA_SaveFIBToFile (composite_dib, TEST_DATA_OUTPUT_DIR "/Palette/Add8BitImageToColourImageTest.bmp", BIT24);

	CuAssertTrue(tc, err == FIA_SUCCESS);

	FreeImage_Unload(dib1);
}

static void
TestFIAOverlay8BitImageOverColourImage(CuTest* tc)
{
	FREE_IMAGE_TYPE type;
	int bpp, width, height, err;
 
    const char *file1 = TEST_DATA_DIR "fly_top_right_gs.jpg";
	const char *file2 = TEST_DATA_DIR "red-kneed-tarantula_topleft_gs.jpg";
	const char *file3 = TEST_DATA_DIR "gregarious-desert-locusts-bottom_left_gs.jpg";
	const char *composite_file = TEST_DATA_DIR "fibres.jpg";

	FIBITMAP *dib1 = FIA_LoadFIBFromFile(file1);
	FIBITMAP *dib2 = FIA_LoadFIBFromFile(file2);
	FIBITMAP *dib3 = FIA_LoadFIBFromFile(file3);
	FIBITMAP *composite_dib = FIA_LoadFIBFromFile(composite_file);

	CuAssertTrue(tc, dib1 != NULL);
	CuAssertTrue(tc, dib2 != NULL);
	CuAssertTrue(tc, dib3 != NULL);
	CuAssertTrue(tc, composite_dib != NULL);

	FIA_InPlaceConvertTo24Bit(&composite_dib);

	width = FreeImage_GetWidth(dib1);
	height = FreeImage_GetHeight(dib1);

	FIA_SetFalseColourPalette_ForColour(dib1, FIA_RGBQUAD(255,0,0));
	FIA_Overlay8BitImageOverColourImage (composite_dib, dib1, 128);

	FIA_SetFalseColourPalette_ForColour(dib2, FIA_RGBQUAD(0,255,0));
	FIA_Overlay8BitImageOverColourImage (composite_dib, dib2, 128);

	FIA_SetFalseColourPalette_ForColour(dib3, FIA_RGBQUAD(255,0,153));
	FIA_Overlay8BitImageOverColourImage (composite_dib, dib3, 128);

	err = FIA_SaveFIBToFile (composite_dib, TEST_DATA_OUTPUT_DIR "/Palette/FIA_Overlay8BitImageOverColourImageTest.bmp", BIT24);

	CuAssertTrue(tc, err == FIA_SUCCESS);

	FreeImage_Unload(dib1);
}




CuSuite* DLL_CALLCONV
CuGetFreeImageAlgorithmsPaletteSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	MkDir(TEST_DATA_OUTPUT_DIR "/Palette");

	SUITE_ADD_TEST(suite, TestFIA_SetFalseColourPalette_ForColour);
	SUITE_ADD_TEST(suite, TestFIA_Add8BitImageToColourImageTest);
	SUITE_ADD_TEST(suite, TestFIAOverlay8BitImageOverColourImage);

	return suite;
}
