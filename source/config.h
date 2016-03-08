//-----------------------------------------------------------------------------
//  IFCB Project
//	config.h
//	Martin Cooper
//
//	Globals etc related to the instrument, not Windows
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//	HARDWARE CONFIGURATION OPTIONS
//-----------------------------------------------------------------------------
// uncomment only one of these options
#define IFCB
//#define IFCB8_BENCHTOP

//-----------------------------------------------------------------------------
#ifdef IFCB
#define WHOI_CUSTOM		// fluidics
#define DAQ_MCC			// DAQ hardware = Martin's Housekeeping board and analog board
#endif

#ifdef IFCB8_BENCHTOP
#define KLOEHN			// fluidics
#define NIUSB6009		// DAQ hardware
#endif

//-----------------------------------------------------------------------------
#define uint8 unsigned char
#define uint16 unsigned short

//-----------------------------------------------------------------------------
//	Variables here are stored in file ifcb.cfg
//#define STRING_PARAM_LENGTH			120
#define STRING_PARAM_LENGTH			240

extern char			fluidicsPort[];			// kloehn pump COM serial port
extern char			daqPort[];				// DAQ_MCC serial port
extern char			dataPath[];				// path for saved files
extern char			fileComment[];			// comment for header file
extern int			FreqUpdateInterval;		// PZT frequency updating
extern int			PZTvolts;				// for acoustic focusing tests
//extern int			PZThz;					// for acoustic focusing tests
extern int			PZTfrequency;			// for acoustic focusing tests
extern double		Tadjm;			// for acoustic focusing temperature adjust equation
extern double		Tadjb;			// for acoustic focusing temperature adjust equation
extern bool			TemperatureAdjust;		// for acoustic focusing tests
extern bool			DoFreqScan;				// for acoustic focusing tests
extern char			voltsPZT[];				// for acoustic focusing tests
extern int			HzPZT[];					// for acoustic focusing tests
extern double		Hz;						// for acoustic focusing tests
extern double			StartFreq;						// for acoustic focusing 
extern double			EndFreq;						// for acoustic focusing 
extern double			FreqStep;						// for acoustic focusing 
extern double			NewFreq;						// for acoustic focusing 
extern double		StepDuration;						// for acoustic focusing 
extern double		mlPerMin;						// for acoustic focusing 
extern double		minPerSyr;						// for acoustic focusing 
extern double		minPerStep;						// for acoustic focusing 
extern double		freqstepsPerSyr;						// for acoustic focusing 
extern int		syrPerCycle;						// for acoustic focusing 
extern int		stepCount;						// for acoustic focusing 
extern int		freqstepsPerCycle;						// for acoustic focusing 
extern int			syringeOffset;			// the counts between syringe bottom and where opto is
extern int			imagerID;				// the ID number of this imager (e.g. 2 for IFCB2)
extern int			binarizeThresh;			// binarize threshold for blob analysis = % of histogram max
extern int			syringeSize;			// syringe size (ml)
extern int			blobXGrow, blobYGrow;	// amount to grow blobs before combining them
extern int			blobJoinGap;			// any blobs with a gap between them less than this get joined together
extern int			resizeFactor;			// image size reduction factor
extern unsigned int	minimumBlobArea;		// minimum area of blobs
extern double		highVoltagePMTA, highVoltagePMTB, highVoltagePMTC;
extern double		highVoltagePMTA2, highVoltagePMTB2, highVoltagePMTC2;
extern double		flashVoltage, flashVoltage2;
extern bool			appDebug;
extern bool			viewImages;
extern bool			blobAnalysis;
//extern bool			discreteSampleIntake;
extern bool			backflushWithSample;
extern bool			primeSampleTube;
extern bool			replaceFromSec;
extern bool			stainSample;
extern bool			StainAuto;
extern bool			StainRinseMixingChamber;
extern bool			LockFilterSlider;
extern bool			outputFiles;
extern bool			fluidicsVerbose;
extern bool			fluidsActive;
extern int	 		fanThresh;				// used by DAQ_MCC only
extern int 			fanHysteresis;			// used by DAQ_MCC only
extern int			humidThresh;			// used by DAQ_MCC only
extern int 			humidHysteresis;		// used by DAQ_MCC only
extern int 			valveDelay;				// used by DAQ_MCC only
extern int 			flashDelay;				// used by DAQ_MCC only
extern int	 		flashPulseLength;		// used by DAQ_MCC only
extern int 			cameraPulseLength;		// used by DAQ_MCC only
extern double		trigThreshA;			// used by DAQ_MCC only
extern double		trigThreshB;			// used by DAQ_MCC only
extern double		trigThreshC;			// used by DAQ_MCC only
extern double		trigThreshD;			// used by DAQ_MCC only
extern double		trigThreshA2;			// used by DAQ_MCC only
extern double		trigThreshB2;			// used by DAQ_MCC only
extern double		trigThreshC2;			// used by DAQ_MCC only
extern double		trigThreshD2;			// used by DAQ_MCC only
extern int			integratorSettleTime;	// used by DAQ_MCC only
extern int			triggerPulseMask;		// used by DAQ_MCC only
extern int			triggerPulseMask2;		// used by DAQ_MCC only
extern int			adcPga;					// used by DAQ_MCC only
extern int			adcPga2;				// used by DAQ_MCC only
extern int			adcDrate;					// used by DAQ_MCC only
extern int			triggerTimeout;			// used by DAQ_MCC only
extern int			pulseStretchTime;		// used by DAQ_MCC only
extern bool			pumpDrive;				// used by DAQ_MCC only
extern double		pumpVolts;				// used by DAQ_MCC only
extern int			primValvePort;			// define valve port for primary intake
extern int			secValvePort;			// define valve port for secondary intake
extern double		sampleVolume;			// the volume of sample to draw into the syringe, in milliliters
extern double		sampleVolume2skip;		// volume to leave behind in the syringe, to prevent degassed air in flow cell
extern double		sampleVolume2;			// the volume of sample to draw into the syringe, in milliliters - for alt runs
extern double		sampleVolume2skipALT;	// [For ALT config] volume to leave behind in the syringe, to prevent degassed air in flow cell
extern double		syringeVolume;			// the volume to draw into the syringe, - current run
extern double		syringeVolume2skip;
extern double		fullSyringeVolume;			//physical size of syringe in mL
extern double		beadSampleTotalVolume;		//total volume of beads plus SW to mix with them for internal beads sampling
extern int			syr_sampling_speed;
extern int			base_syr_sampling_speed;
extern int			altPortNumber;
extern int			normPortNumber;
extern double		beadsVolume;			// the volume of sample to draw into the syringe, - beads run
extern int			samplePortNumber;
extern int			currentValvePosition;
extern int			desiredValvePosition;
//extern int			valveDirection;
extern int			smallFocusStep;
extern int			largeFocusStep;
extern int			smallLaserStep;
extern int			largeLaserStep;
extern bool			debubbleWithSample;
extern bool			debubbleWithSample2;
extern int			altInterval;			// number of syringes between alternate setup runs
extern int			maxPulseStretch;		// maximum pulse length
extern bool			hasExpertTab;				// true if the expert tab is visible
extern int			tcpPort;
extern double		humidityThreshold;
extern int			pumpState;
extern int			laserState;
extern double		runFastFactor;
extern unsigned long	acqStartTimeout;		// timeout for first camera trigger
extern int			fill2bypassFilterState;
extern int			fill3newSheathState;
extern bool			hasCamera;				// set to false when running without camera
extern bool			AcousticFocusing;				// on ALT samples
//extern bool			TemperatureAdjust;				// on ALT samples

