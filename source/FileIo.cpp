//-----------------------------------------------------------------------------
//  IFCB Project
//	FileIo.cpp
//	Martin Cooper
//
//	File access routines
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <sys/timeb.h>
#include "IfcbDlg.h"
#include "config.h"
#include "FileIo.h"
#include "Daq.h"
#include "Process.h"
#include "version.h"
#include <math.h>       /* ceil */
#include <iostream>	
#include <string>	
using namespace std;
#include <fstream>	
#include <stdlib.h>	//atoi

static CFile	HdrFile;
static CFile	RoiFile;
static CFile	AdcFile;
static CFile	DigFile;
//static CFile	FreqFile;
static char		outString[LONG_STR_BUF_LEN];
static CString	FileName;
static CString  ISOTimeStamp;

// types are 's' = string, 'd' = int, 'b' = bool, 'f' = double
static CONFIG_PARAM configParams[] = {
	{	1,	'd', &imagerID,				"imagerID"},
	{	2,	's', fluidicsPort,			"KloehnPort"},
	{	3,	's', dataPath,				"DataFilePath"},
	{	4,	'd', &syringeOffset,		"syringeOffset"},
	{	5,	'b', &appDebug,				"appDebug"},
	{	6,	'b', &viewImages,			"viewImages"},
	{	7,	'b', &blobAnalysis,			"blobAnalysis"},
	{	8,	'b', &pumpDrive,			"pumpDriveBoardPresent"},
	//{	9,	'b', &discreteSampleIntake,	"discreteSampleIntake"},
	{	9,	'b', &backflushWithSample,	"backflushWithSample"},
	{	10,	'b', &outputFiles,			"outputFiles"},
	{	11, 'b', &fluidicsVerbose,		"fluidicsVerbose"},
	{	12, 'b', &fluidsActive,			"fluidsActive"},
	{	13, 'd', &binarizeThresh,		"binarizeThreshold"},
	{	14, 'd', &blobXGrow,			"blobXgrowAmount"},
	{	15, 'd', &resizeFactor,			"imageResizeFactor"},
	{	16, 'd', &minimumBlobArea,		"minimumBlobArea"},
	{	17, 'd', &integratorSettleTime, "triggerPulseEndtoADCstart_x2170ns_DAQ_MCC_only"},
	{	18, 'f', &highVoltagePMTA,		"PMTAhighVoltage"},
	{	19, 'f', &highVoltagePMTB,		"PMTBhighVoltage"},
	{	20, 'f', &highVoltagePMTC,		"PMTChighVoltage"},
	{	21, 's', daqPort,				"DAQ_MCCserialPort_DAC_MCConly"},
	{	22, 'd', &adcDrate,				"ADCdataRateDAQ_MCConly"},
	{	23, 'f', &flashVoltage,			"FlashlampControlVoltage"},
	{	24, 'd', &fanThresh,			"FanTemperatureThreshold_DAC_MCConly"},
	{	25, 'd', &fanHysteresis,		"FanTemperatureHysteresis_DAC_MCConly"},
	{	26, 'd', &humidThresh,			"ValveHumidityThreshold_DAC_MCConly"},
	{	27, 'd', &humidHysteresis,		"ValveHumidityHysteresis_DAC_MCConly"},
	{	28, 'd', &valveDelay,			"ValveHoldTime_x100us_DAC_MCConly"},
	{	29, 'd', &flashDelay,			"HKTRIGGERtoFlashlampDelayTime_x434ns_DAC_MCConly"},
	{	30, 'd', &flashPulseLength,		"FlashlampPulseLength_x434ns_DAC_MCConly"},
	{	31, 'd', &cameraPulseLength,	"CameraPulseLength_x434ns_DAC_MCConly"},
	{	32, 'f', &sampleVolume,			"SyringeSampleVolume"},
	{	33, 'd', &blobYGrow,			"blobYgrowAmount"},
	{	34, 'd', &blobJoinGap,			"minimumGapBewtweenAdjacentBlobs"},
	{	35, 'd', &smallFocusStep,		"focusMotorSmallStep_ms"},
	{	36, 'd', &largeFocusStep,		"focusMotorLargeStep_ms"},
	{	37, 'f', &trigThreshA,			"PMTAtriggerThreshold_DAQ_MCConly"},
	{	38, 'f', &trigThreshB,			"PMTBtriggerThreshold_DAQ_MCConly"},
	{	39, 'f', &trigThreshC,			"PMTCtriggerThreshold_DAQ_MCConly"},
	{	40, 'f', &trigThreshD,			"PMTDtriggerThreshold_DAQ_MCConly"},
	{	41, 'd', &triggerPulseMask,		"PMTtriggerSelection_DAQ_MCConly"},
	{	42, 'd', &adcPga,				"ADCPGA_DAQ_MCConly"},
	{	43, 'd', &triggerTimeout,		"triggerAckTimeout_x34720ns_DAQ_MCConly"},
	{	44, 'd', &pulseStretchTime,		"pulseStretchTime_x2170ns_DAQ_MCConly"},
	{	45, 'b', &debubbleWithSample,	"debubbleWithSample"},
	{	46, 'f', &pumpVolts,			"pumpDriveVoltage"},
	{	47, 'b', &autoShutdown,			"autoWindowsShutdown"},
	{	48, 'd', &nSyringes,			"NumberSyringesToAutoRun"},
	{	49, 'd', &altInterval,			"intervalBetweenAlternateHardwareSettings"},
	{	50, 'f', &trigThreshA2,			"altPMTATriggerThreshold_DAQ_MCConly"},
	{	51, 'f', &trigThreshB2,			"altPMTBTriggerThreshold_DAQ_MCConly"},
	{	52, 'f', &trigThreshC2,			"altPMTCTriggerThreshold_DAQ_MCConly"},
	{	53, 'f', &trigThreshD2,			"altPMTDTriggerThreshold_DAQ_MCConly"},
	{	54, 'f', &highVoltagePMTA2,		"altPMTAHighVoltage"},
	{	55, 'f', &highVoltagePMTB2,		"altPMTBHighVoltage"},
	{	56, 'f', &highVoltagePMTC2,		"altPMTCHighVoltage"},
	{	57, 'd', &adcPga2,				"altADCPGA_DAQ_MCConly"},
	{	58, 'd', &triggerPulseMask2,	"altPMTtriggerSelection_DAQ_MCConly"},
	{	59, 'd', &beadsInterval,		"NumberSyringesBetweenBeadsRun"},
	{	60, 'd', &smallLaserStep,		"laserMotorSmallStep_ms"},
	{	61, 'd', &largeLaserStep,		"laserMotorLargeStep_ms"},	
	{	62, 'f', &sampleVolume2,		"altSyringeSampleVolume"},
	{	63, 'd', &maxPulseStretch,		"maximumPulseLength_x2170ns_DAQ_MCConly"},
	{	64, 'b', &hasExpertTab,			"expertTabVisible"},
	{	65, 'd', &tcpPort,				"TCPport"},
	{	66, 'f', &humidityThreshold,	"HumidityAlarmThreshold(%)"},
	{	67, 'd', &pumpState,			"PumpStates"},
	{	68, 'f', &beadsVolume,			"BeadsSampleVolume"},
	{	69, 'f', &runFastFactor,		"RunFastFactor"},
	{	70, 'd', &acqStartTimeout,		"TimeoutForFirstFrame"},
	{	71, 'f', &sampleVolume2skip,	"sampleVolume2skip"},
	{	72, 'f', &sampleVolume2skipALT,	"altSampleVolume2skip"},
	{	73, 'd', &altPortNumber,		"altSamplePortNumber"},
	{	74,	'b', &primeSampleTube,		"primeSampleTube"},
	{	75,	'b', &stainSample,			"stainSample_manual"}, 
	{	76,	'b', &StainAuto,			"StainAuto_auto"},
	{	77, 'f', &flashVoltage2,		"Alt_FlashlampControlVoltage"},
	{	78, 'd', &stainTime,			"stainTime"},
	{	79, 'b', &StainRinseMixingChamber,			"rinseOption"},
	{	80, 'd', &primValvePort,		"primaryIntakePort"},
	{	81, 'd', &secValvePort,			"secondaryIntakePort"},
	{	82, 'd', &normPortNumber,		"normSamplePortNumber"},
	{	83, 'b', &replaceFromSec,		"replaceFromSecondary"},
	{	84, 'd', &syr_sampling_speed,	"syringeSamplingSpeed"},
	{	85, 'd', &base_syr_sampling_speed,	"baseSyringeSamplingSpeed"},
	{	86, 'b', &runSampleFast,		"runSampleFast"},
	{	87, 'd', &laserState,			"laserState"},
	{	88, 'd', &fill2bypassFilterState,		"fill2bypassFilterState"},
	{	89, 'd', &fill3newSheathState,	"fill3newSheathState = using new SW for sheath"},
	{	90, 'b', &LockFilterSlider,		"LockFilterSlider"},
	{	91, 'd', &syringeSize,			"syringeSize"},
	{	92, 'b', &hasCamera,			"hasCamera"},
	{	93, 'b', &AcousticFocusing,		"AcousticFocusingOnALT"},
	{	94, 'b', &debubbleWithSample2,	"Refill after Debubble"},
	{	95, 's', fileComment,			"FileComment"},
	{	96, 'd', &PZTvolts,				"PZTvolts"},
	{	97, 'd', &PZTfrequency,				"PZTfrequency"},
	{	98, 'b', &TemperatureAdjust,	"TemperatureAdjust"},
	{	99, 'z', NULL,					""},
	{	100,'d', &FreqUpdateInterval,	"FreqUpdateInterval"},
	{	101, 'b', &DoFreqScan,			"DoFreqScan"},
	{	102, 'f', &StartFreq,			"StartFreq"},
	{	103, 'f', &FreqStep,			"FreqStep"},
	{	104, 'f', &EndFreq,				"EndFreq"},
	{	105, 'f', &StepDuration,		"StepDuration"},
	{	106, 'd', &stepCount,			"stepCount"},
	{	107, 'f', &Tadjm,					"Tadjm"},
	{	108, 'f', &Tadjb,					"Tadjb"},

	{	255, 'z', NULL,				""}						// last one - dummy
};

