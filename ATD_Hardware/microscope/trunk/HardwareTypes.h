#ifndef __HARDWARE_TYPES__
#define __HARDWARE_TYPES__

// Defined so the lower level microscope implementations can use use them without having
// to reference microscope.h and other headers which may change alot and
// cause big recompiles.
typedef struct _HardwareDevice HardwareDevice;
typedef struct _OpticalCalibrationDevice OpticalCalibrationDevice;
typedef struct _GciCamera GciCamera;
typedef struct _TemperatureMonitor TemperatureMonitor;
typedef struct _OpticalLift OpticalLift;
typedef struct _Analyzer Analyzer;
typedef struct _ObjectiveManager ObjectiveManager;
typedef struct _FluoCubeManager FluoCubeManager;
typedef struct _FilterSetCollection FilterSetCollection;
typedef struct _OpticalPathManager OpticalPathManager;
typedef struct _CondenserManager CondenserManager;
typedef struct _Lamp Lamp;
typedef struct _Shutter Shutter;
typedef struct _SingleRangeHardwareDevice SingleRangeHardwareDevice;
typedef struct _XYStage XYStage;
typedef struct _StagePlateModule StagePlateModule;
typedef struct _precisExcite precisExcite;
typedef struct _Intensilight Intensilight;
typedef struct _Spc Spc;
typedef struct _Scanner Scanner;
typedef struct _AutofocusCtrl AutofocusCtrl;
typedef struct _Z_Drive Z_Drive;
typedef struct _CoarseZDrive CoarseZDrive;
typedef struct _PowerSwitch PowerSwitch;
typedef struct _stage_scan stage_scan;
typedef struct _Microscope Microscope;  
typedef struct _Laser Laser;
typedef struct _MotorMike MotorMike;
typedef struct _well_plate_definer well_plate_definer;
typedef struct _PrototypeAutoFocus PrototypeAutoFocus;
typedef struct _BatchCounterA1 BatchCounterA1;
typedef struct _BeamScanner BeamScanner;
typedef struct _PmtSet PmtSet;
typedef struct _precisExcite precisExcite;
typedef struct _LaserPowerMonitor LaserPowerMonitor;
typedef struct _RobotInterface RobotInterface;
#endif
