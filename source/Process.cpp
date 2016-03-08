//-----------------------------------------------------------------------------
//  IFCB Project
//	Process.cpp
//	Martin Cooper
//
//	This is the meat of the instrument hardware control
//-----------------------------------------------------------------------------
#include <math.h>
#include "stdafx.h"
#include "config.h"
#include "IfcbDlg.h"
#include "Daq.h"
#include "Process.h"
#include "Fluids.h"
#include "FileIo.h"
#include "Analysis.h"
#include "GraphTab.h"
#include "sig_gen_api.h"


bool			rqPZT = false;						// synchronises the PZT command to only happen directly after a capture
DWORD			StartTickCount;
static CString	debugStr;
static int		sBeadsCounter;	
static int		sAltCounter;

extern bool alt = false;
//extern int PZTfrequency;
 //int processcounter;
//int stepCount;


//-----------------------------------------------------------------------------
// set the PMTs, triggers etc using either the normal or alternate configuration
//-----------------------------------------------------------------------------
void SetHardware(bool alt) {

	if (alt) {
		ChangeHWParam(PMTA, highVoltagePMTA2);
		ChangeHWParam(PMTB, highVoltagePMTB2);
		ChangeHWParam(PMTC, highVoltagePMTC2);
		ChangeHWParam(TRIGA, trigThreshA2);
		ChangeHWParam(TRIGB, trigThreshB2);
		ChangeHWParam(TRIGC, trigThreshC2);
		ChangeHWParam(PGA, adcPga2);
		if (AcousticFocusing) {
					//fanThresh = 1;
					//ChangeHWParam(FAN_THRESH, fanThresh);
					runSampleFast = true;}
				//else {
					//fanThresh = 52428;
					//ChangeHWParam(FAN_THRESH, fanThresh);}
	} else {
		ChangeHWParam(PMTA, highVoltagePMTA);
		ChangeHWParam(PMTB, highVoltagePMTB);
		ChangeHWParam(PMTC, highVoltagePMTC);
		ChangeHWParam(TRIGA, trigThreshA);
		ChangeHWParam(TRIGB, trigThreshB);
		ChangeHWParam(TRIGC, trigThreshC);
		ChangeHWParam(PGA, adcPga);
		runSampleFast = runSampleFastInitial;
	}
	IfcbHandle->dacsOn = true;

}