//-----------------------------------------------------------------------------
//	Converts CString unicode 16-bit characters to char
//	Amazingly, CString doesn't have a member to do this
//-----------------------------------------------------------------------------
void CStringToChar(char *dest, CString *source) {

	int i;
	int len = source->GetLength();
	char *p;

	for (p = dest, i = 0; i < len; i++)
		*p++ = (char)source->GetAt(i);
	*p = '\0';
}

//-----------------------------------------------------------------------------
CString GetOutputFileName(void) {

	return FileName;
}

//-----------------------------------------------------------------------------
void CreateOutputFileNames(unsigned int dataDirectory) {

	struct tm gmttime;

	if (dataDirectory == FILES_NORMAL)
		FileName = dataPath;
	else if (dataDirectory == FILES_BEADS) {
		FileName = dataPath;
		FileName += (_T("/beads"));
	} else {	// throw a fit
		ERROR_MESSAGE_EXT(_T("ERROR in CreateOutputFileNames - forcing no output files\r\n"));
		outputFiles = false;
		return;
	}
	
	DWORD attrib = GetFileAttributes(FileName);		// check the directory exists
	if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectory(FileName, NULL);			// path doesn't exist, so create it
	
	// create a filename from the time, date, etc.
	// should look like:
	// IFCB2_2010_124_135650.roi etc.
	time_t now;
	time(&now);										// Get the current time from the operating system.
	_gmtime64_s( &gmttime, &now );
	
	//FileName += (_T("/IFCB"));

	CString id;
	CString timestr;
	id.Format(_T("%03d"), imagerID);					// add in the imagerID
	//FileName += id;
	//FileName += Time.Format(_T("_%Y_%j_%H%M%S"));
	FileName += (_T("/D"));
	timestr.Format (_T("%4d%02d%02dT%02d%02d%02d"), gmttime.tm_year+1900, gmttime.tm_mon+1, gmttime.tm_mday, gmttime.tm_hour, gmttime.tm_min, gmttime.tm_sec);
	FileName += timestr;
	FileName += (_T("_IFCB"));
	FileName += id;
	
	ISOTimeStamp.Format (_T("%4d-%02d-%02dT%02d:%02d:%02dZ\r\n"), gmttime.tm_year+1900, gmttime.tm_mon+1, gmttime.tm_mday, gmttime.tm_hour, gmttime.tm_min, gmttime.tm_sec);

	DEBUG_MESSAGE_EXT(_T("Created file - "));
	DEBUG_MESSAGE_EXT(FileName);
	DEBUG_MESSAGE_EXT(_T("\r\n"));

	IfcbHandle->SetDlgItemText(IDC_FILENAME, FileName);

	WriteAcqFileNameFile(); //to tell Acoustic Focusing what the current file name is... Rob
}

