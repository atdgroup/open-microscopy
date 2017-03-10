/////////////////////////////////////////////////////////
//Define what hardware is present in the system
//RJL   Dec 2001
/////////////////////////////////////////////////////////
//RJL   Feb 2006 ....
//Microscopy version
//	Items marked "//Obsolete" are left here for backwards compatibility. 
/////////////////////////////////////////////////////////

//#define  __MICROFOCUS_APP__
#define  __MICROSCOPY_APP__

#define HARDWARE_EXISTS 1		//Obsolete		
//#define I2C_PRESENT 	1			
#define I2CPORT         4		//Obsolete 
#define I2CPORTSTR  	"COM4"	//Obsolete
//#define POWER_VIA_I2C   1

#define USE_ASYNCHRONOUS_TIMERS //Comment to use normal timers

//////////////////// Microscope definitions ////////////////////

//For new modular applications, (>= 2005) comment out if not present
//For older applications set as 0

//#define TE2000_PRESENT  				1
//#define OBJECT_Z_TURRET_PRESENT 		1  
#define XYZ_PRESENT  					1
#define OPTICAL_SHUTTER_PRESENT 		1	//Obsolete 
#define FAST_SHUTTER_PRESENT 			1
#define OBJECTIVE_TURRET_PRESENT 		1
#define FLUOR_CUBES_PRESENT 			1
//#define EPI_CUBES_PRESENT 			1
//#define HG_LAMP_PRESENT 				1
#define LAMP_PRESENT	 				1

//For TE2000 these are the actual numbers sent to the microscope to set the 
//optical path. They may vary from system to system depending on where the
//camera(s) are mounted.
#define LASER_PATH      0   //mirror position
#define ORCA_PATH       5
#define JVC_KYF75_PATH  2   
#define EYEPIECE_PATH   1   

//////////////////// Camera definitions ////////////////////

#define CAMERA_PRESENT 		1		//Obsolete
//#define ORCA_PRESENT 		1
//#define JVC_PRESENT 		1
//#define KYF75_PRESENT 	0
//#define DUMMYCAM_PRESENT 	0

#define CAM_EYE 		0
#define CAM_LASER 		1
#define CAM_ORCA 		2
#define CAM_JVC_KYF75 	3
#define CAM_DUMMY 		4

#define IMTYPE_MONO 	0
#define IMTYPE_RGB  	1

//////////////////// Micro-focus definitions ////////////////////

//#define ELECTRON_BEAM_SOURCE_PRESENT 	1

//////////////////// Multi-photon definitions ////////////////////

#define GAIN_BOARD_PRESENT 		0
#define DESCANNED_DETR_PRESENT 	0
#define DIRECT_DETR_PRESENT 	0
#define TRANS_DETR_PRESENT 		0

#define NO_DETECTOR     -1  //no detectors
#define TRANSMISSION    0   //transmission detectors
#define DIRECT          1   //external detectors
#define DESCANNED       2   //internal detectors

#define NONE        0       //currently selected mode of operation
#define TRANS       1
#define FLUOR       2
#define MP          3
  
/////////////////////////////////////////////////////////////////////////
typedef unsigned char byte;
#define Round  RoundRealToNearestInteger

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
