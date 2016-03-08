//-----------------------------------------------------------------------------
//  IFCB Project
//	FluidsTab.cpp
//	Martin Cooper
//-----------------------------------------------------------------------------

#include "config.h"
#include "stdafx.h"
#include "FluidsTab.h"
#include "Fluids.h"
#include "IfcbDlg.h"
#include "FileIo.h"

bool		fluidicsVerbose = false;
bool		fluidsActive = false;

IMPLEMENT_DYNAMIC(CFluidsTab, CDialog)

//-----------------------------------------------------------------------------
CFluidsTab::CFluidsTab(CWnd* pParent) : CTab(CFluidsTab::IDD, pParent) {

}

//-----------------------------------------------------------------------------
CFluidsTab::~CFluidsTab() {

}

//-----------------------------------------------------------------------------
void CFluidsTab::DoDataExchange(CDataExchange* pDX) {

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FLUID_ACTIVE, FluidActive);
	DDX_Control(pDX, IDC_VERBOSE, VerboseCheckBox);
	DDX_Control(pDX, IDC_FLUIDS_INIT, ButFluidsInit);
	DDX_Control(pDX, IDC_FLUIDS_STOP, ButFluidsStop);
	DDX_Control(pDX, IDC_SYRINGE0, ButSyringe0);
//	DDX_Control(pDX, IDC_DEBUBBLE, ButDebubble);
	DDX_Control(pDX, IDC_DEBUBBLE_AND_REFILL, ButDebubbleAndRefill);
	DDX_Control(pDX, IDC_DEBUBBLE_AND_REFILL2, ButDebubbleAndRefill2);
	DDX_Control(pDX, IDC_RUN_SAMPLE, ButRunSample);
	//DDX_Control(pDX, IDC_RUN_SAMPLE_FAST, ButRunSampleFast);
	//DDX_Control(pDX, IDC_RUN_SAMPLE_FAST, RunSampleFastCheckBox);
	DDX_Control(pDX, IDC_DRAIN_FILTERS, ButDrainFilters);
	DDX_Control(pDX, IDC_ADD_STAIN, ButAddStain);
	DDX_Control(pDX, IDC_SAMPLE2CONE, ButSample2Cone);
	DDX_Control(pDX, IDC_RINSE_STAIN, ButRinseStain);
	DDX_Control(pDX, IDC_EMPTY_MIXING_CHAMBER, ButEmptyMixingChamber);
	DDX_Control(pDX, IDC_FLUSH, ButFlush);
	//DDX_Control(pDX, IDC_DISCRETE, DiscreteSample);
	//DDX_Control(pDX, IDC_BACKFLUSH_WITH_SAMPLE, BackflushWithSampleCheckbox);
	DDX_Control(pDX, IDC_AZIDE, ButAzide);
	DDX_Control(pDX, IDC_CLOROX, ButClorox);
	DDX_Control(pDX, IDC_BEADS, BeadsButton);
	DDX_Control(pDX, IDC_DEBUBBLE_WITH_SAMPLE, DebubbleWithSampleCheckBox);
	DDX_Control(pDX, IDC_DEBUBBLE_WITH_SAMPLE2, DebubbleWithSample2CheckBox);
	DDX_Control(pDX, IDC_BACKFLUSH_WITH_SAMPLE, BackflushWithSampleCheckBox);
	DDX_Control(pDX, IDC_PRIME_SAMPLE_TUBE, PrimeSampleTubeCheckBox);
	DDX_Control(pDX, IDC_REPLACEFROMSEC, ReplaceFromSecCheckBox);
	DDX_Control(pDX, IDC_RUN_SAMPLE_FAST, RunSampleFastCheckBox);
	DDX_Control(pDX, IDC_STAIN_SAMPLE, StainSampleCheckBox);
	DDX_Control(pDX, IDC_STAIN_SAMPLE2, StainAutoCheckBox);
	DDX_Control(pDX, IDC_STAIN_SAMPLE3, StainRinseMixingChamberCheckBox);
	DDX_Control(pDX, IDC_STAIN_SAMPLE4, LockFilterSliderCheckBox);
	DDX_Control(pDX, IDC_STAIN_SAMPLE5, AcousticFocusingCheckBox);
	DDX_Control(pDX, IDC_EDIT1, SampVolume);
	DDX_Control(pDX, IDC_EDIT3, SampleVolume2);
	DDX_Control(pDX, IDC_Vol2skip, SampleVolume2skip);
	DDX_Control(pDX, IDC_Vol2skipALT, SampleVolume2skipALT);
	DDX_Control(pDX, IDC_BEADSINT, BeadsInterval);
	DDX_Control(pDX, IDC_STAINTIME, StainTime);
	DDX_Control(pDX, IDC_BEADSVOL, BeadsVolume);
	DDX_Control(pDX, IDC_ALT, AltLabel);
	DDX_Control(pDX, IDC_EDIT4, RunFastFactor);
	DDX_Control(pDX, IDC_AltPort, ButAltPort);
	DDX_Control(pDX, IDC_NormPort, ButNormPort);
}

