#include "gci_dcam_camera.h"
#include "gci_dcam_camera_lowlevel.h"
#include "gci_dcam_camera_settings.h"
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Statistics.h" 

#include "dcamprop.h" 

#include "string_utils.h"
#include "gci_utils.h"
#include <utility.h>
#include "toolbox.h"


void gci_dcam_error_code_to_string(unsigned long error, char *string)
{
	switch (error) {
	
		case 0x80000101:
		
			strcpy(string, "busy cannot process");
		
			break;
			
		case 0x80000102:
		
			strcpy(string, "abort process");
		
			break;
			
		case 0x80000103:
		
			strcpy(string, "not ready state");
		
			break;
			
		case 0x80000104:
		
			strcpy(string, "not stable state");
		
			break;
			
		case 0x80000106:
		
			strcpy(string, "timeout");
		
			break;
			
		case 0x80000107:
		
			strcpy(string, "not busy state");
		
			break;
            
        case 0x80000f02:
		
			strcpy(string, "feature not implemented yet");
		
			break;
	}

	return;
}


int gci_dcam_precapture(GciDCamCamera *gci_dcam_camera, int mode)
{
	GciCamera *camera = (GciCamera *) gci_dcam_camera;

	//Steps we must take prior to getting images
	//mode is ccCapture_Snap or ccCapture_Sequence

	PROFILE_START("gci_dcam_precapture");
	
	if (!dcam_precapture( gci_dcam_camera->_hCam, mode)) {

		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "Precapture routine failed");  

		PROFILE_STOP("gci_dcam_precapture");
		return CAMERA_ERROR;
	}

	PROFILE_STOP("gci_dcam_precapture");
	
	return CAMERA_SUCCESS;
}


int gci_dcam_wait_for_captured_frame(GciDCamCamera *gci_dcam_camera)
{
	int event = DCAM_EVENT_CYCLEEND, error;
	char buffer[255] = "";

	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	PROFILE_START("gci_dcam_wait_for_captured_frame");     
	//
	// Wait for acquisition complete to CCD or timeout
	if (!dcam_wait(gci_dcam_camera->_hCam, &event, gci_dcam_camera->_timeout, NULL) ) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);

		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);

		gci_dcam_error_code_to_string(error, buffer);  
	
		//send_error_text(camera, "dcam_wait failed: %s", string_buffer);
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s %s in gci_dcam_wait_for_captured_frame.", UIMODULE_GET_DESCRIPTION(camera), buffer);
		
		PROFILE_STOP("gci_dcam_wait_for_captured_frame");      
		
		return CAMERA_ERROR;
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", GCI_VOID_POINTER, gci_dcam_camera);   
	
	PROFILE_STOP("gci_dcam_wait_for_captured_frame");
	
	return CAMERA_SUCCESS;
}

int gci_dcam_capture(GciDCamCamera *gci_dcam_camera)
{
	int error;
	char buffer[255] = ""; 
	GciCamera *camera = (GciCamera *) gci_dcam_camera;
	TriggerMode trigger_mode;
	
	gci_camera_get_trigger_mode(camera, &trigger_mode); 
		
	//  Currently PreCapture triggers the shutter which sends a pulse to the camera, this must occur after dcam_capture
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PreCapture", GCI_VOID_POINTER, camera);

	PROFILE_START("gci_dcam_capture");    

	//dcam_getstatus( gci_dcam_camera->_hCam, &status);
	//printf("dcam_getstatus %d\n", status);
	
	if (!dcam_capture(gci_dcam_camera->_hCam)) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		gci_dcam_error_code_to_string(error, buffer);  
	
        //send_error_text(camera, "gci_gci_dcam_camera_get_image capture failed: %s", string_buffer);
        logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s %s in gci_dcam_capture.", UIMODULE_GET_DESCRIPTION(camera), buffer);
		
		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);

		PROFILE_STOP("gci_dcam_capture"); 
		
        return CAMERA_ERROR;
    }
		
	if (trigger_mode == CAMERA_EXTERNAL_TRIG)   
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "TriggerNow", GCI_VOID_POINTER, camera);     

	gci_dcam_wait_for_busy(gci_dcam_camera);

	PROFILE_STOP("gci_dcam_capture");     
	
    return CAMERA_SUCCESS;
}
    



