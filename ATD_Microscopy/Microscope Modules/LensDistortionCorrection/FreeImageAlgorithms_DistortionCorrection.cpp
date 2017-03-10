#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_DistortionCorrection.h"

#include "math.h"

#define DISCAL_NORMALISATION ((double) sqrt((double) (768.0*768.0 + 576.0*576.0)))

template < class Tsrc > class DISTORTION
{
  public:
    FIBITMAP* CorrectLensDistortion (FIBITMAP *input, double a, double b, double c, double d, FIAPOINT offset, bool colour);	
	inline Tsrc* GetPixelFromCenter(FIBITMAP *input, int width, int height, int rx, int ry, FIAPOINT offset);
	inline float AverageNearestNeighbours(Tsrc* ptr, unsigned int pitch);
};

DISTORTION < unsigned char >distortionUCharImage;
DISTORTION < unsigned short >distortionUShortImage;
DISTORTION < short >distortionShortImage;
DISTORTION < unsigned long >distortionULongImage;
DISTORTION < long >distortionLongImage;
DISTORTION < float >distortionFloatImage;
DISTORTION < double >distortionDoubleImage;


FIBITMAP *DLL_CALLCONV
FIA_CorrectLensDistortion ( FIBITMAP* input, double a, double b, double c, double d, FIAPOINT offset)
{
    FIBITMAP *dst = NULL;

    if (!input)
        return NULL;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (input);

    switch (src_type)
    {
		case FIT_BITMAP: {    // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
        
			int bpp = FreeImage_GetBPP (input);

			if (bpp == 8)
                dst = distortionUCharImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
			else if(bpp >= 24)
				dst = distortionUCharImage.CorrectLensDistortion (input, a, b, c, d, offset, true);

            break;
		}
		
		case FIT_UINT16:       // array of unsigned short: unsigned 16-bit
            dst = distortionUShortImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        case FIT_INT16:        // array of short: signed 16-bit
            dst = distortionShortImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        case FIT_UINT32:       // array of unsigned long: unsigned 32-bit
            dst = distortionULongImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        case FIT_INT32:        // array of long: signed 32-bit
            dst = distortionLongImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        case FIT_FLOAT:        // array of float: 32-bit
            dst = distortionFloatImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        case FIT_DOUBLE:       // array of double: 64-bit
            dst = distortionDoubleImage.CorrectLensDistortion (input, a, b, c, d, offset, false);
            break;
        default:
            break;
    }

    if (NULL == dst)
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                     "FREE_IMAGE_TYPE: Unable to perform FIA_CorrectLensDistortion for type %d.",
                                     src_type);
    }

    return dst;
}

 
template <class Tsrc>
inline Tsrc* DISTORTION <Tsrc>::GetPixelFromCenter(FIBITMAP *input, int width, int height, 
												   int rx, int ry, FIAPOINT offset)
{
	FIAPOINT origin;

	origin.x = width/2  - offset.x;      // image centre wrt image bottom-left.
    origin.y = height/2 - offset.y;

	int x = origin.x + rx;
	int y = origin.y + ry;
	
	if(x < 0 || x > width - 1)
		return NULL;

	if(y < 0 || y > height - 1)
		return NULL;

	Tsrc *ptr = (Tsrc *) FreeImage_GetScanLine (input, y);

	ptr += x;

	return ptr;
}

template <class Tsrc>
inline float DISTORTION <Tsrc>::AverageNearestNeighbours(Tsrc* ptr, unsigned int pitch)
{
	float value = 0.0f;

	ptr -= pitch;
	value += *(ptr - 1) + *ptr + *(ptr + 1);

	ptr += pitch;
	value += *(ptr - 1) + *(ptr + 1);

	ptr -= pitch;
	value += *(ptr - 1) + *ptr + *(ptr + 1);
	
	value /= 8.0;

	return value;
}


// Remove Barrel and/or Pin Cushion distortion using bilinear interpolation, Paul Barber 21.11.01
// -ve a, b or c factors correct for barrel distortion, +ve correct for pin cushion
// -ve d magnifies, +ve d reduces (i.e. it is additional to a scaling of 1.0)
// to retain scale, make sure a+b+c+d=0, (applying distortion will inevitably change the scale in some areas of the image)
template <class Tsrc>
FIBITMAP* DISTORTION <Tsrc>::CorrectLensDistortion (FIBITMAP *input, double a, double b, double c, double d,
																			 FIAPOINT offset, bool colour)
{
    FIBITMAP *dst = NULL;
    double r,sr,norm;
	int width, height;
	int cx,cy,sx=1,sy=1, x_origin, y_origin, maxX, maxY;
    Tsrc *pix = NULL, *pix2 = NULL;
	
	width = FreeImage_GetWidth(input);
	height = FreeImage_GetHeight(input);

	dst = FreeImage_Clone(input);

    //norm = ((double) sqrt((double) (width*width + height*height))) / 2.0;
	norm = (double) min(width, height) / 2.0;
	//norm = (double) max(width, height) / 2.0;
    
    x_origin = width/2  - offset.x;      // image centre wrt image bottom-left.
    y_origin = height/2 - offset.y;
 
    maxY = height - 1;
    maxX = width - 1;
    
	Tsrc *src_ptr = NULL;
	Tsrc *dst_ptr = NULL;

	unsigned int pitch = FreeImage_GetPitch(input);

	for(register int y = 0; y < height; y++)
	{
		cy = y-y_origin;

		dst_ptr = (Tsrc *) FreeImage_GetScanLine (dst, y);
		src_ptr = (Tsrc *) FreeImage_GetScanLine (input, y);

		for(register int x = 0; x < width; x++) {

			cx = x-x_origin;
			r  = (double) (sqrt((double)(cx*cx+cy*cy)) / norm);    // Normalised distance to (x,y) in destination image from imager centre
			sr = (a*r*r*r + b*r*r + c*r + d); // Do this instead, no need to *r and then /r.

			pix = this->GetPixelFromCenter(input, width, height, sr*cx, sr*cy, offset);
			pix2 = this->GetPixelFromCenter(dst, width, height, cx, cy, offset);

			if(pix != NULL && pix2 != NULL) { 
				*pix2 = this->AverageNearestNeighbours(pix, pitch);

			}
		}
	}

    return dst;
}