//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CFluidsTab, CDialog)
	ON_BN_CLICKED(IDC_FLUIDS_INIT, &CFluidsTab::OnBnClickedFluidsInit)
	ON_BN_CLICKED(IDC_SYRINGE0, &CFluidsTab::OnBnClickedSyringe0)
	ON_BN_CLICKED(IDC_SYRINGE1, &CFluidsTab::OnBnClickedSyringe1)
	ON_BN_CLICKED(IDC_FLUIDS_STOP, &CFluidsTab::OnBnClickedFluidsStop)
//	ON_BN_CLICKED(IDC_DEBUBBLE, &CFluidsTab::OnBnClickedDebubble)
	ON_BN_CLICKED(IDC_DEBUBBLE_AND_REFILL, &CFluidsTab::OnBnClickedDebubbleAndRefill)
	ON_BN_CLICKED(IDC_DEBUBBLE_AND_REFILL2, &CFluidsTab::OnBnClickedDebubbleAndRefill2) // for DebubbleCartridge
	ON_BN_CLICKED(IDC_RUN_SAMPLE, &CFluidsTab::OnBnClickedRunSample)
	ON_BN_CLICKED(IDC_RUN_SAMPLE_FAST, &CFluidsTab::OnBnClickedRunSampleFast)
	ON_BN_CLICKED(IDC_DRAIN_FILTERS, &CFluidsTab::OnBnClickedDrainFilters)
	ON_BN_CLICKED(IDC_FILL_FILTERS, &CFluidsTab::OnBnClickedAddStain)
	ON_BN_CLICKED(IDC_SAMPLE2CONE, &CFluidsTab::OnBnClickedSample2Cone)
	ON_BN_CLICKED(IDC_RINSE_STAIN, &CFluidsTab::OnBnClickedRinseStain)
	ON_BN_CLICKED(IDC_FLUSH, &CFluidsTab::OnBnClickedFlush)
	ON_BN_CLICKED(IDC_VERBOSE, &CFluidsTab::OnBnClickedVerbose)
	ON_BN_CLICKED(IDC_FLUID_ACTIVE, &CFluidsTab::OnBnClickedFluidActive)
	ON_BN_CLICKED(IDC_BACKFLUSH_WITH_SAMPLE, &CFluidsTab::OnBnClickedBackflushWithSample)
	ON_BN_CLICKED(IDC_PRIME_SAMPLE_TUBE, &CFluidsTab::OnBnClickedPrimeSampleTube)
	ON_BN_CLICKED(IDC_REPLACEFROMSEC, &CFluidsTab::OnBnClickedReplaceFromSec)
	//ON_BN_CLICKED(IDC_DISCRETE, &CFluidsTab::OnBnClickedDiscrete)
	ON_BN_CLICKED(IDC_SAMP_VOL_SET, &CFluidsTab::OnBnClickedSampVolSet)
	ON_BN_CLICKED(IDC_AZIDE, &CFluidsTab::OnBnClickedAzide)
	ON_BN_CLICKED(IDC_CLOROX, &CFluidsTab::OnBnClickedClorox)
	ON_BN_CLICKED(IDC_DEBUBBLE_WITH_SAMPLE, &CFluidsTab::OnBnClickedDebubbleWithSample)
	ON_BN_CLICKED(IDC_DEBUBBLE_WITH_SAMPLE2, &CFluidsTab::OnBnClickedDebubbleWithSample2)
	ON_BN_CLICKED(IDC_BEADS, &CFluidsTab::OnBnClickedBeads)
	ON_BN_CLICKED(IDC_NormPort, &CFluidsTab::OnBnClickedNormPort)
	ON_BN_CLICKED(IDC_AltPort, &CFluidsTab::OnBnClickedAltPort)
	ON_BN_CLICKED(IDC_STAIN_SAMPLE, &CFluidsTab::OnBnClickedStainSample)
	ON_BN_CLICKED(IDC_STAIN_SAMPLE2, &CFluidsTab::OnBnClickedStainAuto)
	ON_BN_CLICKED(IDC_STAIN_SAMPLE3, &CFluidsTab::OnBnClickedStainRinseMixingChamber)
	ON_BN_CLICKED(IDC_STAIN_SAMPLE4, &CFluidsTab::OnBnClickedLockFilterSlider)
	ON_BN_CLICKED(IDC_EMPTY_MIXING_CHAMBER, &CFluidsTab::OnBnClickedEmptyMixingChamber)
	ON_BN_CLICKED(IDC_STAIN_SAMPLE5, &CFluidsTab::OnBnClickedStainSample5)
	ON_BN_CLICKED(IDC_DEBUBBLE_AND_REFILL2, &CFluidsTab::OnBnClickedDebubbleAndRefill2)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