//-----------------------------------------------------------------------------
//	send command to PZT control. Three modes: PZT_INIT, PZT_RUN, PZT_OFF, PZT_SCAN
//-----------------------------------------------------------------------------
void PZTcommand(PZTMODE mode) {

	if (!(AcousticFocusing && runType == ALT))				// exit if not using PZT
		return;

	CString args, str;

	if (mode == PZT_OFF) {
		siggen_on(false);	
		ChangeHWParam(FANS, OFF);							// turn off 24V to acoustic focusing freq generator

		return;
	}

	if ((mode == PZT_INIT)) { // || (mode == PZT_SCAN)) {
		ChangeHWParam(FANS, ON);						// turn on 24V for acoustic focusing freq generator
		Sleep(3000); // to power up hardware
		//siggen_on(true);
		if (!siggen_initialize()) {
			DEBUG_MESSAGE_EXT(_T("Failed to init siggen\r\n"));
		//ERROR_MESSAGE(_T("Failed to init siggen\r\n"));
		//if (!siggen_setCalibrationFile("AcousticCal.txt"))
			//ERROR_MESSAGE(_T("Failed to set cal file\r\n"))
		}
	}
		//if ((mode == PZT_SCAN) {

	if (TemperatureAdjust) //|| DoFreqScan)  //  modes where we want to update Freq during the run
			IfcbHandle->SetTimer(PZT_TIMER_ID, (int)(FreqUpdateInterval * 1000.0), NULL);	// set a timer that fires every X s and calls PZTcommand
		
	if (TemperatureAdjust) {
		ReadHumidTemp();
		//Temperature = 25; // for testing outside of ifcb
		//PZTfrequency = int(Temperature * 1740.2 + 1579000); // use calibration equation here to calculate frequency from temperature
		PZTfrequency = int(Temperature * Tadjm + Tadjb); // use calibration equation here to calculate frequency from temperature (m and b from cfg file)
		str.Format(_T("t = %f, f =  %d\r\n"), Temperature, PZTfrequency);
		DEBUG_MESSAGE_EXT(str);

		// now send the settings
		if (!siggen_on(true))
			DEBUG_MESSAGE_EXT(_T("Error turning siggen ON\r\n"));
		if (!siggen_setVoltage(PZTvolts))
			DEBUG_MESSAGE_EXT(_T("Error setting PZTvolts\r\n"));
		if (!siggen_setFrequency(PZTfrequency))
			DEBUG_MESSAGE_EXT(_T("Error setting PZTfrequency\r\n"));
		str.Format(_T("v = %d, f =  %d\r\n"), PZTvolts, PZTfrequency);
		DEBUG_MESSAGE_EXT(str);
	} 
	//else
		//PZTfrequency = PZThz;  //rem out 2 Feb 2016

	if (DoFreqScan) {
		//ReadFreqFile();
		
	// now send the settings
	if (!siggen_on(true))
		DEBUG_MESSAGE_EXT(_T("Error turning siggen ON\r\n"));
	if (!siggen_setVoltage(PZTvolts))
		DEBUG_MESSAGE_EXT(_T("Error setting PZTvolts\r\n"));
	if (!siggen_setFrequency(PZTfrequency))
		DEBUG_MESSAGE_EXT(_T("Error setting PZTfrequency\r\n"));
	str.Format(_T("v = %d, f =  %d\r\n"), PZTvolts, PZTfrequency);
	DEBUG_MESSAGE_EXT(str);
	}

}

	
/*	void PZTcommand(PZTMODE mode) {

	if (!(AcousticFocusing && runType == ALT))				// exit if not using PZT
		return;

	CString args, str;


	if (mode == PZT_OFF) {
		args = _T(" --nogui --off --hz 0 --volts 0");

		// turn off 24V to acoustic focusing freq generator
		if (AcousticFocusing && runType == ALT)
			ChangeHWParam(FANS, OFF);
	} 
	else {
		if ((mode == PZT_INIT) | (mode == PZT_SCAN)) {
			ChangeHWParam(FANS, ON);						// turn on 24V for acoustic focusing freq generator
			Sleep(3000);									// to power up hardware
			//IfcbHandle->SetTimer(PZT_TIMER_ID, 2000, NULL);	// set a timer that fires every 2 s and calls PZTcommand
			if (TemperatureAdjust | DoFreqScan)  // this is the only mode where we want to update Freq during the run (until scanning is implemented)
				IfcbHandle->SetTimer(PZT_TIMER_ID, FreqUpdateInterval * 1000, NULL);	// set a timer that fires every X s and calls PZTcommand
		}

		if (TemperatureAdjust) {
			ReadHumidTemp();
			//Temperature = 25; // for testing outside of ifcb
			PZTfrequency = int(Temperature * 1740.2 + 1579000); // use calibration equation here to calculate frequency from temperature
			str.Format(_T("t = %f, f =  %d\r\n"), Temperature, PZTfrequency);
			DEBUG_MESSAGE_EXT(str);
		} else
			PZTfrequency = PZThz;
		if (DoFreqScan)
			ReadFreqFile();
		
		args.Format(_T(" --nogui --on --hz %d --volts %d"), PZTfrequency, PZTvolts);
		if (mode != PZT_INIT)
			args += _T(" --noinit");
				//	args.Format(_T(""));
				//	str = "%windir\\system32\\notepad.exe\r\n"; //testing
				//	str += (args);
					//DEBUG_MESSAGE_EXT(str);

	}
	DEBUG_MESSAGE_EXT(args); 
	DEBUG_MESSAGE_EXT(_T("\r\n"));

//	str = sigGenPath; // note -- this includes the file name as well as path (e.g., "C:\AcousticFocusingWHOI\sig_gen\sig-gen-v1.0.4_64bit\sig-gen-v1.0.4\sig-gen")

	ShellExecute(NULL, _T("open"), str, args, NULL, SW_HIDE);			// run sig-gen with the appropriate arguments
	//ShellExecute(NULL, _T("open"), str, args, NULL, SW_NORMAL);			// run sig-gen with the appropriate arguments
	//				args.Format(_T(""));
	//				str = "%windir\\system32\\notepad.exe";
	//				str += (args);
	//				DEBUG_MESSAGE_EXT(str);
	//				DEBUG_MESSAGE_EXT(_T("\r\n")); 
	//				ShellExecute(NULL, _T("open"), str, args, NULL, SW_NORMAL); 
}*/

