//-----------------------------------------------------------------------------
//  IFCB Project
//	daq_niusb6009.h
//
//	NIUSB6009 card-specific details
//-----------------------------------------------------------------------------
#pragma once


#ifdef NIUSB6009

#include "NIDAQmx.h"

#define DAC_OUTPUT_MAX 5.0

// DAC channels
#define	PMTA_DAC	1
#define PMTB_DAC	2

// don't really exist on benchtop
#define PMTC_DAC	0
#define FLASH_DAC	0



// ADC channels
#define	PMTA_ADC	2
#define PMTB_ADC	3
#define PMTC_ADC	4
#define TRIG_ADC	7
#define TEMP_ADC	1
#define HUMID_ADC	0
#define FLASH_ADC	5
#define SPARE_ADC	6


void SetupNI6009TriggerTask(void);
void SetupNI6009AnalogOutputTask(int DACchannel);
void SetupNI6009ArmTask(void);
void SetupNI6009AnalogInputTask(void);

#endif