//-----------------------------------------------------------------------------
//	DATA STRUCTURES AND GLOBAL (NON-HARDWARE) SETTINGS
//-----------------------------------------------------------------------------

#define ROI_ARRAY_SIZE_INC		200000000					// bytes - should be at least the size of the camera image (1360 * 1024 = 1392640)
#define IFCB_ARRAY_SIZE_INC		ROI_ARRAY_SIZE_INC / 100	// records - should be ROI_ARRAY_SIZE_INC * sizeof(DataStruct) / (average roi size * average # rois)

// output data structure
typedef struct {
	unsigned int	trigger;						// number of the ROI in the file: first one is 0
	float			ADC_time;						// timestamp: absolute time
#ifdef DAQ_MCC
	float			integA, integB, integC, integD;	// integrated values for each channel
	float			peakA, peakB, peakC, peakD;		// peak PMT values for each channel
#else
	float			integAlo, integAhi, integClo, integChi;	// four integrated values for each channel
#endif
	float			time_of_flight;					// duration of comparator "on" (width, in us): the CHB LO integrated signal on AD CH8
	unsigned long	grabtimestart, grabtimeend;		//  using GetTickCount()
	long			LLHxloc, LLHyloc;				// lower left hand coords of ROI in pixels
	long			roiSizeX;						// pixel size of ROI x and Y
	long			roiSizeY;
	unsigned long	start_byte;						// the start byte of this ROI in the ROI file: first byte is 1 (ONE) - bad!
	// this above issue will cause some complications in merging C code with BASIC code to get indices right
	float			comparator_out;					// whatever is on AD CH9
	unsigned long	StartPoint;						// for data record from chase card if not using a fixed buffer length
	unsigned long	SignalLength;					// for digitizer
	unsigned int	status;							// status flags
	double			runTime;						// Analog board timer, in s (resolution of 2.17 us)
	double			inhibitTime;					// Analog board timer, in s (resolution of 2.17 us)
	double			temperature;					// used by PZT system
	int				frequency;						// used by PZT system
	double			Tadjm;						// used by PZT system
	double			Tadjb;						// used by PZT system
} DataStruct;