//-----------------------------------------------------------------------------```
// initialises the hardware for a sampling run
//	called when the "Run" button is pressed in IfcbDlg::OnPlayBtnClicked()
//	or when a full syringe is completed, and you are in continuous mode
//	returns false if it failed to start
//-----------------------------------------------------------------------------
bool AcqInit(void) {

	currTrig = 1;							// first trigger is number 1 - R&H original usage for ifcb_data array
	currIndx = 0;							// this is zero-indexed (for ifcb_data array)
	// rob's original file structure has it start on 2nd byte, with first (&last) bytes in file 0x00
	int processcounter = 0;
	// initialise data arrays
	ROIArray = (uint8 *)realloc(ROIArray, ROI_ARRAY_SIZE_INC);
	ROIArrayPtr = ROIRamPtr = 0;
	ifcb_data = (DataStruct *)realloc(ifcb_data, IFCB_ARRAY_SIZE_INC * sizeof(DataStruct));
	dataStructSize = IFCB_ARRAY_SIZE_INC;

	base_syr_sampling_speed = 8962;

	if (!CheckHumidity())							// humidity too high
		return false;

	// prepare the run
	if (runType == INIT) {
		sBeadsCounter = 0;
		sAltCounter = 0;
		SetHardware(false);							// set 'normal' settings
		runType = NORMAL;
		//FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_IN, 0); // initialize filter slider to IN position
		//ChangeHWParam(FILL3newSheath, ON); //open solenoid 5 (NC, in sample tube from outside to syringe
	}

	if (manualBeads)									// this is a manual beads run
		runType = BEADS;

	IfcbHandle->WriteSyringeNum();

	if (altInterval == 999) {	// for testing, to make ALT the first thing run...
		runType = ALT;
		AcousticFocusingOnALT = true;  //rob
		alt = true;
		SetHardware(true);							// set hardware to alternate settings
	}

	switch (runType) {
		case INIT:
			sBeadsCounter = 0;
			sAltCounter = 0;
			SetHardware(false);						// set 'normal' settings
			// fall through...
		case NORMAL:
			DEBUG_MESSAGE_EXT(_T("AcqInit: Starting NORMAL syringe\r\n"));
			CreateOutputFileNames(FILES_NORMAL); 	// set up new output files
			syringeVolume = sampleVolume;			// set the syringe volume
			syringeVolume2skip = sampleVolume2skip;
			samplePortNumber = normPortNumber;
			FluidicsPullNewSample();				// pull in a new sample & start dispensing
			break;
		case ALT:									// hardware changes were made in AcqComplete()
			DEBUG_MESSAGE_EXT(_T("AcqInit: Starting ALT syringe\r\n"));
			CreateOutputFileNames(FILES_NORMAL); 	// set up new output files
			syringeVolume = sampleVolume2;			// set the syringe volume
			syringeVolume2skip = sampleVolume2skipALT; 
			samplePortNumber = altPortNumber;
			alt = true;
			SetHardware(true);							// set hardware to alternate settings
			//if (manualBeads & AcousticFocusingOnALT)  //rob
				//Beads();								//rob
			FluidicsPullNewSample();				// pull in a new sample & start dispensing
			break;
		case BEADS:
			CreateOutputFileNames(FILES_BEADS); 	// set up new output files
			syringeVolume = beadsVolume;			// set the syringe volume (from cfg file)
			if (!manualBeads)  {
				DEBUG_MESSAGE_EXT(_T("AcqInit: Starting Biocide, Clorox (sample tube), BEADS  \r\n"));
				Biocide();							// have to add Biocide before Beads or else Beads don't acquire...
				Clorox();							// backflush sample tubing with Clorox
				Beads();
			}
			else	{
				DEBUG_MESSAGE_EXT(_T("AcqInit: Starting BEADS \r\n"));
				Beads();
			}
			break;
	}

	// need to know ticks for when syringe starts, to time all ROI aqcs off of
	StartTickCount = GetTickCount();		// in ms since system started

	// figure out how much time to elapse given syringeVolume in cfg file
	// 5ml is ~ 20 mins = 1200 secs, 1200000 ms. so 240000 msecs per ml
	// SetTimer is in milliseconds
	DEBUG_MESSAGE_EXT(_T(" - in AcqInit - Starting syringe timer\r\n"));
	// start the timer for syringe completion & add a second extra for overhead

	if (runType == BEADS) {
		IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((beadSampleTotalVolume-syringeVolume2skip) * 240000 / runFastFactor) + 1000), NULL);
	}

	if (runSampleFast) {
		//IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000 / runFastFactor) + 1000), NULL);
		IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000 / runFastFactor) - 2000), NULL);//stop acq 2 s before syringe move ends, while the sample is still pumping normally -- to get accurate sample volume from timer.
		//add 30 s for long-sample-tube pause
		//(Rob) no, don't do extra 30-s -- this should be handled by a sample-tube-length variable if it is needed -- otherwise normal sampling is excruciatingly slow.
		//IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000 / runFastFactor) + 31000), NULL);
	}
	else {
		//IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000) + 1000), NULL);
//		IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000) + 5000), NULL);
		//IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000 ) + 5000), NULL);
		IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor((syringeVolume-syringeVolume2skip) * 240000 ) - 2000), NULL); //stop acq 2 s before syringe move ends, while the sample is still pumping normally -- to get accurate sample volume from timer.
	}
	//	IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor(syringeVolume * 240000 / runFastFactor) + 1000), NULL);
	//else
	//	IfcbHandle->SetTimer(SYRINGE_TIMER_ID, (UINT)(floor(syringeVolume * 240000) + 1000), NULL);

	IfcbHandle->SetTimer(ACQ_START_TIMEOUT_ID, acqStartTimeout, NULL);	// set a timer that fires after timeout period
	DEBUG_MESSAGE_EXT(_T("AcqInit: Armed and ready\r\n"));

	if (hasCamera)
		IfcbHandle->TabContainer.tabPages[cameraTab]->ZeroFrameCount();

	if (DoFreqScan){
		ReadFreqFile(); //get the current PZTfrequency (and stepCount);
		DEBUG_MESSAGE_EXT(_T("DoFreqScan \r\n"));
		PZTcommand(PZT_INIT);
	}	
	
	if (TemperatureAdjust){
		DEBUG_MESSAGE_EXT(_T("TemperatureAdjust \r\n"));
		PZTcommand(PZT_INIT);
	}
	//PZTcommand(PZT_INIT);

	// arm the trigger circuit
	StartAcquisition();
	DaqArmTrigger();
	
	if(DoFreqScan) {
		if (stepCount==1) {
			NewFreq = StartFreq;}
		IfcbHandle->SetTimer(PZT_SCAN_TIMER_ID, (int)(StepDuration * 1000.0), NULL);	// set a timer that fires every StepDuration s to set the new freq.
	}

	return true;
}

