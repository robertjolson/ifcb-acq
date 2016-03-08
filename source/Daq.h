//-----------------------------------------------------------------------------
//  IFCB Project
//	Daq.h
//	Martin Cooper
//
//	This file contains items common to all io interfaces
//	IO card-specific details go in individual .h files 
//-----------------------------------------------------------------------------
#pragma once

#include "config.h"

#ifdef NIUSB6009
#include "Daq_niusb6009.h"
#endif

#define OFF		(uint16)0
#define ON		(uint16)1
#define AUTO	(uint16)2

#ifdef DAQ_MCC
#include "DaqMcc.h"
#endif

extern double	Humidity, Temperature;
extern double	Dac[];

void		DAQInit(void);
bool		DAQShutdown(void);
void		StartAcquisition(void);
void		StopAcquisition(void);
void		DaqArmTrigger(void);
void		GenerateTrigger(void);
void		ReadDAQIntegrated(DataStruct *data);
double		ReadAdc(uint8 channel);
void		ReadHousekeeping(void);
bool		CheckForCapture(void);
