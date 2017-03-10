// xml_utils

#include "string_utils.h"
#include "gci_utils.h"
#include "xml_utils.h"

// For Loading **************************************************************

int openXmlRoot (char filename[], char expectedRoot[], CVIXMLDocument *xmlDocOut, CVIXMLElement *xmlRootOut)
{
	CVIXMLElement root=-1;
	CVIXMLDocument xmlDoc=-1;
	char str[64];
    CVIXMLStatus xmlError;

	// check file is there
    if (!FileExists (filename, 0)) return -2;
    
	// load xml doc and check root element
	xmlError = CVIXMLLoadDocument (filename, &xmlDoc); 
	if (xmlError)
	{
		char buffer [256];
		CVIXMLGetErrorString (xmlError, buffer, 256);
		GCI_MessagePopup("XML  File Error", buffer);
		goto Error;
	}
	if (CVIXMLGetRootElement (xmlDoc, &root))   goto Error;
	CVIXMLGetElementTag (root, str);
	if (stricmp(str, expectedRoot)!=0)
	{
//		GCI_ErrorPopup("Error", "This file is not the correct type of XML file.");
		goto Error;
	}

	if (xmlDocOut!=NULL)  *xmlDocOut  = xmlDoc;
	if (xmlRootOut!=NULL) *xmlRootOut = root;
	
	return 0;
	
Error:
	if (root >= 0)   CVIXMLDiscardElement(root);
	if (xmlDoc >= 0) CVIXMLDiscardDocument(xmlDoc);

	return -1;
}

void closeXmlRoot (CVIXMLDocument xmlDoc, CVIXMLElement root)
{
	CVIXMLDiscardElement(root);   
	CVIXMLDiscardDocument(xmlDoc);
}	

int getXmlSettingInt (CVIXMLElement root, const char *name, int *val)
{
	CVIXMLElement el=0;
	char str[64];
	
	if (CVIXMLGetChildElementByTag (root, name, &el)) goto Error;
	if (el==0) goto Error; 
	if (CVIXMLGetElementValue (el, str)) goto Error;
	CVIXMLDiscardElement(el);
	*val = atoi(str);
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

int getXmlSettingDbl (CVIXMLElement root, const char *name, double *val)
{
	CVIXMLElement el=0;
	char str[64];
	
	if (CVIXMLGetChildElementByTag (root, name, &el)) goto Error;
	if (el==0) goto Error; 
	if (CVIXMLGetElementValue (el, str)) goto Error;
	CVIXMLDiscardElement(el);
	*val = atof(str);
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

int getXmlSettingStr (CVIXMLElement root, const char *name, char *val, int maxChars)
{
	CVIXMLElement el=0;
	char str[500];
	
	if (CVIXMLGetChildElementByTag (root, name, &el)) goto Error;
	if (el==0) goto Error; 
	if (CVIXMLGetElementValue (el, str)) goto Error;
	CVIXMLDiscardElement(el);
	strncpy(val, str, maxChars);
	val[maxChars-1]=0;  // make sure of ascii null byte
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

int getXmlDimensionFromElement (CVIXMLElement el, double *val)
{   // read and decode a length and return result in metres
	CVIXMLAttribute att=0;
	char str[64];

	if (CVIXMLGetElementValue (el, str)) goto Error;
	*val = atof(str);

	CVIXMLGetAttributeByName (el, "unit", &att);
	if (att>0)   // there is a "unit" attribute
	{
		CVIXMLGetAttributeValue (att, str);
		switch (str[0])
		{
			case 'u':
				*val *= 1e-6;
				break;
			case 'm':
				if (str[1]=='m') *val *= 1e-3; // if 'mm'
				break;						   // if just 'm', leave as is
			case 'c':
				*val *= 1e-2;
				break;
			case 'i':
				if (str[1]=='n') *val *= 25.4e-3; // if 'in' for inches
				break;
		}
	}
	CVIXMLDiscardAttribute (att);

	Error:
	return -1;
}

int getXmlDimension (CVIXMLElement root, const char *name, double *val)
{   // read and decode a length and return result in metres
	CVIXMLElement el=0;
	
	if (CVIXMLGetChildElementByTag (root, name, &el)) goto Error;
	if (el==0) goto Error; 

	if (getXmlDimensionFromElement (el, val)<0) goto Error;

	CVIXMLDiscardElement(el);

	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

// For Saving **************************************************************

int newXmlSettingDbl (CVIXMLElement root, const char *name, double val)
{
	CVIXMLElement el=0;
	char buffer[64];

	if (CVIXMLNewElement      (root, -1, name, &el)) goto Error;

	// Do not use ftoa from string_utils as this rounds to 2DP!
//	if (CVIXMLSetElementValue (el, ftoa(val, buffer))) goto Error;
	sprintf(buffer, "%f", val);
	if (CVIXMLSetElementValue (el, buffer)) goto Error;

	CVIXMLDiscardElement(el);
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

int newXmlSettingInt (CVIXMLElement root, const char *name, int val)
{
	CVIXMLElement el=0;
	char buffer[64];

	if (CVIXMLNewElement      (root, -1, name, &el)) goto Error;
	if (CVIXMLSetElementValue (el, str_itoa(val, buffer))) goto Error;
	CVIXMLDiscardElement(el);
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}

int newXmlSettingStr (CVIXMLElement root, const char *name, const char *val)
{
	CVIXMLElement el=0;

	if (CVIXMLNewElement      (root, -1, name, &el)) goto Error;
	if (CVIXMLSetElementValue (el, val)) goto Error;
	CVIXMLDiscardElement(el);
	return 0;

	Error:
	if (el > 0) CVIXMLDiscardElement(el);
	return -1;
}