//-----------------------------------------------------------------------------
// performs all tasks that need to be done after a number of triggers
//	e.g., after a full syringe
//	manual flag means override nSyringes and stop
//-----------------------------------------------------------------------------
void AcqComplete(int mode) {

	DEBUG_MESSAGE_EXT(_T(" - in AcqComplete\r\n"));

	#ifdef DAQ_MCC
	StopAcquisition();
	debugStr.Format(_T("run time = %lf s\r\ninhibit time = %lf s\r\n"), gRunTime, gInhibitTime);
	ERROR_MESSAGE_EXT(debugStr);
	#endif

	if (AcousticFocusing) {
		PZTcommand(PZT_OFF);
		WriteFreqFile();
		IfcbHandle->KillTimer(PZT_TIMER_ID);
		IfcbHandle->KillTimer(PZT_SCAN_TIMER_ID);
		//IfcbHandle->KillTimer(PZT_TIMER_NOINIT_ID);
	}

	IfcbHandle->KillTimer(SYRINGE_TIMER_ID);				// kill the timers
	IfcbHandle->KillTimer(ACQ_START_TIMEOUT_ID);
	FluidicsRoutine(FLUIDICS_STOP, 0);						// stop the syringe if it's running

	WriteHeaderFile();
	WriteAdcFile();
	if (hasCamera)
		WriteRoiFile();
	
	if (stainSample || (StainAuto && runType == ALT))	{ 
	DEBUG_MESSAGE_EXT(_T(" - washing stain out for 15 s before recirculating sheath... \r\n"));
	Sleep(15000); //wait 15 sec before switching back to recirculating sheath.
		if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
			ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
			ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
			DEBUG_MESSAGE_EXT(_T(" - checkbox says DON'T recirculate sheath... \r\n"));
		}
		else {
			ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
			ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
			DEBUG_MESSAGE_EXT(_T(" - recirculating sheath... \r\n"));
		}
	}

	// check the file sizes and exit if they're empty