//This is only supported by the DCam.
//It enables us to close the shutter sooner, i.e. before data transfer from the CCD
int gci_dcam_wait_for_frame_valid(GciDCamCamera *gci_dcam_camera)
{
	int event = DCAM_EVENT_VVALIDBEGIN, error;
	char buffer[255] = "";

	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	PROFILE_START("gci_dcam_wait_for_frame_valid");     
	
	// Wait for acquisition complete to CCD or timeout
	if (!dcam_wait(gci_dcam_camera->_hCam, &event, gci_dcam_camera->_timeout, NULL) ) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);

		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);
	
		gci_dcam_error_code_to_string(error, buffer);  
	
		//send_error_text(camera, "dcam_wait failed: %s", string_buffer);
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s %s in gci_dcam_wait_for_frame_valid.", UIMODULE_GET_DESCRIPTION(camera), buffer);
		
		PROFILE_STOP("gci_dcam_wait_for_frame_valid");     
		
		return CAMERA_ERROR;
	}
	
	PROFILE_STOP("gci_dcam_wait_for_frame_valid");
	
	return CAMERA_SUCCESS;
}


int gci_dcam_wait_for_frame_end(GciDCamCamera *gci_dcam_camera)
{
	int event = DCAM_EVENT_FRAMEEND, error;
	char buffer[255] = "";

	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	PROFILE_START("gci_dcam_wait_for_frame_end");   
	
	// Wait for acquisition complete and data transfered or timeout
	if (!dcam_wait(gci_dcam_camera->_hCam, &event, gci_dcam_camera->_timeout, NULL) ) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);

		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);
	
		//gci_dcam_error_code_to_string(error, string_buffer);  
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s %s in  gci_dcam_wait_for_frame_end.", UIMODULE_GET_DESCRIPTION(camera), buffer);
		
		//send_error_text(camera, "dcam_wait failed: %s", buffer);
		
		PROFILE_STOP("gci_dcam_wait_for_frame_end"); 

		return CAMERA_ERROR;
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", GCI_VOID_POINTER, gci_dcam_camera);   
	
	PROFILE_STOP("gci_dcam_wait_for_frame_end"); 
	
	return CAMERA_SUCCESS;
}



int gci_dcam_idle(GciDCamCamera *gci_dcam_camera)
{
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	PROFILE_START("gci_dcam_idle");   
	
	if (!dcam_idle(gci_dcam_camera->_hCam) ) {
		
		//send_error_text(camera, "dcam_idle routine failed");
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "dcam_idle routine failed");
		PROFILE_STOP("gci_dcam_idle"); 

		return CAMERA_ERROR;
	}

	PROFILE_STOP("gci_dcam_idle"); 
	
	return CAMERA_SUCCESS;
}


int gci_dcam_freeframe(GciDCamCamera *gci_dcam_camera)
{
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 
	
	PROFILE_START("gci_dcam_freeframe");   
	
	//Free resources
	if (!dcam_freeframe(gci_dcam_camera->_hCam)) {
	
		send_error_text(camera, "dcam_freeframe routine failed");
		PROFILE_STOP("gci_dcam_freeframe");
		return CAMERA_ERROR;
	}
	
	PROFILE_STOP("gci_dcam_freeframe");
	
	return CAMERA_SUCCESS;
}


int gci_dcam_wait_for_stable(GciDCamCamera *gci_dcam_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
    double t = Timer();
    
	PROFILE_START("gci_dcam_wait_for_stable");     
	
	while (status != DCAM_STATUS_STABLE) {
		
        dcam_getstatus( gci_dcam_camera->_hCam, &status);
        
        if (Timer() - t > 0.5)
            break;
    }
        
	PROFILE_STOP("gci_dcam_wait_for_stable");  
	
	return CAMERA_SUCCESS; 
}