BOOL CFluidsTab::OnInitDialog() {

	CDialog::OnInitDialog();

	#ifdef DAQ_MCC
	//  comment out these lines to restore the buttons
	//ButAddStain.ShowWindow(false);
	//ButDrainFilters.ShowWindow(false);
	//ButFlush.ShowWindow(false);
	//ButSample2Cone.ShowWindow(false);
	ButRunSample.ShowWindow(false);
	//ButSample2Cone.ShowWindow(false);
	ButFluidsStop.ShowWindow(false);
	//ButFluidsInit.ShowWindow(false);
	//DiscreteSample.ShowWindow(false);
	//BackflushWithSample.ShowWindow(false);
	//BeadsVolume.ShowWindow(false);
	
	#endif

	fluidSyncBusy = false;
	//runSampleFast = false;  // let runSampleFast be determined by cfg file

	return TRUE;
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnShowTab() {

	CString valstr;

	if (fluidicsVerbose)
		VerboseCheckBox.SetCheck(BST_CHECKED);
	if (fluidsActive)
		FluidActive.SetCheck(BST_CHECKED);
	if (debubbleWithSample)
		DebubbleWithSampleCheckBox.SetCheck(BST_CHECKED);
	if (debubbleWithSample2)
		DebubbleWithSample2CheckBox.SetCheck(BST_CHECKED);
	if (backflushWithSample)
		BackflushWithSampleCheckBox.SetCheck(BST_CHECKED);
	if (primeSampleTube)
		PrimeSampleTubeCheckBox.SetCheck(BST_CHECKED);	
	if (stainSample)
		StainSampleCheckBox.SetCheck(BST_CHECKED);
	if (StainAuto) 
		StainAutoCheckBox.SetCheck(BST_CHECKED);
	if (StainRinseMixingChamber) 
		StainRinseMixingChamberCheckBox.SetCheck(BST_CHECKED);
	if (LockFilterSlider) 
		LockFilterSliderCheckBox.SetCheck(BST_CHECKED);
	if (replaceFromSec)
		ReplaceFromSecCheckBox.SetCheck(BST_CHECKED);
	if (runSampleFast)
		RunSampleFastCheckBox.SetCheck(BST_CHECKED);
	if (AcousticFocusing)
		AcousticFocusingCheckBox.SetCheck(BST_CHECKED);

#ifdef IFCB8_BENCHTOP
	if (discreteSampleIntake)
		DiscreteSample.SetCheck(BST_CHECKED);
//#else
//	DiscreteSample.ShowWindow(false);
#endif

	valstr.Format(_T("%.2f"), sampleVolume);
	SampVolume.SetSel(0, -1);
	SampVolume.ReplaceSel(valstr, false);

	valstr.Format(_T("%.2f"), sampleVolume2skip);
	SampleVolume2skip.SetSel(0, -1);
	SampleVolume2skip.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), beadsInterval);
	BeadsInterval.SetSel(0, -1);
	BeadsInterval.ReplaceSel(valstr, false);

	valstr.Format(_T("%d"), stainTime);
	StainTime.SetSel(0, -1);
	StainTime.ReplaceSel(valstr, false);