//-----------------------------------------------------------------------------
//	writes header file, then closes it
//-----------------------------------------------------------------------------
bool WriteHeaderFile(void) {

	if (!outputFiles) 
		return true;

	CFileException error;
	CALL_CHECKED_EXT(HdrFile.Open(FileName + _T(".hdr"), CFile::modeCreate | CFile::modeWrite, &error), TRUE, _T("Error opening file: ") + FileName + _T(".hdr\r\n"));

	if (HdrFile.m_hFile == CFile::hFileNull) {
		ERROR_MESSAGE_EXT(_T("Error in WriteHeaderFile: header file not open\r\n"));
		return false;
	}
	
	DEBUG_MESSAGE_EXT(_T("WriteHeaderFile: writing file\r\n"));
	
	sprintf_s(outString, LONG_STR_BUF_LEN, "softwareVersion: Imaging FlowCytobot Acquisition Software version 2.0, build %d; May 2010\r\n", BUILD_VERSION);
	HdrFile.Write(outString, strlen(outString));
	#ifdef DAQ_MCC
	sprintf_s(outString, LONG_STR_BUF_LEN, "Housekeeping firmware version %d\r\n", HKFirmwareVersion);
	HdrFile.Write(outString, strlen(outString));
	sprintf_s(outString, LONG_STR_BUF_LEN, "Analog firmware version %d\r\n", AnalogFirmwareVersion);
	HdrFile.Write(outString, strlen(outString));
	#endif

	sprintf_s(outString, LONG_STR_BUF_LEN, "syringe number %d\r\n", syringesDone + 1);
	HdrFile.Write(outString, strlen(outString));

	strcpy_s(outString, LONG_STR_BUF_LEN, "sampleTime: ");
	HdrFile.Write(outString, strlen(outString));
	CStringToChar(outString, &ISOTimeStamp);
    HdrFile.Write(outString, strlen(outString));

	// copy header file info
	//strcpy_s(outString, LONG_STR_BUF_LEN, "\r\nConfiguration File Parameters:\r\n");
	//HdrFile.Write(outString, strlen(outString));

	for (int i = 0; configParams[i].index != 255; i++) {
		switch (configParams[i].type) {
			case 's':						// strings
				sprintf_s(outString, LONG_STR_BUF_LEN, "%s: %s\t\r\n", configParams[i].description, configParams[i].p);
				break;
			case 'd':						// integers
				sprintf_s(outString, LONG_STR_BUF_LEN, "%s: %d\t\r\n", configParams[i].description, *(int *)configParams[i].p);
				break;
			case 'b':						// bool
				sprintf_s(outString, LONG_STR_BUF_LEN, "%s: %c\t\r\n", configParams[i].description, *(bool *)configParams[i].p ? '1' : '0');
				break;
			case 'f':						// double
				sprintf_s(outString, LONG_STR_BUF_LEN, "%s: %lf\t\r\n", configParams[i].description, *(double *)configParams[i].p);
				break;
			default:
				break;
		}
		HdrFile.Write(outString, strlen(outString));
	}

	// run type
	switch (runType) {
		case INIT:
		case NORMAL:
			strcpy_s(outString, LONG_STR_BUF_LEN, "runType: NORMAL\r\n");
			break;
		case ALT:
			strcpy_s(outString, LONG_STR_BUF_LEN, "runType: ALT\r\n");
			break;
		case BEADS:
			strcpy_s(outString, LONG_STR_BUF_LEN, "runType: BEADS\r\n");
			break;
	}
	HdrFile.Write(outString, strlen(outString));

	#ifdef DAQ_MCC
	//sprintf_s(outString, LONG_STR_BUF_LEN, "run time = %lf s\t inhibit time = %lf s\r\n", gRunTime, gInhibitTime);
	sprintf_s(outString, LONG_STR_BUF_LEN, "runTime: %lf\t\r\n", gRunTime);
	HdrFile.Write(outString, strlen(outString));
	sprintf_s(outString, LONG_STR_BUF_LEN, "inhibitTime: %lf\t\r\n", gInhibitTime);
	HdrFile.Write(outString, strlen(outString));
	ReadHumidTemp();
	//sprintf_s(outString, LONG_STR_BUF_LEN, "%lf temperature,\t%lf humidity\r\n", Temperature, Humidity);
	sprintf_s(outString, LONG_STR_BUF_LEN, "temperature: %lf\t\r\n", Temperature);
	HdrFile.Write(outString, strlen(outString));
	sprintf_s(outString, LONG_STR_BUF_LEN, "humidity: %lf\t\r\n", Humidity);
	HdrFile.Write(outString, strlen(outString));
	#endif

	sprintf_s(outString, LONG_STR_BUF_LEN, "ADCFileFormat: trigger#, ADC_time, PMTA, PMTB, PMTC, PMTD, peakA, peakB, peakC, peakD, time of flight, grabtimestart, grabtimeend, ROIx, ROIy, ROIwidth, ROIheight, start_byte, comparator_out, STartPoint, SignalLength, status, runTime, inhibitTime\r\n");
	if (AcousticFocusing)
		strcpy_s(outString + strlen(outString) - 2, LONG_STR_BUF_LEN, ", temperature, frequency\r\n");
	HdrFile.Write(outString, strlen(outString));

	HdrFile.Close();

	return true;
}