int gci_dcam_wait_for_ready(GciDCamCamera *gci_dcam_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
    double t = Timer();
    
	PROFILE_START("gci_dcam_wait_for_ready");     
	
	while (status != DCAM_STATUS_READY) {
    
		dcam_getstatus( gci_dcam_camera->_hCam, &status);
	
        if (Timer() - t > 0.5)
            break;
    }
	
	PROFILE_STOP("gci_dcam_wait_for_ready"); 
	
	return CAMERA_SUCCESS; 
}


int gci_dcam_wait_for_busy(GciDCamCamera *gci_dcam_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
    double t = Timer();
    
	PROFILE_START("gci_dcam_wait_for_busy");    
	
	while (status != DCAM_STATUS_BUSY) {
    
		dcam_getstatus( gci_dcam_camera->_hCam, &status);
		
        if (Timer() - t > 0.5)
            break;
    }
	
	PROFILE_STOP("gci_dcam_wait_for_busy"); 
	
	return CAMERA_SUCCESS; 
}


int gci_dcam_allocframe(GciDCamCamera *gci_dcam_camera)
{
	GciCamera *camera = (GciCamera *) gci_dcam_camera;

	PROFILE_START("gci_dcam_allocframe");  
	
	if( !dcam_allocframe( gci_dcam_camera->_hCam, 1))
	{
		send_error_text(camera, "allocframe routine failed");
		PROFILE_STOP("gci_dcam_allocframe"); 
		return CAMERA_ERROR;
	}
		
	PROFILE_STOP("gci_dcam_allocframe"); 
	
	return CAMERA_SUCCESS; 
}


int gci_dcam_set_to_ready_mode(GciDCamCamera *gci_dcam_camera)
{
	PROFILE_START("gci_dcam_set_to_ready_mode");  
	
	gci_dcam_wait_for_stable(gci_dcam_camera);
	
	// Slow operation try to only call once for live mode
	gci_dcam_allocframe(gci_dcam_camera);
	
	gci_dcam_wait_for_ready(gci_dcam_camera);

	PROFILE_STOP("gci_dcam_set_to_ready_mode");
	
	return CAMERA_SUCCESS;
}


int gci_dcam_free_camera_resources(GciDCamCamera *gci_dcam_camera)
{
    int status;
	double t;
    GciCamera *camera = (GciCamera *) gci_dcam_camera;
    
	PROFILE_START("gci_dcam_free_camera_resources");     
	
    if (!dcam_getstatus(gci_dcam_camera->_hCam, &status)) {
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "dcam_getstatus routine failed"); 
		return CAMERA_ERROR;
	}
    
    if (status == DCAM_STATUS_BUSY) {

		if(gci_dcam_idle(gci_dcam_camera) == CAMERA_ERROR) {
			return CAMERA_ERROR;
        }
        
        if (gci_dcam_freeframe(gci_dcam_camera) == CAMERA_ERROR) {
			PROFILE_STOP("gci_dcam_free_camera_resources");  
        	return CAMERA_ERROR;	
		}
    }
    
    if (status == DCAM_STATUS_READY) {
        
		if (gci_dcam_freeframe(gci_dcam_camera) == CAMERA_ERROR) {
			PROFILE_STOP("gci_dcam_free_camera_resources");  
        	return CAMERA_ERROR;	
		}
    }
    
	if (!dcam_getstatus(gci_dcam_camera->_hCam, &status)) {
		
		send_error_text(camera, "dcam_getstatus routine failed");
		return CAMERA_ERROR;
	}

    t = Timer();
	
	//while (status != DCAM_STATUS_STABLE) {
	while ((status != DCAM_STATUS_STABLE) && (status != DCAM_STATUS_UNSTABLE)){  

		dcam_getstatus(gci_dcam_camera->_hCam, &status);

		if (Timer() - t > 3)
		{	
			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "dcam status failed to return to STABLE"); 
			PROFILE_STOP("gci_dcam_free_camera_resources");  
			return CAMERA_ERROR;
		}
    }
    
	PROFILE_STOP("gci_dcam_free_camera_resources");  
	
    return CAMERA_SUCCESS;
}