/*	valstr.Format(_T("%d"), runFastFactor);
	RunFastFactor.SetSel(0, -1);
	RunFastFactor.ReplaceSel(valstr, false);
*/
	valstr.Format(_T("%.2f"), beadsVolume);
	BeadsVolume.SetSel(0, -1);
	BeadsVolume.ReplaceSel(valstr, false);

	valstr.Format(_T("%.1f"), runFastFactor);
	RunFastFactor.SetSel(0, -1);
	RunFastFactor.ReplaceSel(valstr, false);

	ButNormPort.ShowWindow(true);

	if (altInterval) {
		SampleVolume2.ShowWindow(true);
		AltLabel.ShowWindow(true);
		valstr.Format(_T("%.2f"), sampleVolume2);
		SampleVolume2.SetSel(0, -1);
		SampleVolume2.ReplaceSel(valstr, false);
		ButAltPort.ShowWindow(true);
		
		SampleVolume2skipALT.ShowWindow(true);
		valstr.Format(_T("%.2f"), sampleVolume2skipALT);
		SampleVolume2skipALT.SetSel(0, -1);
		SampleVolume2skipALT.ReplaceSel(valstr, false);
	} else {
		SampleVolume2.ShowWindow(false);
		AltLabel.ShowWindow(false);
		SampleVolume2skipALT.ShowWindow(false);
		ButAltPort.ShowWindow(false);

	}

	BeadsButton.SetCheck(manualBeads ? BST_CHECKED : BST_UNCHECKED);
}

