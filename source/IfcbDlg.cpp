//-----------------------------------------------------------------------------
//  IFCB Project
//	IfcbDlg.cpp : implementation file
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Ifcb.h"
#include "IfcbDlg.h"
#include "config.h"
#include "Fluids.h"
#include "Daq.h"
#include "Process.h"
#include "Analysis.h"
#include "FileIo.h"
#include "ControlTabs.h"
#include "GraphTab.h"
#include "tcp.h"
#include "math.h"
#include "sig_gen_api.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// globals
CIfcbDlg	*IfcbHandle;				// handle to the application dialog. Used by external C routines

char		dataPath[STRING_PARAM_LENGTH];
char		fileComment[STRING_PARAM_LENGTH];
int			PZTvolts;
//int			PZThz;
int			PZTfrequency;
double		Tadjm;
double		Tadjb;
int			FreqUpdateInterval;
bool		TemperatureAdjust;
double		StepDuration;
double		mlPerMin;						// for acoustic focusing 
double		minPerSyr;						// for acoustic focusing 
double		minPerStep;						// for acoustic focusing 
double		freqstepsPerSyr;						// for acoustic focusing 
int			syrPerCycle;						// for acoustic focusing 
int			freqstepsPerCycle;						// for acoustic focusing 
bool		DoFreqScan;
double			FreqStep;
double			EndFreq;
double		StartFreq;
double			NewFreq;
int			newHz;
int			stepCount;
bool		appDebug = true;			// if no config file found to say if appDebug, default true shows msgs
bool		viewImages = false;
bool		blobAnalysis = false;
//bool		discreteSampleIntake = false;
bool		backflushWithSample = false;
bool		primeSampleTube = false;
bool		replaceFromSec = false;
bool		stainSample = false;
bool		StainAuto = false;
bool		StainRinseMixingChamber = false;
bool		LockFilterSlider = false;
bool		outputFiles = false;
bool		autoShutdown = false;

bool		debubbleWithSample = false;
bool		debubbleWithSample2 = true;
bool		isStartup = true;
int			nSyringes = 0;
int			beadsInterval;
int			stainTime = 30;
bool		manualBeads;
bool		AcousticFocusingOnALT;	 //rob
bool		runSampleFast;
bool		runSampleFastInitial;
double		runFastFactor;
RUNTYPE		runType = INIT;			// type of syringe

// Housekeeping board reports
double		Humidity, Temperature;
double		Dac[10];
//bool		Flags[16];

// Housekeeping board configuration
int			HKFirmwareVersion;
int 		fanThresh;				// used by DAQ_MCC only
int 		fanHysteresis;			// used by DAQ_MCC only
int			humidThresh;			// used by DAQ_MCC only
int 		humidHysteresis;		// used by DAQ_MCC only
int 		valveDelay;				// used by DAQ_MCC only
int 		flashDelay;				// used by DAQ_MCC only
int 		flashPulseLength;		// used by DAQ_MCC only
int 		cameraPulseLength;		// used by DAQ_MCC only
double		highVoltagePMTA = 0.0;
double		highVoltagePMTB = 0.0;
double		highVoltagePMTC = 0.0;
double		highVoltagePMTA2 = 0.0;
double		highVoltagePMTB2 = 0.0;
double		highVoltagePMTC2 = 0.0;
double		flashVoltage = 0.0;
double		flashVoltage2 = 0.0;
double		pumpVolts;				// used by DAQ_MCC only
bool		pumpDrive = false;		// used by DAQ_MCC only

// Analog board configuration
int			AnalogFirmwareVersion;
int			AcqFileName;
double		trigThreshA;
double		trigThreshB;
double		trigThreshC;
double		trigThreshD;
double		trigThreshA2;
double		trigThreshB2;
double		trigThreshC2;
double		trigThreshD2;
int			integratorSettleTime;
int			triggerPulseMask;
int			triggerPulseMask2;
int			adcPga;
int			adcPga2;
int			adcDrate;
int			primValvePort;
int			secValvePort;
int			triggerTimeout;
int			pulseStretchTime;
int			altInterval;
int			maxPulseStretch;
bool		hasExpertTab;
int			tcpPort;
double		humidityThreshold;
int			pumpState;
int			laserState;
int			fill2bypassFilterState;
int			fill3newSheathState;