//-----------------------------------------------------------------------------
bool WriteAdcFile(void) {

	if (!outputFiles)
		return true;					// exit if output files not required

	CString str; 

	CALL_CHECKED_EXT(AdcFile.Open(FileName + _T(".adc"), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite, NULL), TRUE, _T("Error opening file: ") + FileName + _T(".adc\r\n"));
	if (AdcFile.m_hFile == CFile::hFileNull) {
		ERROR_MESSAGE_EXT(_T("Error in WriteAdcFile: adc file not open\r\n"));
		return false;
	}

	str.Format(_T("WriteAdcFile: writing file of %d records\r\n"), currIndx);
	DEBUG_MESSAGE_EXT(str);

	AdcFile.SeekToEnd();

	// loop through all the data records in ifcb_data & write them out to ADC file
	for (unsigned int i = 0; i < currIndx; i++)	{
//		if (sprintf_s(outString, LONG_STR_BUF_LEN, "%d,%f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%f,%f,%d,%d,%d,%d,%u,%f,%d,%d,%d\r\n",	// no peak values
		if (sprintf_s(outString, LONG_STR_BUF_LEN, "%d,%f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%1.5f,%f,%f,%d,%d,%d,%d,%u,%f,%d,%d,%d,%f,%f\r\n",	// peak values
					ifcb_data[i].trigger, ifcb_data[i].ADC_time, 
#ifdef DAQ_MCC
					ifcb_data[i].integA, ifcb_data[i].integB, ifcb_data[i].integC, ifcb_data[i].integD,
					ifcb_data[i].peakA, ifcb_data[i].peakB, ifcb_data[i].peakC, ifcb_data[i].peakD,				// peak values
#else
					-ifcb_data[i].integAlo, -ifcb_data[i].integAhi, -ifcb_data[i].integClo, -ifcb_data[i].integChi,
#endif					
					ifcb_data[i].time_of_flight, 
					(float)(ifcb_data[i].grabtimestart/1000.0), (float)(ifcb_data[i].grabtimeend/1000.0), // these need to be in fracs of secs
					ifcb_data[i].LLHxloc, ifcb_data[i].LLHyloc,
					ifcb_data[i].roiSizeX, ifcb_data[i].roiSizeY,
					ifcb_data[i].start_byte, ifcb_data[i].comparator_out,
					ifcb_data[i].StartPoint, ifcb_data[i].SignalLength, (unsigned int)ifcb_data[i].status,
					ifcb_data[i].runTime, ifcb_data[i].inhibitTime) == -1) {

			ERROR_MESSAGE_EXT(_T("Error making string for ADC file\r\n"));
			return false;
		}
		if (AcousticFocusing) {
			if (sprintf_s(outString + strlen(outString) - 2, LONG_STR_BUF_LEN, ",%f,%d\r\n", ifcb_data[i].temperature, ifcb_data[i].frequency) == -1) {
				ERROR_MESSAGE_EXT(_T("Error making string for ADC file\r\n"));
				return false;
			}
		}

		// write it out
		AdcFile.Write(outString, strlen(outString));
	}
	AdcFile.Close();

	return true;
}