//-----------------------------------------------------------------------------
bool CFluidsTab::Run(bool run) {

	if (run) {												// Run
		RunFastFactor.EnableWindow(FALSE);
		//utRunSampleFast.EnableWindow(FALSE);
		//RunSampleFastCheckbox.EnableWindow(FALSE);
		return true;
	} else {												// Stop 
		RunFastFactor.EnableWindow(TRUE);
		//ButRunSampleFast.EnableWindow(TRUE);
		//RunSampleFastCheckbox.EnableWindow(TRUE);
		return false;
	}
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedNormPort() {
	// toggle between Alt and Std sample port for alt sampling
	if (ButNormPort.GetCheck() == BST_CHECKED) {		// enable
        ButNormPort.SetWindowText(_T("Secondary"));
		normPortNumber = secValvePort;
		//triggerPulseMask |= 1;
	} else {										// disable
		ButNormPort.SetWindowText( _T("Primary") );
		normPortNumber = primValvePort;
	}
	WriteCfgFile();
}
//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedAltPort() {
	// toggle between Alt and Std sample port for alt sampling
	if (ButAltPort.GetCheck() == BST_CHECKED) {		// enable
        ButAltPort.SetWindowText(_T("Secondary"));
		altPortNumber = secValvePort;
		//triggerPulseMask |= 1;
	} else {										// disable
		ButAltPort.SetWindowText( _T("Primary") );
		altPortNumber = primValvePort;
	}
	WriteCfgFile();
}
//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedFluidsInit() {

	// initialize the pump
	FluidicsRoutine(FLUIDICS_INIT, 0);
	DEBUG_MESSAGE_EXT(_T("Done initializing syringe pump\r\n"));
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedSyringe0() {

	if (MessageBox(_T("This will zero the syringe - continue?"), _T("WARNING!"), MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
		FluidicsRoutine(FLUIDICS_SET_SYRINGE_OFFSET, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedSyringe1() {

	//if (MessageBox(_T("This will move the filter slider to OUT position - continue?"), _T("WARNING!"), MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
		//FluidicsRoutine(FLUIDICS_SET_FILTER_OFFSET, 0);
		FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_OUT, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedFluidsStop() {

	FluidicsRoutine(FLUIDICS_STOP, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedDebubble() {

	FluidicsRoutine(FLUIDICS_DEBUBBLE, 0);
}
//-----------------------------------------------------------------------------

void CFluidsTab::OnBnClickedDebubbleAndRefill() {
	if (MessageBox(_T("Ensure that sample tube is immersed in sample - continue?"), _T("WARNING!"), MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
		FluidicsRoutine(FLUIDICS_DEBUBBLE_AND_REFILL, 0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CFluidsTab::OnBnClickedDebubbleAndRefill2() {
	if (MessageBox(_T("Before debubbling cartridge, turn off sheath.  *NOTE*: this is via Port 5 (not for standard IFCBs)"), _T("WARNING!"), MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
		FluidicsRoutine(FLUIDICS_DEBUBBLE_AND_REFILL2, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedRunSample() {

	FluidicsRoutine(FLUIDICS_RUN_SAMPLE, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedDrainFilters() {

	//FluidicsRoutine(FLUIDICS_DRAIN_FILTERS, 0);
	FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_IN, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedAddStain() { // now used for Add stain to sample -- routes sample flow through stain loop

	//FluidicsRoutine(FLUIDICS_FILL_FILTERS, 0);
	FluidicsRoutine(FLUIDICS_ADD_STAIN, 0);
}

//-----------------------------------------------------------------------------
/*void CFluidsTab::OnBnClickedSample2Cone() {
	//DEBUG_MESSAGE_EXT(_T("This button currently does nothing\r\n"));

	FluidicsRoutine(FLUIDICS_SAMPLE2CONE, 0);
}
*/
//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedFlush() {

	FluidicsRoutine(FLUIDICS_FLUSH_SAMPLETUBE, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedVerbose() {

	fluidicsVerbose = (VerboseCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedFluidActive() {

	fluidsActive = (FluidActive.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
//void CFluidsTab::OnBnClickedBackflushWithSample() {			// discrete sample

//	backflushWithSample = (BackflushWithSampleCheckbox.GetCheck() == BST_CHECKED);
//	WriteCfgFile();
//}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedSampVolSet() {

	CString valstr;
	char cval[20];

	// Sample volume
	// need something here to limit volumes to 0.2 to 5 ml, or reasonable values
	SampVolume.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &sampleVolume);

	// re-format the input field
	valstr.Format(_T("%.2f"), sampleVolume);
	SampVolume.SetSel(0, -1);
	SampVolume.ReplaceSel(valstr, false);

	RunFastFactor.GetWindowTextW(valstr); //Rob - added this to save value in cfg file.
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &runFastFactor);
	// re-format the input field
	valstr.Format(_T("%.1f"), runFastFactor);
	RunFastFactor.SetSel(0, -1);
	RunFastFactor.ReplaceSel(valstr, false);


	SampleVolume2skip.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &sampleVolume2skip);
	if (sampleVolume2skip >= sampleVolume) {
		sampleVolume2skip = 0;
		DEBUG_MESSAGE_EXT(_T("volume to skip must be less than Sample volume...\r\n"))
	}

	// re-format the input field
	valstr.Format(_T("%.2f"), sampleVolume2skip);
	SampleVolume2skip.SetSel(0, -1);
	SampleVolume2skip.ReplaceSel(valstr, false);

	// alt values
	SampleVolume2.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &sampleVolume2);
	if (sampleVolume2 == 5) { 
			sampleVolume2 = 4.98; // to allow for 20 ul stain to be added.
		}

	SampleVolume2skipALT.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &sampleVolume2skipALT);
	if (sampleVolume2skipALT >= sampleVolume2) {
		sampleVolume2skipALT = 0;
		DEBUG_MESSAGE_EXT(_T("volume to skip must be less than Sample volume...\r\n"))
	}

	// re-format the input field
	valstr.Format(_T("%.2f"), sampleVolume2);
	SampleVolume2.SetSel(0, -1);
	SampleVolume2.ReplaceSel(valstr, false);

	valstr.Format(_T("%.2f"), sampleVolume2skipALT);
	SampleVolume2skipALT.SetSel(0, -1);
	SampleVolume2skipALT.ReplaceSel(valstr, false);


	// beads volume
	BeadsVolume.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %lf", &beadsVolume);


	// re-format the input field
	valstr.Format(_T("%.2f"), beadsVolume);
	BeadsVolume.SetSel(0, -1);
	BeadsVolume.ReplaceSel(valstr, false);

	// beads interval
	BeadsInterval.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &beadsInterval);

	StainTime.GetWindowTextW(valstr);
	CStringToChar(cval, &valstr);
	sscanf_s(cval, " %d", &stainTime);

//	AltConfig.SetSel(0, -1);
//	AltConfig.ReplaceSel(valstr, false);

//	RunFastFactor.GetWindowTextW(valstr);
//	CStringToChar(cval, &valstr);
//	sscanf_s(cval, " %d", &runFastFactor);


	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedAzide() {

	FluidicsRoutine(FLUIDICS_AZIDE, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedClorox() {

	FluidicsRoutine(FLUIDICS_CLOROX, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedSample2Cone() {

	FluidicsRoutine(FLUIDICS_SAMPLE2CONE, 0);
}
//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedRinseStain() {

	FluidicsRoutine(FLUIDICS_RINSE_STAIN, 0);
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedDebubbleWithSample() {			// continuous sample

	debubbleWithSample = (DebubbleWithSampleCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedDebubbleWithSample2() {			// 

	debubbleWithSample2 = (DebubbleWithSample2CheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedBackflushWithSample() {			// continuous sample

	backflushWithSample = (BackflushWithSampleCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedPrimeSampleTube() {			// continuous sample

	primeSampleTube = (PrimeSampleTubeCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedReplaceFromSec() {			// continuous sample

	replaceFromSec = (ReplaceFromSecCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedStainSample() {			// Stain the current sample

	stainSample = (StainSampleCheckBox.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedStainAuto() {			// Stain alternate samples
	
		StainAuto = (StainAutoCheckBox.GetCheck() == BST_CHECKED);
		stainSample = (StainSampleCheckBox.GetCheck() == BST_CHECKED); // how to click this box automatically for alternate samples?
		WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedStainRinseMixingChamber() {			// option to rinse mixing chamber between samples
	
		StainRinseMixingChamber = (StainRinseMixingChamberCheckBox.GetCheck() == BST_CHECKED);
		WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedLockFilterSlider() {			// option to lock filter slider position
	
		LockFilterSlider = (LockFilterSliderCheckBox.GetCheck() == BST_CHECKED);
		WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedBeads() {

	manualBeads = (BeadsButton.GetCheck() == BST_CHECKED);
	WriteCfgFile();
}

//-----------------------------------------------------------------------------
void CFluidsTab::OnBnClickedRunSampleFast() {

	CString valstr;
	char cval[20];

	runSampleFast = (RunSampleFastCheckBox.GetCheck() == BST_CHECKED);
	runSampleFastInitial = runSampleFast;

	WriteCfgFile();

	//if (runSampleFast = (ButRunSampleFast.GetCheck() == BST_CHECKED)) {
	if (runSampleFast = (RunSampleFastCheckBox.GetCheck() == BST_CHECKED)) {
		RunFastFactor.GetWindowTextW(valstr);
		CStringToChar(cval, &valstr);
		sscanf_s(cval, " %lf", &runFastFactor);

		valstr.Format(_T("%.1f"), runFastFactor);
		RunFastFactor.SetSel(0, -1);
		RunFastFactor.ReplaceSel(valstr, false);

		RunFastFactor.EnableWindow(FALSE);
		//runSampleFast=true; //was remmed out "want it to be set by cfg file now..."
		WriteCfgFile();
	} else
		RunFastFactor.EnableWindow(TRUE);
}


void CFluidsTab::OnBnClickedEmptyMixingChamber()
{
	FluidicsRoutine(FLUIDICS_EMPTY_MIXING_CHAMBER, 0);
}


void CFluidsTab::OnBnClickedStainSample5() //acoustic focusing for ALT samples
//void CFluidsTab::OnBnClickedAcousticFocusing() //acoustic focusing for ALT samples
{
		AcousticFocusing = (AcousticFocusingCheckBox.GetCheck() == BST_CHECKED);
		WriteCfgFile();
}