extern DataStruct		*ifcb_data;
extern unsigned long	dataStructSize;				// the capacity of the ifcb_data array, in records

// status flags
#define FLAG_NONE				0x00l
#define FLAG_ROI_REALLOC		0x01l				// ROI array was expanded
#define FLAG_IFCB_REALLOC		0x02l				// ifcb_data array was expanded
#define FLAG_DIGITIZE_FAIL		0x04l				// digitizer failed
#define	FLAG_FILE_WRITE			0x08l				// data files were written to disc
#define	FLAG_REARM_FORCED		0x10l				// the rearm timer was fired before this trigger

extern unsigned long	triggerTickCount;			// the tick count when the camera callback occurred
#ifdef DAQ_MCC
extern double gRunTime, gInhibitTime;
#endif
extern int			syringesDone;

// globals used throughout the code
extern int		stepCount;						// for acoustic focusing 

extern unsigned int currTrig;						// the current trigger, globally available
extern unsigned int currIndx;						// the current ADC file index, globally available

extern unsigned long ROIArrayPtr;					// this is the pointer to the current location in the streamfile (next byte to write)
extern unsigned int	 ROIRamPtr;						// pointer into the local ram array
extern uint8		*ROIArray;						// array gets malloc'ed when written to and expands as needed

extern int	beadsInterval;
extern int	stainTime;
extern bool	autoShutdown;							// shutdown Windows when the run is complete
extern int	nSyringes;								// total number of syringes to run when in autostart mode
extern bool	manualBeads;							// if beads button is checked, the next sample(s) will be from internal bead reservoir (diluted with SW).
extern bool	AcousticFocusingOnALT;		//rob
extern bool	fluidSyncBusy;							// true when a FLUIDICS_SYNC operation is in progress
extern bool	runSampleFast;
extern bool	runSampleFastInitial;
extern bool	AcousticFocusing;

typedef enum	{INIT, NORMAL, ALT, BEADS} RUNTYPE;			// type of syringe
extern RUNTYPE	runType;

extern int AnalogFirmwareVersion;
extern int AcqFileName;

extern int HKFirmwareVersion;
