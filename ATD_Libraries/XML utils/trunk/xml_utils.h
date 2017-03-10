#ifndef __GCI_XML_UTILS_
#define __GCI_XML_UTILS_

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "cvixml.h"

int openXmlRoot (char filename[], char expectedRoot[], CVIXMLDocument *xmlDocOut, CVIXMLElement *xmlRootOut);
void closeXmlRoot (CVIXMLDocument xmlDoc, CVIXMLElement root);
int getXmlSettingInt (CVIXMLElement root, const char *name, int *val);
int getXmlSettingDbl (CVIXMLElement root, const char *name, double *val);
int getXmlSettingStr (CVIXMLElement root, const char *name, char *val, int maxChars);
int getXmlDimension (CVIXMLElement root, const char *name, double *val);
int getXmlDimensionFromElement (CVIXMLElement el, double *val);

int newXmlSettingDbl (CVIXMLElement root, const char *name, double val);
int newXmlSettingInt (CVIXMLElement root, const char *name, int val);
int newXmlSettingStr (CVIXMLElement root, const char *name, const char *val);

#endif