double		sampleVolume;			// the volume of sample to draw into the syringe, in millilitres - normal runs
double		sampleVolume2skip;	// the volume of sample to leave behind in the syringe, to prevent degassed air in flowcell
double		sampleVolume2skipALT;	// [ALT config] the volume of sample to leave behind in the syringe, to prevent degassed air in flowcell
double		sampleVolume2;			// the volume of sample to draw into the syringe, in millilitres - alt runs
double		syringeVolume;			// the volume of sample to draw into the syringe, - current run
double		syringeVolume2skip;
int			syr_sampling_speed;
int			base_syr_sampling_speed;
int			altPortNumber;			// 8 is std intake port, 5 is alternate
int			normPortNumber;
double		beadsVolume = 2.50;			// the volume of sample to draw into the syringe, - beads run
#ifdef DAQ_MCC
double gRunTime, gInhibitTime;
#endif
int			syringesDone;	 
bool		fluidSyncBusy;			// true when a FLUIDICS_SYNC operation is in progress
int			samplePortNumber;
int			currentValvePosition;
int			desiredValvePosition;
int			ValveDirection;


unsigned long	triggerTickCount;					// the tick count when the camera callback occurred
unsigned int	currTrig;							// the current trigger, globally available
unsigned int	currIndx;							// the current ADC file index, globally available

DataStruct		*ifcb_data = NULL;					// expands as needed
unsigned long	dataStructSize;						// the capacity of the ifcb_data array, in records
uint8			*ROIArray = NULL;					// expands as needed
unsigned long	ROIArrayPtr;						// pointer into the file
unsigned int	ROIRamPtr;							// pointer into the local ram array
int				imagerID;							// imager ID
unsigned int	minimumBlobArea;					// ignore blobs with area smaller than this value
unsigned long	acqStartTimeout;					// timeout for first camera trigger
bool			hasCamera;							// set to false when running without camera
bool			AcousticFocusing;							// for ALT samples
extern int		processcounter = 0;
//-----------------------------------------------------------------------------
CIfcbDlg::CIfcbDlg(CWnd* pParent) : CDialog(CIfcbDlg::IDD, pParent) {

    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//-----------------------------------------------------------------------------
CIfcbDlg::~CIfcbDlg() {

}

//-----------------------------------------------------------------------------
//	Tidying-up functions to perform before destroying window and dialog
//-----------------------------------------------------------------------------
BOOL CIfcbDlg::DestroyWindow() {

	ChangeHWParam(PMTA, 0.0);
	ChangeHWParam(PMTB, 0.0);
	ChangeHWParam(PMTC, 0.0);
	IfcbHandle->dacsOn = false;
	DAQShutdown();
	FluidicsClose();						// serial port
	CloseTcp();

	free(ROIArray);

	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK6, OutputFiles);
	DDX_Control(pDX, IDC_CHECK7, DebugCheckBox);
	DDX_Control(pDX, IDC_EDIT1, DebugWindow);
	DDX_Control(pDX, IDC_BUTTON13, ClearDebug);
	DDX_Control(pDX, IDC_TAB1, TabContainer);
	DDX_Control(pDX, IDC_RUN, RunButton);
	DDX_Control(pDX, IDC_TRIGGER, TriggerButton);
	DDX_Control(pDX, IDC_TRIG_CONT, TriggerContinuous);
	DDX_Control(pDX, IDC_CHECK10, AutoshutdownCheckBox);
	DDX_Control(pDX, IDC_EDIT_NSYRINGES, nSyringesEditBox);
	DDX_Control(pDX, IDC_SYRINGE_SPIN, nSyringesSpinner);
	DDX_Control(pDX, IDC_CHECK11, BlobAnalysis);
	//DDX_Control(pDX, IDC_STAIN_SAMPLE, StainSampleCheckBox);
}


