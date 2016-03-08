//-----------------------------------------------------------------------------
//  IFCB Project
//	FluidsTab.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#include "resource.h"
#include "afxwin.h"
#include "Tab.h"

class CFluidsTab : public CTab
{
	DECLARE_DYNAMIC(CFluidsTab)

public:
	CFluidsTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFluidsTab();

	enum { IDD = IDD_FLUIDS };
	void	OnShowTab();
	bool	Run(bool run);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	CButton FluidActive;
	CButton VerboseCheckBox;
	CButton ButFluidsInit;
	CButton ButFluidsStop;
	CButton ButSyringe0;
	CButton ButDebubble;
	CButton ButDebubbleAndRefill;
	CButton ButDebubbleAndRefill2;
	CButton ButRunSample;
	CButton RunSampleFastCheckBox;
	CButton ButDrainFilters;
	CButton ButAddStain;
	CButton ButSample2Cone;
	CButton ButRinseStain;
	CButton ButEmptyMixingChamber;
	CButton ButFlush;
	//CButton	DiscreteSample;
	//CButton	BackflushWithSampleCheckBox;
	CButton ContinuousSample;
	CButton ButAzide;
	CButton ButClorox;
	CButton BeadsButton;
	CButton ButAdjustFocus;
	CButton DebubbleWithSampleCheckBox;
	CButton DebubbleWithSample2CheckBox;
	CButton BackflushWithSampleCheckBox;
	CButton PrimeSampleTubeCheckBox;
	CButton ReplaceFromSecCheckBox;
	CButton StainSampleCheckBox;
	CButton StainAutoCheckBox;
	CButton StainRinseMixingChamberCheckBox;
	CButton LockFilterSliderCheckBox;
	CButton AcousticFocusingCheckBox;

public:
	afx_msg void OnBnClickedFluidsInit();
	afx_msg void OnBnClickedSyringe0();
	afx_msg void OnBnClickedSyringe1();
	afx_msg void OnBnClickedFluidsStop();
	afx_msg void OnBnClickedDebubble();
	afx_msg void OnBnClickedDebubbleAndRefill();
	afx_msg void OnBnClickedRunSample();
	afx_msg void OnBnClickedRunSampleFast();
	afx_msg void OnBnClickedDrainFilters();
	afx_msg void OnBnClickedAddStain();
	afx_msg void OnBnClickedSample2Cone();
	afx_msg void OnBnClickedRinseStain();
	afx_msg void OnBnClickedEmptyMixingChamber();
	afx_msg void OnBnClickedFlush();
	afx_msg void OnBnClickedVerbose();
	afx_msg void OnBnClickedFluidActive();
	//afx_msg void OnBnClickedDiscrete();
	afx_msg void OnBnClickedBackflushWithSample();
	afx_msg void OnBnClickedPrimeSampleTube();
	afx_msg void OnBnClickedReplaceFromSec();
	afx_msg void OnBnClickedStainSample();
	afx_msg void OnBnClickedStainAuto();
	afx_msg void OnBnClickedStainRinseMixingChamber();
	afx_msg void OnBnClickedLockFilterSlider();
	afx_msg void OnBnClickedSampVolSet();
	afx_msg void OnBnClickedContinuous();
	afx_msg void OnBnClickedAzide();
	afx_msg void OnBnClickedClorox();
	afx_msg void OnBnClickedDebubbleWithSample();
	afx_msg void OnBnClickedDebubbleWithSample2();
	CEdit SampVolume;
	CEdit SampleVolume2;
	CEdit SampleVolume2skip;
	CEdit SampleVolume2skipALT;
	CButton ButAltPort;
	CButton ButNormPort;
	CEdit BeadsInterval;
	CEdit StainTime;
	CEdit BeadsVolume;
	CStatic AltLabel;
	afx_msg void OnBnClickedBeads();
	afx_msg void OnBnClickedBeads2();
	afx_msg void OnBnClickedAltPort();
	afx_msg void OnBnClickedNormPort();
protected:
	CEdit RunFastFactor;
public:
	//afx_msg void OnBnClickedPrimeSampleTube2();
	//afx_msg void OnBnClickedEmptyMixingChamber();
	afx_msg void OnBnClickedStainSample5();
	afx_msg void OnBnClickedDebubbleAndRefill2();
};