int DCamSetFeature(GciDCamCamera *gci_dcam_camera, int featureID, double value)
{
	DCAM_PARAM_FEATURE FeatureValue;
	
	//Used to set gain and brightness.
	
	//Set up structure with camera commands. Cribbed from the example programs. 
	FeatureValue.hdr.cbSize = sizeof(DCAM_PARAM_FEATURE);
	FeatureValue.hdr.id = DCAM_IDPARAM_FEATURE;
	FeatureValue.hdr.iFlag = 0;
	FeatureValue.hdr.oFlag = 0;
	FeatureValue.featureid = featureID;
	FeatureValue.flags = 0;
	FeatureValue.featurevalue = (float)value;

	//Send it
	if (!dcam_extended(gci_dcam_camera->_hCam, DCAM_IDMSG_SETPARAM, (LPVOID)&FeatureValue, sizeof(DCAM_PARAM_FEATURE))) 
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}


int DCamSetSubarray(GciDCamCamera *gci_dcam_camera, int ox, int oy, int gx, int gy)
{
	DCAM_PARAM_SUBARRAY Subarray;

	// Set up structure with camera commands. Cribbed from the example programs. 
	Subarray.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY);
	Subarray.hdr.id = DCAM_IDPARAM_SUBARRAY;
	Subarray.hdr.iFlag = 0;
	Subarray.hdr.oFlag = 0;
	Subarray.hpos = ox;
	Subarray.vpos = oy;
	Subarray.hsize = gx;
	Subarray.vsize = gy;

	if (!dcam_extended(gci_dcam_camera->_hCam, DCAM_IDMSG_SETPARAM, (LPVOID)&Subarray, sizeof(DCAM_PARAM_SUBARRAY))) 
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}


int DCamGetSubarray(GciDCamCamera *gci_dcam_camera, int *ox, int *oy, int *gx, int *gy)
{
	DCAM_PARAM_SUBARRAY Subarray;

	//Set up structure with camera commands. Cribbed from the example programs. 
	Subarray.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY);
	Subarray.hdr.id = DCAM_IDPARAM_SUBARRAY;
	Subarray.hdr.iFlag = 0;
	Subarray.hdr.oFlag = 0;

	//Get current subarray settings
	if (!dcam_extended(gci_dcam_camera->_hCam, DCAM_IDMSG_GETPARAM, (LPVOID)&Subarray, sizeof(DCAM_PARAM_SUBARRAY))) 
		return CAMERA_ERROR;
	
	*ox = Subarray.hpos;
	*oy = Subarray.vpos;
	*gx = Subarray.hsize;
	*gy = Subarray.vsize;
	
	return CAMERA_SUCCESS;
}