//-----------------------------------------------------------------------------
bool WriteRoiFile(void) {

	if (!outputFiles) 
		return true;					// exit if output files not required

	CALL_CHECKED_EXT(RoiFile.Open(FileName + _T(".roi"), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite, NULL), TRUE, _T("Error opening file: ") + FileName + _T(".roi\r\n"));
	if (RoiFile.m_hFile == CFile::hFileNull) {
		ERROR_MESSAGE_EXT(_T("Error in WriteRoiFile: roi file not open\r\n"));
		return false;
	}

	DEBUG_MESSAGE_EXT(_T("WriteRoiFile: writing file\r\n"));

	RoiFile.SeekToEnd();
	RoiFile.Write(ROIArray, ROIRamPtr);
	RoiFile.Close();

	return true;
}

//-----------------------------------------------------------------------------
bool WriteDigFile(unsigned int nRecs) {

	if (!outputFiles) 
		return true;					// exit if output files not required

	CALL_CHECKED_EXT(DigFile.Open(FileName + _T(".dig"), CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite, NULL), TRUE, _T("Error opening file: ") + FileName + _T(".dig\r\n"));
	if (DigFile.m_hFile == CFile::hFileNull) {
		ERROR_MESSAGE_EXT(_T("Error: dig file not open\r\n"));
		return false;
	}
	DigFile.SeekToEnd();
	DigFile.Close();

	return true;
}