//	if (outputFiles && CheckEmptyFiles())
//		ExitProcess(IDOK);

	DEBUG_MESSAGE_EXT(_T("AcqComplete: Acquisition finished\r\n"));

	// now decide what to do next
	if ((mode == END_RETRY) && hasCamera) {
		IfcbHandle->TabContainer.tabPages[cameraTab]->CloseDriver();		// shutdown the camera driver
		IfcbHandle->TabContainer.tabPages[cameraTab]->OpenDriver();		// and restart it
		IfcbHandle->TabContainer.tabPages[cameraTab]->Run(true);
		return;																// pretend nothing happened
	}


	if (!nSyringes || (mode == END_MANUAL)) {				// stop
		ChangeHWParam(PMTA, 0.0);							// switch PMTs off
		ChangeHWParam(PMTB, 0.0);
		ChangeHWParam(PMTC, 0.0);
		IfcbHandle->dacsOn = false;
		runType = INIT;
		return;
	}

	switch (runType) {										// runType is the run that has just completed
		case INIT:
			sAltCounter++;
			sBeadsCounter++;
			runType = NORMAL;
			break;
		case NORMAL:
			sAltCounter++;
			sBeadsCounter++;
			if (altInterval && (sAltCounter >= altInterval)) {	// alt takes precedence over beads
				runType = ALT;
				SetHardware(true);							// set hardware to alternate settings
			} else if (beadsInterval && (sBeadsCounter >= beadsInterval))
				runType = BEADS;
			break;
		case ALT:
			sAltCounter = 0;
			sBeadsCounter++;
			SetHardware(false);								// set hardware back to 'normal' settings
			//SetHardware(true);							// set hardware to alternate settings
			if (beadsInterval && (sBeadsCounter >= beadsInterval))
				runType = BEADS;
			else
				runType = NORMAL;
			break;
		case BEADS:
			sBeadsCounter = 0;
			runType = NORMAL;
			DEBUG_MESSAGE_EXT(_T(" - starting Biocide, Clorox, syringe rinses (~ 7 min) \r\n"));
			Biocide();  //rob -- put these here to get bead file to acq during automatic running..
			Clorox();
			break;
	}
}

//-----------------------------------------------------------------------------
//	Writes arrays to file when ram full
//-----------------------------------------------------------------------------
void WriteArraysToFile(void) {

	unsigned int status = ifcb_data[currIndx].status;

	WriteAdcFile();
	WriteRoiFile();

	// reset pointers
	currIndx = 0;
	ROIRamPtr = 0;

	// reallocate arrays
	free(ROIArray);
	free(ifcb_data);
	ROIArray = (uint8 *)malloc(ROI_ARRAY_SIZE_INC);
	ifcb_data = (DataStruct *)malloc(IFCB_ARRAY_SIZE_INC * sizeof(DataStruct));
	dataStructSize = IFCB_ARRAY_SIZE_INC;

	ifcb_data[currIndx].status = status | FLAG_FILE_WRITE;
}

