#include <cvidef.h>

#ifdef __cplusplus
    extern "C" {
#endif

	//int			m_DLLResultat;			// DLL-Zugriff ok
	//int		LS_GetDLLLoadResult() {return m_DLLResultat;}; // Funktion, ob Laden der DLL erfolgreich war.
	// sollte Wert 0 zurückgeben

	// Funktionen der Dll
        int __stdcall LS_Connect (void);
        int __stdcall LS_ConnectSimple (int lAnInterfaceType, char *pcAComName, int lABaudRate, BOOL bAShowProt);
        //int ConnectEx (TLS_ControlInitPar *pAControlInitPar);
        int __stdcall LS_Disconnect (void);
        int __stdcall LS_LoadConfig (char *pcFileName);
        int __stdcall LS_SaveConfig (char *pcFileName);
        int __stdcall LS_MoveAbs (double dX, double dY, double dZ, double dA, BOOL bWait);
        int __stdcall LS_MoveRel (double dX, double dY, double dZ, double dA, BOOL bWait);
        int __stdcall LS_Calibrate (void);
        int __stdcall LS_RMeasure (void);
        int __stdcall LS_SetPitch (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetPos (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetActiveAxes (int lFlags);
        int __stdcall LS_SetVel (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetAccel (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetReduction (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetDelay (int lDelay);
        int __stdcall LS_SetSwitchPolarity (int lXP, int lYP, int lZP, int lAP);
        int __stdcall LS_SetSwitchActive (int lXA, int lYA, int lZA, int lAA);
        int __stdcall LS_SetJoystickOn (BOOL bPositionCount, BOOL bEncoder);
        int __stdcall LS_SetJoystickOff (void);
        int __stdcall LS_SoftwareReset (void);
        int __stdcall LS_SetDigitalOutput (int lIndex, BOOL bValue);
        int __stdcall LS_GetDigitalInputs (int *plValue);
        int __stdcall LS_SetAnalogOutput (int lIndex, int lValue);
        int __stdcall LS_GetAnalogInput (int lIndex, int *plValue);
        int __stdcall LS_SetLimit (int lAxis, double dMinRange, double dMaxRange);
        int __stdcall LS_SetLimitControl (int lAxis, BOOL bActive);
        int __stdcall LS_GetPos (double *pdX, double *pdY, double *pdZ, double *pdA);
        int __stdcall LS_GetStatus (char *pcStat, int lMaxLen);
        int __stdcall LS_GetEncoderMask (int *plFlags);
        int __stdcall LS_StopAxes (void);
        int __stdcall LS_SetAbortFlag (void);
        int __stdcall LS_MoveRelSingleAxis (int lAxis, double dValue, BOOL bWait);
        int __stdcall LS_MoveAbsSingleAxis (int lAxis, double dValue, BOOL bWait);
        int __stdcall LS_SetControlPars (void);
        int __stdcall LS_SetMotorCurrent (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetVelSingleAxis (int lAxis, double dVel);
        int __stdcall LS_SetAccelSingleAxis (int lAxis, double dAccel);
        int __stdcall LS_CalibrateEx (int lFlags);
        int __stdcall LS_RMeasureEx (int lFlags);
        int __stdcall LS_SetShowProt (BOOL bShowProt);
        int __stdcall LS_GetAnalogInputs2 (int *plPT100, int *plMV, int *plV24);
        int __stdcall LS_SendString (char *pcStr, char *pcRet, int lMaxLen, BOOL bReadLine, int lTimeOut);
        int __stdcall LS_SetSpeedPoti (BOOL bSpeedPoti);
        int __stdcall LS_SetTVRMode (int lXT, int lYT, int lZT, int lAT);
        int __stdcall LS_SetAutoStatus (int lValue);
        int __stdcall LS_GetStatusAxis (char *pcStatusAxisStr, int lMaxLen);
        int __stdcall LS_SetDigIO_Off (int lIndex);
        int __stdcall LS_SetDigIO_Polarity (int lIndex, BOOL bHigh);
        int __stdcall LS_SetDigIO_EmergencyStop (int lIndex);
        int __stdcall LS_SetDigIO_Distance (int lIndex, BOOL bFkt, double dDist, int lAxis);
        int __stdcall LS_SetDimensions (int lXD, int lYD, int lZD, int lAD);
        int __stdcall LS_MoveRelShort (void);
        int __stdcall LS_SetEncoderPeriod (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetJoystickDir (int lXD, int lYD, int lZD, int lAD);
        int __stdcall LS_SetEncoderMask (int lValue);
        int __stdcall LS_SetGear (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetHandWheelOn (BOOL bPositionCount, BOOL bEncoder);
        int __stdcall LS_SetHandWheelOff (void);
        int __stdcall LS_SetFactorTVR (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetTargetWindow (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetController (int lXC, int lYC, int lZC, int lAC);
        int __stdcall LS_SetControllerCall (int lCtrCall);
        int __stdcall LS_SetControllerSteps (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetControllerFactor (double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetControllerTWDelay (int lCtrTWDelay);
        int __stdcall LS_SetEncoderRefSignal (int lXR, int lYR, int lZR, int lAR);
        int __stdcall LS_SetEncoderPosition (BOOL bValue);
        int __stdcall LS_GetVersionStr (char *pcVers, int lMaxLen);
        int __stdcall LS_GetError (int *plErrorCode);
        int __stdcall LS_GetPosSingleAxis (int lAxis, double *pdPos);
        int __stdcall LS_SetDistance (double dX, double dY, double dZ, double dA);
        int __stdcall LS_GetPosEx (double *pdX, double *pdY, double *pdZ, double *pdR, BOOL bEncoder);
        int __stdcall LS_SetShowCmdList (BOOL bShowCmdList);
        int __stdcall LS_SetWriteLogText (BOOL bAWriteLogText);
        int __stdcall LS_SetControllerTimeout (int lACtrTimeout);
        int __stdcall LS_SetEncoderActive (int lFlags);
        int __stdcall LS_GetSnapshotCount (int *plSnsCount);
        int __stdcall LS_GetSnapshotPos (double *pdX, double *pdY, double *pdZ, double *pdA);
        int __stdcall LS_SetCorrTblOff (void);
        int __stdcall LS_SetCorrTblOn (char *pcAFileName);
        int __stdcall LS_SetFactorMode (BOOL bAFactorMode, double dX, double dY, double dZ, double dA);
        int __stdcall LS_SetSnapshot (BOOL bASnapshot);
        int __stdcall LS_SetSnapshotPar (BOOL bHigh, BOOL bAutoMode);
        int __stdcall LS_SetTrigger (BOOL bATrigger);
        int __stdcall LS_SetTriggerPar (int lAxis, int lMode, int LSignal, double dDistance);
        int __stdcall LS_SetLanguage (char *pcPLN);
        int __stdcall LS_GetSwitches (int *plFlags);
        int __stdcall LS_GetSerialNr (char *pcSerialNr, int lMaxLen);
        int __stdcall LS_SetCalibOffset (double dX, double dY, double dZ, double dR);
        int __stdcall LS_SetRMOffset (double dX, double dY, double dZ, double dR);
        int __stdcall LS_GetSnapshotPosArray (int lIndex, double *pdX, double *pdY, double *pdZ, double *pdR);
        int __stdcall LS_SetAxisDirection (int lXD, int lYD, int lZD, int lAD);

#ifdef __cplusplus
    }
#endif