//-----------------------------------------------------------------------------
bool WriteAcqFileNameFile(void) {   //to allow Acoustic Focusing to find the name of the current IFCB Acq file.

	CFile namFile;

	DEBUG_MESSAGE_EXT(_T("Writing Acq file name file\r\n"));

	CALL_CHECKED_RETFALSE_EXT(namFile.Open(_T("c://data//ifcb.nam"), CFile::modeCreate | CFile::modeWrite, NULL), TRUE, _T("Error opening config file\r\n"));

	char line[81];

	CT2A ascii(FileName);
	TRACE(_T("ASCII: %S\n"), ascii.m_psz);

	sprintf_s(line, 80, "%s\r\n", ascii.m_psz);
	namFile.Write(line, strlen(line));

	//also tell Labview whether VS is saving file... so Labview won't write a file unless VS does.
	if (outputFiles == true) {
		sprintf_s(line, 80, "%s\r\n", "1");
		namFile.Write(line, strlen(line));
	}
	else {
		sprintf_s(line, 80, "%s\r\n", "0");
		namFile.Write(line, strlen(line));
	}

	namFile.Close();

	return true;
}


//-----------------------------------------------------------------------------

bool WriteCfgFile(void) {

	CFile cfgFile;

	DEBUG_MESSAGE_EXT(_T("Writing config file\r\n"));

	CALL_CHECKED_RETFALSE_EXT(cfgFile.Open(_T("ifcb.cfg"), CFile::modeCreate | CFile::modeWrite, NULL), TRUE, _T("Error opening config file\r\n"));

	char line[150];
	unsigned char item;

	sprintf_s(line, 80, "IFCB Configuration File\r\n");			// This is to get round a weird bug in ReadCfgFile()
	cfgFile.Write(line, strlen(line));							// ... it mis-reads the first line

	for (int i = 0; ; i++) {
		item = configParams[i].index;
		if (item == 255)
			break;
		switch (configParams[i].type) {
			case 's':						// strings
				sprintf_s(line, 80, "%d)\t%s\t: %s\r\n", item, configParams[i].p, configParams[i].description);
				break;
			case 'd':						// integers
				sprintf_s(line, 80, "%d)\t%d\t: %s\r\n", item, *(int *)configParams[i].p, configParams[i].description);
				break;
			case 'b':						// bool
				sprintf_s(line, 80, "%d)\t%c\t: %s\r\n", item, *(bool *)configParams[i].p ? '1' : '0', configParams[i].description);
				break;
			case 'f':						// double
				sprintf_s(line, 80, "%d)\t%lf\t: %s\r\n", item, *(double *)configParams[i].p, configParams[i].description);
				break;
			default:
				*line = '\0';
				break;
		}
		cfgFile.Write(line, strlen(line));
	}
	cfgFile.Close();

	return true;
}