//-----------------------------------------------------------------------------
//	by arriving here, there has been a trigger with 0 or more rois
//	performs all the high-level operations following an image grab
//	called from IfcbDlg.cpp OnFrameReadyCallback
//-----------------------------------------------------------------------------
void ProcessTrigger(unsigned int numRois) {
/*
	// saved by ReadDAQIntegrated
	float			integAlo, integAhi, integClo, integChi;	// four integrated values for each channel

	// saved by StoreBlobs
	long			LLHxloc, LLHyloc;				// lower left hand coords of ROI in pixels
	long			roiSizeX, roiSizeY;				// pixel size of ROI x and Y
	unsigned long	start_byte;						// the start byte of this ROI in the ROI file: first byte is 1 (ONE) - bad!

	// saved here
	unsigned int	trigger;						// number of the ROI in the file: first one is 0
	float			ADC_time;						// timestamp: absolute time in seconds
	unsigned long	grabtimestart,					// using GetTickCount()
	unsigned long	grabtimeend;					// cannot be determined
	float			time_of_flight;					// duration of comparator "on" (width, in us): the CHB LO integrated signal on AD CH8
	float			comparator_out;					// whatever is on AD CH9 - no idea
	unsigned long	StartPoint;						// for data record from chase card if not using a fixed buffer length
	unsigned long	SignalLength;					// for digitizer
	unsigned short	status;
*/
	CString str;

	if (appDebug) {
		if (hasCamera)
			str.Format(_T("Processing trigger %d - %d blobs\r\n"), currTrig, numRois);
		else
			str.Format(_T("Processing trigger %d\r\n"), currTrig);
		DEBUG_MESSAGE_EXT(str);
	}

	if (!outputFiles && (GraphData.GraphType != GraphData.GRAPH_NONE) && GraphData.running) {
		DataStruct Data;
		ReadDAQIntegrated(&Data);					// reads integrators and copies to graph data
	}

	if (outputFiles) {
		ifcb_data[currIndx].status = FLAG_NONE;

		// check for space in array
		if (currIndx + numRois > dataStructSize) {					// need more space
			DEBUG_MESSAGE_EXT(_T("Increasing ifcb_data array size\r\n"));
			ifcb_data[currIndx].status |= FLAG_IFCB_REALLOC;
			void *p = realloc(ifcb_data, (dataStructSize + IFCB_ARRAY_SIZE_INC) * sizeof(DataStruct));			// ask for more memory
			if (p) {
				ifcb_data = (DataStruct *)p;
				dataStructSize += IFCB_ARRAY_SIZE_INC;
			} else
				WriteArraysToFile();
		}

		// store the data
		unsigned int n = max(numRois, 1);							// store one dataset, even if there are no rois
		unsigned int i;

		// fill in all the data for the first record
		ifcb_data[currIndx].time_of_flight = -999.0;
		ReadDAQIntegrated(&ifcb_data[currIndx]);					// reads integrators ONCE & stores data in data struct
		ifcb_data[currIndx].trigger = currTrig;						// number of the ROI in the file: first one is 1
		ifcb_data[currIndx].ADC_time = (float)(triggerTickCount - StartTickCount)/ (float)1000.0;
		ifcb_data[currIndx].comparator_out = -999.0;				// not used
		ifcb_data[currIndx].StartPoint = 0;							// not used
		ifcb_data[currIndx].SignalLength = 0;						// not used
		ifcb_data[currIndx].grabtimestart = triggerTickCount - StartTickCount;	// adjust for actual start of syringe
		if (AcousticFocusing) {
			ifcb_data[currIndx].temperature = Temperature;
			ifcb_data[currIndx].frequency = PZTfrequency;
			ifcb_data[currIndx].Tadjm = Tadjm;
			ifcb_data[currIndx].Tadjb = Tadjb;
		}
		
		for (i = 1; i < n; i++) {									// duplicate other entries in same trigger
			memcpy(&ifcb_data[currIndx + i], &ifcb_data[currIndx], sizeof(DataStruct));
		}
		for (i = 0; i < n; i++)
			StoreBlob(&ifcb_data[currIndx + i], i);					// store the blobs

		// make this just about the last thing done before rearming triggers	
		ifcb_data[currIndx].grabtimeend = GetTickCount() -  StartTickCount;	// this is in real seconds since start of syringe
		for (i = 1; i < n; i++)										// and any others if there were multiple ROIs
			ifcb_data[currIndx + i].grabtimeend = ifcb_data[currIndx].grabtimeend;

		currIndx += n;
	}
	currTrig++;

	// run the sig-gen command here so that it doesn't interfere with data collection from Analog Board
	if (rqPZT) {
		if (DoFreqScan)  {
			ReadHumidTemp();
			PZTcommand(PZT_SCAN);
		}
		else 
			PZTcommand(PZT_RUN);
		rqPZT = false;
	}

	return;
}

//-----------------------------------------------------------------------------
// here is where you put all the stuff to do for a clean+calibration sequence
//-----------------------------------------------------------------------------
void DoCleanCalib(void) {


	// run clorox


	// run beads




	// run biocide

	return;
}