int DCamSetDataType(GciDCamCamera *gci_dcam_camera, int bits)
{
	int data_type, max_value;
	
	GciCamera *camera = (GciCamera *) gci_dcam_camera;
	
	//Camera can operate in 8 bit or 12 bit mode - 16 bit buffer used for 12 bit mode.
	//Camera must be idle with no attached buffers before changing data type
	if (gci_dcam_idle(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	if (gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	if (bits == BPP8) {
	
		data_type = ccDatatype_uint8;
		max_value = 255;
	}
	else if(bits == BPP12) {
	
		data_type = ccDatatype_uint16;
		max_value = 4095;
	}
    else if(bits == BPP14) {
    
        data_type = ccDatatype_uint16;
        max_value = 16383;
    }
    else if(bits == BPP16) {
    
        data_type = ccDatatype_uint16;
        max_value = 65535;
    }
	else {
	
        send_error_text(camera, "Data type not supported");
		return CAMERA_ERROR;
	}
	
	if(!dcam_setdatatype(gci_dcam_camera->_hCam, data_type)) {
	
		send_error_text(camera, "dcam_setdatatype routine failed");
		return CAMERA_ERROR;	
	}
	
	// Must be 8 for mono camera  
	if(bits == BPP8 && !dcam_setbitstype(gci_dcam_camera->_hCam, ccBitstype_index8)) {
	
		send_error_text(camera, "dcam_setbitstype routine failed");
		return CAMERA_ERROR;	
	} 
	
	if(!dcam_setbitsinputlutrange(gci_dcam_camera->_hCam, max_value, 0)) {
	
		send_error_text(camera, "dcam_setbitsinputlutrange routine failed");
		return CAMERA_ERROR;	
	}
	
	if(!dcam_setbitsoutputlutrange(gci_dcam_camera->_hCam, max_value, 0)) {
	
		send_error_text(camera, "dcam_setbitsoutputlutrange routine failed");
		return CAMERA_ERROR;	
	}
		
	gci_dcam_camera->_hammamatsu_data_type = data_type;
    gci_dcam_camera->_data_type = bits;
        
	return CAMERA_SUCCESS;
}


int DCamGetFeature(GciDCamCamera *gci_dcam_camera, int featureID, double *value)
{
	DCAM_PARAM_FEATURE FeatureValue;
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 
	
	//Set up structure with camera commands. Cribbed from the example programs. 
	FeatureValue.hdr.cbSize = sizeof(DCAM_PARAM_FEATURE);
	FeatureValue.hdr.id = DCAM_IDPARAM_FEATURE;
	FeatureValue.hdr.iFlag = dcamparam_feature_featureid | dcamparam_feature_featurevalue;
	FeatureValue.hdr.oFlag = 0;
	FeatureValue.featureid = featureID;
	FeatureValue.flags = 0;

	//Send it
	if (!dcam_extended(gci_dcam_camera->_hCam, DCAM_IDMSG_GETPARAM, (LPVOID)&FeatureValue, sizeof(DCAM_PARAM_FEATURE))) {
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_extended failed: %s", string_buffer);
		return CAMERA_ERROR;
	}
	
	if( ! (FeatureValue.hdr.oFlag & dcamparam_feature_featurevalue ) )
		return CAMERA_ERROR;

	*value = FeatureValue.featurevalue;
	
	return CAMERA_SUCCESS;
}

/*
int DCamQueryCurrentTempertureSupport(GciDCamCamera *gci_dcam_camera, int* supported)
{
	DCAM_PARAM_FEATURE_INQ	param;
	BOOL	support = FALSE;
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	memset( &param, 0, sizeof( param ));
	param.hdr.cbSize = sizeof( param );
	param.hdr.id	= DCAM_IDPARAM_FEATURE_INQ;
	param.hdr.iFlag = dcamparam_featureinq_featureid | dcamparam_featureinq_capflags;

	param.featureid = DCAM_IDFEATURE_TEMPERATURE;
	if( ! dcam_extended( gci_dcam_camera->_hCam, DCAM_IDMSG_GETPARAM, &param, sizeof( param )) ) {
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_extended failed: %s", string_buffer);
		return CAMERA_ERROR;
	}
	
	if( param.capflags & DCAM_FEATURE_FLAGS_ONOFF )
	{
		if( param.capflags & DCAM_FEATURE_FLAGS_READ_OUT )
		{
			support = TRUE;
		}
	}

	*supported = support;

	return CAMERA_SUCCESS;
}


int DCamPrintProperties(GciDCamCamera *gci_dcam_camera)
{
	int error;
	char string_buffer[255]; 
	int iprop=0, type, i, option, iDestProp;
	DCAM_PROPERTYATTR attr;
	char name[64], type_str[10], str[100], text[64], tempstr[50];
	double v;
	DCAM_PROPERTYVALUETEXT valuetext;
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 
	
	//What properties does this camera support?
	error = dcam_getnextpropertyid(gci_dcam_camera->_hCam, &iprop, DCAMPROP_OPTION_SUPPORT);
	if (error) {
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_extended failed: %s", string_buffer);
		return CAMERA_ERROR;
	}
	
	while (dcam_getnextpropertyid(gci_dcam_camera->_hCam, &iprop, DCAMPROP_OPTION_SUPPORT)) {
		if (iprop==0) break;
		
		memset(&attr, 0, sizeof(attr));
		attr.iProp = iprop;
		dcam_getpropertyattr(gci_dcam_camera->_hCam, &attr);
		dcam_getpropertyname(gci_dcam_camera->_hCam, iprop, name, sizeof(name));
		
		type = attr.attribute & DCAMPROP_TYPE_MASK;
		if (type == DCAMPROP_TYPE_MODE)
			strcpy(type_str, "mode");
		else if (type == DCAMPROP_TYPE_LONG)
			strcpy(type_str, "long");
		else if (type == DCAMPROP_TYPE_REAL)
			strcpy(type_str, "real");
			
		sprintf(str, "Property %s, type %s", name, type_str);

		if (attr.attribute & DCAMPROP_ATTR_HASRANGE) {
			sprintf(tempstr, ", min %f, max %f", attr.valuemin, attr.valuemax);
			strcat(str, tempstr);
			for (v = attr.valuemin; v <= attr.valuemax; v++) {
				memset(&valuetext, 0, sizeof(valuetext));
				valuetext.iProp = iprop;
				valuetext.value = v;
				valuetext.text = text;
				valuetext.textbytes = sizeof(text);
				
				if (dcam_getpropertyvaluetext(gci_dcam_camera->_hCam, &valuetext)) {
					sprintf(tempstr, ", val %f:%s", v, text);
					strcat(str, tempstr);
				}
					
				if (!dcam_querypropertyvalue(gci_dcam_camera->_hCam, iprop, &v, DCAMPROP_OPTION_NEXT))
					break;
			}
		}
		
		strcat(str, "\n");
		
		//get influential properties
		for (i=1;;i++) {
			option = DCAMPROP_OPTION_INFLUENCE;
			option |= (DCAMPROP_OPTION_NEXT * i);
			iDestProp = iprop;
			if (!dcam_getnextpropertyid(gci_dcam_camera->_hCam, &iDestProp, option))
				break;
				
			dcam_getpropertyname(gci_dcam_camera->_hCam, iDestProp, name, sizeof(name));
			sprintf(tempstr, "\tInfluence %d:%s\n", i, name);
			strcat(str, tempstr);
		}
		
		printf(str);
		
	}
	
	return CAMERA_SUCCESS;
}
*/

int VOID_DCAM_CAMERA_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (GciDCamCamera *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (GciDCamCamera *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int DCamPowerUp(GciCamera* camera)
{
	int no_of_cameras;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	double t1;
	char model[100];
	char buffer[500] = "";
	
	//If this is part of a larger application the camera may already have been powered up
	//If this is a standalone program we power it up if it is off

    #ifdef POWER_VIA_I2C 
     	
	    if ( GCI_Power_GetCameraStatus() ) {
        
    	  	if( GCI_PowerCameraOn() != 0) {
    	  	
    	  		send_error_text(camera, "Can not power on camera");
        		return CAMERA_ERROR;   	// Some I2C problem	
        	}
        }
        	
    #endif
      
		
    t1 = Timer();
    while (!dcam_init(gci_dcam_camera->_ghInstance, &no_of_cameras, NULL)) {

		//error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		//gci_dcam_error_code_to_string(error, buffer);  

		if ((Timer() - t1) > 3.0) {
			return CAMERA_ERROR;
		}
	}
  
    if (!dcam_open(&(gci_dcam_camera->_hCam), gci_dcam_camera->_index, NULL)) {
    
		send_error_text(camera, "Failed to open camera");
		return CAMERA_ERROR;
	}

	//Is this really an DCam?
	if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_MODEL, model, 64)) {
		send_error_text(camera, "Failed to get model info.");
		return CAMERA_ERROR;
	}	

	if (gci_dcam_precapture(gci_dcam_camera, ccCapture_Snap) == CAMERA_ERROR) 
			return CAMERA_ERROR;
	
	if( gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

  	return CAMERA_SUCCESS;
}

FIBITMAP * DCamGetImage(GciCamera* camera, const Rect *rect)
{
	int frame=-1, nFrames=0, rowbytes, real_bpp = 8;
	void *buf = NULL;
	SIZE size;
	FREE_IMAGE_TYPE data_type;
	FIBITMAP *dib;
	StatisticReport report;

	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	
	if(camera->_aquire_mode == SNAP) {
	
		gci_dcam_idle(gci_dcam_camera);	
		gci_dcam_capture(gci_dcam_camera);
		gci_dcam_wait_for_captured_frame(gci_dcam_camera);
		//gci_dcam_wait_for_frame_end (gci_dcam_camera);          
	}
	else if(camera->_aquire_mode == LIVE) {
	
		// Live Mode
		gci_dcam_wait_for_frame_end (gci_dcam_camera);
		
	}
	else if(camera->_aquire_mode == SNAP_SEQUENCE) {
	
		gci_dcam_idle(gci_dcam_camera);	
		gci_dcam_capture(gci_dcam_camera);
		gci_dcam_wait_for_frame_end (gci_dcam_camera);   
	}
	else {
	
		send_error_text(camera, "%s", "No such aquire mode!");
		return NULL;
	}
	
	// Transfer the pixels to a buffer.
	if (!dcam_gettransferinfo(gci_dcam_camera->_hCam, &frame, &nFrames)) {
	
		send_error_text(camera, "dcam_gettransferinfo failed");
		return NULL;
	}
		
	if (nFrames < 1)
		return NULL;
	
	if (!dcam_lockdata(gci_dcam_camera->_hCam, &buf, &rowbytes, frame)) {
		
		send_error_text(camera, "lockbits failed");
		return NULL;
	}
	
	if (!dcam_getdatasize(gci_dcam_camera->_hCam, &size)) {
	
		send_error_text(camera, "getdatasize routine failed");
		return NULL;
	}
	
    switch(camera->_data_mode)
    {
        case BPP8:
        {
            data_type = FIT_BITMAP;
            break;
        }
        
        case BPP12:
        case BPP14:
        case BPP16:
        {
            data_type = FIT_UINT16;
			real_bpp = 16;
            break;
        }
        
        default:
        {
            send_error_text(camera, "invalid bpp");
            return NULL;	
        }
    }
    
	dib = FIA_LoadGreyScaleFIBFromArrayData (buf, real_bpp, size.cx, size.cy, data_type, 1, COLOUR_ORDER_RGB);
	
	dcam_unlockdata( gci_dcam_camera->_hCam);

	if(gci_camera_feature_enabled(camera, CAMERA_FEATURE_SENSITIVITY))
	{
		// If the dcam camera has sensitivity (EM Gain) then we need to protect the camera
		// Here we look at the amount of overloaded pixels in the image
		
		FIA_StatisticReport(dib, &report);

		if(report.percentage_overloaded >= 0.75) {
			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "Camera Image is 75%% saturated. This could cause camera damage");  
		}
		else if(report.percentage_overloaded >= 0.5) {
			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "Camera Image is 50%% saturated. This could cause camera damage");  
		}
		else if(report.percentage_overloaded >= 0.25) {
			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "Camera Image is 25%% saturated. This could cause camera damage");  
		}

	}

	return dib;
}