//-----------------------------------------------------------------------------

bool WriteFreqFile(void) {

	CFile FreqFile;
	CString str;

	DEBUG_MESSAGE_EXT(_T("Writing freq.txt\r\n"));

	CALL_CHECKED_RETFALSE_EXT(FreqFile.Open(_T("freq.txt"), CFile::modeCreate | CFile::modeWrite, NULL), TRUE, _T("Error opening config file\r\n"));
	//CALL_CHECKED_RETFALSE_EXT(FreqFile.Open(_T("C:\\ifcbAcq_AcousticFocusing_500\\IFCB NoMIL\\Release\\freq.txt"), CFile::modeCreate | CFile::modeWrite, NULL), TRUE, _T("Error opening config file\r\n"));

	char line[150];
	//unsigned char item;

	//sprintf_s(line, 80, "IFCB Freq File\r\n");			// This is to get round a weird bug in ReadCfgFile()
	//freqFile.Write(line, strlen(line));
	// ... it mis-reads the first line
	//int TempFreq = 1660000;
	//int TempFreq = PZTfrequency;//PZThz
	str.Format(_T("freq =  %d\r\n"), PZTfrequency);
		DEBUG_MESSAGE_EXT(str);

	sprintf_s(line, 80, "%d\r\n", PZTfrequency);
	FreqFile.Write(line, strlen(line));	
	sprintf_s(line, 80, "%d\r\n", PZTfrequency); // write it twice in case first line misreads...
	FreqFile.Write(line, strlen(line));
	sprintf_s(line, 80, "%d\r\n", stepCount);
	FreqFile.Write(line, strlen(line));

	FreqFile.Close();
	DEBUG_MESSAGE_EXT(_T("freq.txt written... \r\n"));

	return true;
}