BEGIN_MESSAGE_MAP(CIfcbDlg, CDialog)
	ON_WM_TIMER()
    ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK6, &CIfcbDlg::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK7, &CIfcbDlg::OnBnClickedCheck7)
	ON_BN_CLICKED(IDC_BUTTON13, &CIfcbDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_RUN, &CIfcbDlg::OnBnClickedRun)
	ON_BN_CLICKED(IDC_TRIGGER, &CIfcbDlg::OnBnClickedTrigger)
	ON_BN_CLICKED(IDC_TRIG_CONT, &CIfcbDlg::OnBnClickedTrigCont)
	ON_EN_MAXTEXT(IDC_EDIT1, &CIfcbDlg::OnBnClickedButton13)	// clear the debug window when it's maxed out
	ON_BN_CLICKED(IDC_CHECK10, &CIfcbDlg::OnBnClickedCheck10)
	ON_EN_KILLFOCUS(IDC_EDIT_NSYRINGES, &CIfcbDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_CHECK11, &CIfcbDlg::OnBnClickedCheck11)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CIfcbDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BOOL CIfcbDlg::OnInitDialog() {

    CDialog::OnInitDialog();
//    SetIcon(m_hIcon, TRUE);
//    SetIcon(m_hIcon, FALSE);
	IfcbHandle = this;

	// Tabbed windows
	int i = 0;
	if (hasCamera)
		TabContainer.InsertItem(i++, _T("Camera"));
	TabContainer.InsertItem(i++, _T("Fluids"));
	TabContainer.InsertItem(i++, _T("Hardware"));
	TabContainer.InsertItem(i++, _T("Graphs"));
	if (hasExpertTab)
		TabContainer.InsertItem(i++, _T("Expert"));
#ifdef IFCB8_BENCHTOP
	TabContainer.InsertItem(i++, _T("Diagnostics"));
#endif
	TabContainer.Init();

	// configuration check boxes
	if (appDebug)
		DebugCheckBox.SetCheck(BST_CHECKED);
	if (outputFiles)
		OutputFiles.SetCheck(BST_CHECKED);
	if (autoShutdown)
		AutoshutdownCheckBox.SetCheck(BST_CHECKED);
	if (hasCamera && blobAnalysis)
		BlobAnalysis.SetCheck(BST_CHECKED);
	if (!hasCamera)
		BlobAnalysis.ShowWindow(false);
	grabbing = false;
	nSyringesSpinner.SetRange(0, 32000);
	nSyringesSpinner.SetPos(nSyringes);

	// debug window
#ifdef IFCB
	ERROR_MESSAGE(_T("IFCB Control Software - IFCB version\r\n\r\n"));
#endif
#ifdef IFCB8_BENCHTOP
	ERROR_MESSAGE(_T("IFCB Control Software - IFCB8_BENCHTOP version\r\n\r\n"));
#endif

	// hardware initialisation
	FluidicsSerialInit();	 // init serial port for whatever pump is used

#ifdef IFCB8_BENCHTOP
	FluidicsRoutine(FLUIDICS_STOP, 0);	// stop the syringe if it's running
	FluidicsRoutine(FLUIDICS_INIT, 0);	// initialize syringe
#endif	

	DAQInit();									// init whichever DAQ IO card is installed

	if (AcousticFocusing) {
		//siggen_test(true);						// for test only - comment out if hardware is present
		//if (!siggen_initialize())
		//	ERROR_MESSAGE(_T("Failed to init siggen\r\n"));
		//if (!siggen_setCalibrationFile("AcousticCal.txt"))
		//	ERROR_MESSAGE(_T("Failed to set cal file\r\n"));
	}

	SetTimer(AUTOSTART_TIMER_ID, AUTOSTART_DELAY * 1000, NULL);	// set a timer that fires after startup
	if (!hasCamera){
		SetTimer(CAPTURE_TIMER_ID, 2, NULL);	// used to check to see if the Analog board has triggered
	} else {
		SetTimer(FRAMERATE_TIMER_ID, 5000, NULL);	// set a timer that fires every 5s
		SetTimer(FRAMECOUNT_TIMER_ID, 1000, NULL);	// set a timer that fires every s
	}
	SetTimer(TCP_TIMER_ID, 50, NULL);

	rqStop = false;

	return TRUE;
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnTimer(UINT_PTR nIDEvent) {

	static UINT32 prevCount = 0;
	CString fpsText;
	unsigned int fps;
	CString str;
	//int stepCount;
	char cval[20];
	CString valstr;
	CStringToChar(cval, &valstr);
	CString args;		
	CString args2;	

	switch (nIDEvent) {
		case SYRINGE_TIMER_ID:
			DEBUG_MESSAGE(_T("Syringe timeout fired\r\n"));
			AcqComplete(END_AUTO);						// finish this run
			if (++syringesDone < nSyringes) {			// start a new run if required
				if (!AcqInit())
					Stop();
			} else {									// release the start button
				RunButton.SetCheck(BST_UNCHECKED);
				grabbing = false;						// this stops the data files being written twice
				OnBnClickedRun();
				if (autoShutdown)
					ShutdownWindows();
			}
			break;

		case TRIGGER_TIMER_ID:
			GenerateTrigger();
			break;

		case AUTOSTART_TIMER_ID:
			KillTimer(AUTOSTART_TIMER_ID);
			if (nSyringes) {							// start a run if required
				RunButton.SetCheck(BST_CHECKED);
				OnBnClickedRun();
			}
			break;

		case ACQ_START_TIMEOUT_ID:
			DEBUG_MESSAGE(_T("Trigger timeout fired\r\n"));
			AcqComplete(END_RETRY);						// finish this run
			if (!AcqInit())								// start a new run
				Stop();
			break;

		case FRAMERATE_TIMER_ID:
			if (!grabbing)
				break;
			UINT32_TYPE thisCount;
			thisCount = TabContainer.tabPages[cameraTab]->GetFrameCount();
//			TabContainer.tabPages[cameraTab]->SetDlgItemInt(IDC_FRAMEDROPPEDCOUNT, (thisCount - prevCount) * 2, FALSE);
			fps = (thisCount - prevCount) * 2;
			prevCount = thisCount;
			fpsText.Format(_T("%d.%d"), fps / 10, fps % 10);
			TabContainer.tabPages[cameraTab]->SetDlgItemText(IDC_FRAMEDROPPEDCOUNT, fpsText);
			break;

		case FRAMECOUNT_TIMER_ID:
			if (!grabbing)
				break;
			TabContainer.tabPages[cameraTab]->SetDlgItemInt(IDC_FRAMECOUNT, TabContainer.tabPages[cameraTab]->GetFrameCount(), FALSE);
			break;

		case TCP_TIMER_ID:
			HandleTcp();
			break;

		case PZT_TIMER_ID:							// synchronise the PZT command to only happen directly after a capture
			rqPZT = true;
			//PZTcommand(PZTmode RUN);
			break;

		case PZT_SCAN_TIMER_ID:
			rqPZT = true; // at 492, add this here to see if it fixes eventual-crashing problem with scanning
			str.Format(_T("in scan timer; stepCount = %d\r\n"), stepCount);
				DEBUG_MESSAGE_EXT(str); 
				//To DoFreqScan,
				//we want a loop using a counter for FreqStep (e.g.,StepCounter), 
				//so that at each StepCounter, FreqStep is added to NewFreq. 
				//This loop needs to extend over multiple syringes (i.e., using 
				//freqstepsPerSyr = minPerSyr / minPerStep (from Fileio.cpp) 
				//to keep track of when to go back to StartFreq.
				//We want the loop to continue indefinitely so that multiple scans
				//can be made as instrument temperature is varied (for calibration).

				if (stepCount == 1) {
					PZTfrequency = (int)(StartFreq + FreqStep);
				}
				else if(stepCount % freqstepsPerCycle == 0) {
					PZTfrequency = (int)StartFreq;
				}
				else {
					PZTfrequency = PZTfrequency + (int)FreqStep;
				}
				WriteFreqFile();
					/*args.Format(_T(" --nogui --on --noinit --hz %d --volts %d"), PZTfrequency, PZTvolts);
					DEBUG_MESSAGE_EXT(args); 
					DEBUG_MESSAGE_EXT(_T("\r\n"));
					str = sigGenPath; // note -- this includes the file name as well as path (e.g., "C:\AcousticFocusingWHOI\sig_gen\sig-gen-v1.0.4_64bit\sig-gen-v1.0.4\sig-gen")
					//DEBUG_MESSAGE_EXT(str); 
					DEBUG_MESSAGE_EXT(_T("\r\n"));
					//args2.Format(_T("floor(freqstepsPerSyr) = %f"), floor(freqstepsPerSyr)); 
					args2.Format(_T("freqstepsPerCycle = %d"), int(floor(freqstepsPerCycle))); 
					DEBUG_MESSAGE_EXT(args2); 
					DEBUG_MESSAGE_EXT(_T("\r\n"));
					//args.Format(_T(""));
					//str = "%windir\\system32\\notepad.exe";
					//str += (args);
					//DEBUG_MESSAGE_EXT(str);
					//DEBUG_MESSAGE_EXT(_T("\r\n")); 
					ShellExecute(NULL, _T("open"), str, args, NULL, SW_HIDE);	
					//ShellExecute(NULL, _T("open"), str, args, NULL, SW_NORMAL);	
					*/
					stepCount = stepCount + 1;
					//return;
			rqPZT =  true;
			//}
			break;
			

		case CAPTURE_TIMER_ID:						// used when hasCamera == false
			if (rqStop) {
				Stop();
				break;
			}

			if (!grabbing)
				break;
			
			if (!CheckForCapture()) {
				if (rqStop)
					Stop();
				break;
			}

			KillTimer(ACQ_START_TIMEOUT_ID);
			triggerTickCount = GetTickCount();
			ProcessTrigger(0);
			SetHWParams();							// change any parameters that need it

			// alignment graph plot - takes about 0.5 ms
			if (GraphData.GraphType != GraphData.GRAPH_NONE) {
				TabContainer.tabPages[graphTab]->UpdateGraph();
				TabContainer.tabPages[graphTab]->PostMessage(WM_PAINT, 0, 0);
			}

			DaqArmTrigger();
			if (rqStop)
				Stop();
			break;

		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
//-----------------------------------------------------------------------------
HCURSOR CIfcbDlg::OnQueryDragIcon() {

    return static_cast<HCURSOR>(m_hIcon);
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedCheck6() {		// output files

	outputFiles = (OutputFiles.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedCheck7() {		// debug

	appDebug = (DebugCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedButton13() {		// clear debug window

	DebugWindow.SetSel(0, -1, false);		// select all
	DebugWindow.SetReadOnly(false);
	DebugWindow.Clear();					// and clear
	DebugWindow.SetReadOnly(true);
}

//-----------------------------------------------------------------------------
void CIfcbDlg::CreateSyringeNumString(CString *string) {

	if (grabbing) {
		string->Format(_T("Syringe #%d : "), syringesDone + 1);
		switch (runType) {
			case INIT:
			case NORMAL:
				*string += _T("NORMAL");
				break;
			case ALT:
				*string += _T("ALT");
				break;
			case BEADS:
				*string += _T("BEADS");
				break;
			default:
				break;
		}
	} else
		*string = "No syringe running";
}

//-----------------------------------------------------------------------------
void CIfcbDlg::WriteSyringeNum() {

	CString label;

	CreateSyringeNumString(&label);
	SetDlgItemText(IDC_CURRENT_SYRINGE, label);
}

//-----------------------------------------------------------------------------
void CIfcbDlg::Run() {

	RunButton.EnableWindow(FALSE);								// Disable the Run button

	if (hasCamera) {
		if (!TabContainer.tabPages[cameraTab]->Run(true)) {		// Camera error
			RunButton.SetCheck(BST_UNCHECKED);
			RunButton.EnableWindow(TRUE);						// Enable the Run button
			return;
		}
	}

	RunButton.SetCheck(BST_CHECKED);
	grabbing = true;
	DEBUG_MESSAGE(_T("Acquisition started\r\n"));
	RunButton.SetWindowText(_T("Stop Acq"));
	TabContainer.tabPages[fluidsTab]->Run(true);
	if (hasExpertTab)
		TabContainer.tabPages[expertTab]->Run(true);
	syringesDone = 0;
	if (!AcqInit())												// start the hardware & draw a new sample
		Stop();
	TabContainer.tabPages[hardwareTab]->Run(true);

	RunButton.EnableWindow(TRUE);								// Enable the Run button
}

//-----------------------------------------------------------------------------
void CIfcbDlg::Stop() {

	if (hasCamera)
		TabContainer.tabPages[cameraTab]->Run(false);

	TabContainer.tabPages[fluidsTab]->Run(false);
	if (hasExpertTab)
		TabContainer.tabPages[expertTab]->Run(false);
	DEBUG_MESSAGE(_T("Acquisition stopped\r\n"));
	RunButton.SetWindowText( _T("Start Acq") );
//	KillTimer(ACQ_START_TIMEOUT_ID);
	if (grabbing) {
		grabbing = false;
		AcqComplete(END_MANUAL);
	}
	RunButton.SetCheck(BST_UNCHECKED);
	SetDlgItemText(IDC_CURRENT_SYRINGE, _T(""));
	TabContainer.tabPages[hardwareTab]->Run(false);
	rqStop = false;
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedRun() {

	if (RunButton.GetCheck() == BST_CHECKED) { 					// Run
		KillTimer(AUTOSTART_TIMER_ID);
		Run();
	} else {
		if (hasCamera)
			Stop();
		else													// have to synchronise stop with the capture timer
			rqStop = true;
	}
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedTrigger() {

	GenerateTrigger();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedTrigCont() {

    if ((TriggerContinuous.GetCheck() == BST_CHECKED) && (RunButton.GetCheck() == BST_CHECKED)) {
		DEBUG_MESSAGE(_T("Triggering\r\n"));
		TriggerContinuous.SetWindowText( _T("Triggering") );
		SetTimer(TRIGGER_TIMER_ID, 500, NULL);
	} else {												// Stop
		TriggerContinuous.SetCheck(BST_UNCHECKED);
		TriggerContinuous.SetWindowText( _T("Trigger cont") );
		KillTimer(TRIGGER_TIMER_ID);
	}
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedCheck10() {

	autoShutdown = (AutoshutdownCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::ShutdownWindows() {

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

   HANDLE hToken; 
   TOKEN_PRIVILEGES tkp; 
 
	// Get a token for this process. 
 	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	   return;
 
	// Get the LUID for the shutdown privilege. 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
    tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
	// Get the shutdown privilege for this process. 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
 
	if (GetLastError() != ERROR_SUCCESS) 
	   return;
 
	// Shut down the system and force all applications to close. 
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED)) 
		return;

   //shutdown was successful
	return;
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnEnChangeEdit2() {

	if (!nSyringesSpinner)
		return;
	nSyringes = nSyringesSpinner.GetPos();
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CIfcbDlg::OnBnClickedCheck11() {

	blobAnalysis = (BlobAnalysis.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}


void CIfcbDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
