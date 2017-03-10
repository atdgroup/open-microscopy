#ifndef _GCI_CVIREGITRY_
#define _GCI_CVIREGITRY_

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

// Easy Registry commands for CVI, Paul Barber, Feb 2002, Aug 2002
int RegWriteLong (unsigned int rootKey, const char subkeyName[], const char valueName[], long dataValue);
int RegReadLong (unsigned int rootKey, const char subkeyName[], const char valueName[], long *uLongData);
int RegistrySavePanelSizePosition (unsigned int userRootKey, const char* userSubKeyName, int panel);
int RegistryReadPanelSizePosition (unsigned int userRootKey, const char* userSubKeyName, int panel);
int RegistrySavePanelPosition (unsigned int userRootKey, const char* userSubKeyName, int panel);
int RegistryReadPanelPosition (unsigned int userRootKey, const char* userSubKeyName, int panel);
int RegistrySavePanelVisibility (unsigned int userRootKey, const char* userSubKeyName, int panel);
int checkRegistryValueForPanelAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int attribute);
int checkRegistryValueForMenuAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int menuItem, int attribute);
int checkRegistryValueForCtrlAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute);
int checkRegistryValueForCtrlAttribDouble (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute);
int checkRegistryValueForCtrlAttribFloat (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute);
int checkRegistryValueForCtrlAttribString (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute);
int checkRegistryValueForInt   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   int *value);
int checkRegistryValueForString           (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   char *string);
int checkRegistryValueForDouble   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   double *value);
int checkRegistryValueForFloat   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   float *value);

#endif