//-----------------------------------------------------------------------------
bool ReadCfgFile() {

	CFile cfgFile;
	CString str;
	// bail out if there's no config file
	if (!cfgFile.Open(_T("ifcb.cfg"), CFile::modeRead, NULL)) {
		IfcbHandle->MessageBox(_T("Fatal error - config file not found"), _T("Fatal error\r\n"), MB_ICONERROR | MB_OK);
		ExitProcess(IDOK);
	}

	int item, i, x;
	bool found;
	char file[CFG_FILE_LENGTH + 1], *line, *p;
	cfgFile.Read(file, CFG_FILE_LENGTH);
	cfgFile.Close();

	// read file a line at a time
	if ((line = strtok_s(file, "\r", &p)) == NULL) {
		ERROR_MESSAGE_EXT(_T("Error reading config file\r\n"));
		return false;
	}

	do {
		if (!sscanf_s(line, " %d)", &item))					// extract the index
			continue;
		for (i = 0, found = false; ; i++) {					// now look for this index
			if (configParams[i].index == 255)				// end of list
				break;
			if (item == configParams[i].index) {			// found it
				found = true;
				break;
			}
		}
		if (!found)
			continue;

		switch (configParams[i].type) {						// now read the data
			case 's':										// strings
				sscanf_s(line, " %d) %s ", &item, (char *)configParams[i].p, STRING_PARAM_LENGTH);
				break;
			case 'b':										// bool
				sscanf_s(line, " %d) %d", &item, &x);
				*(bool *)configParams[i].p = (x == 1);
				break;
			case 'd':										// int
				sscanf_s(line, " %d) %d", &item, configParams[i].p);
				break;
			case 'f':
				sscanf_s(line, " %d) %lf", &item, configParams[i].p);
				break;
			default:
				break;
		}
	} while ((line = strtok_s(NULL, "\n", &p)) != NULL);

	// some tidying
	blobXGrow = max(blobXGrow, 1);
	blobYGrow = max(blobYGrow, 1);

	// for acoustic focusing frequency scanning
	if (DoFreqScan)	{ // calculate things needed to loop within and across syringes
		mlPerMin = runFastFactor * 0.25; //0.25 ml/min is default sample rate for IFCBs
		minPerSyr = (sampleVolume2 - sampleVolume2skipALT) / (mlPerMin);// for acoustic focusing
		minPerStep = StepDuration / 60;	
		freqstepsPerSyr = minPerSyr / minPerStep;						// for acoustic focusing 
		syrPerCycle = (int)(ceil((EndFreq - StartFreq) / freqstepsPerSyr / FreqStep));
		freqstepsPerCycle = (int)((EndFreq - StartFreq) / FreqStep) + 1;
		/*str.Format(_T("freqstepsPerCycle = %d"), int(floor(freqstepsPerCycle))); 
			DEBUG_MESSAGE_EXT(str); //crashes because Debug window doesn't exist yet???
		str.Format(_T("Number of syringes needed =  %d\r\n"), syrPerCycle);
			DEBUG_MESSAGE_EXT(str); */ //crashes because Debug window doesn't exist yet???
		stepCount = 1; //for acoustic focusing freq scanning
		//PZThz = (int)StartFreq;
		PZTfrequency = (int)StartFreq;
	}
	return true;
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
bool ReadFreqFile() {

	CFile FreqFile;
	CString str, str2, str1;
	string STRING1, STRING2, STRING3;
	ifstream infile;

	//infile.open ("C:\\ifcbAcq_AcousticFocusing_487Rob\\ifcb-aq\\trunk\\AcousticFocusing_Acq\\IFCB NoMIL\\freq.txt");
	infile.open ("freq.txt");
	getline(infile,STRING1); // Saves the line in STRING1.
	getline(infile,STRING2); // use the second line because Martin saw misreads of first line in cfg file
	getline(infile,STRING3);
	infile.close();
	str1=STRING2.c_str(); //convert string to CString
	Sleep(100);
	infile.open ("freq.txt");
	getline(infile,STRING1); // Saves the line in STRING1.
	getline(infile,STRING2);// use the second line because Martin saw misreads of first line in cfg file
	getline(infile,STRING3);
	infile.close();
	str2=STRING2.c_str(); //convert string to CString

	if (str1 == str2)   {
		str = str1;
		DEBUG_MESSAGE_EXT(_T("str1 = str2... \r\n"));
	}
	else {
		str = str2;
		DEBUG_MESSAGE_EXT(_T("str1 ~= str2... using str2.  \r\n"));
	}
	PZTfrequency = _ttoi(str);// convert C-string to integer
	//str.Format(_T("ReadFreqFile: freq = %d \r\n"), PZTfrequency);
		//DEBUG_MESSAGE_EXT(str);
		return true;
}

//-----------------------------------------------------------------------------
bool CheckEmptyFiles(void) {

	DWORD size;
	CFile file;

//	if (!file.Open(FileName + _T(".roi"), CFile::modeRead, NULL))
//		return true;										// failed to open the file
//
//	size = GetFileSize(HANDLE(file), NULL);
//	if (size == 0)
//		return true;

	if (!file.Open(FileName + _T(".adc"), CFile::modeRead, NULL))
		return true;										// failed to open the file

	size = GetFileSize(HANDLE(file), NULL);
	file.Close();

	return (size == 0);
}
