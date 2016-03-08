//-----------------------------------------------------------------------------
//  IFCB Project
//	FluidsWHOI.h
//
//	Details specific to the WHOI custom fluidics system
//-----------------------------------------------------------------------------
#pragma once

#include "config.h"

#ifdef WHOI_CUSTOM

// serial port settings
#define FLUIDICS_BAUD		CBR_9600
#define FLUIDICS_PARITY		NOPARITY
#define FLUIDICS_STOP_BITS	ONESTOPBIT
#define FLUIDICS_BITS			8
#define FLUIDICS_HANDSHAKE	'None'

#define FLUIDICS_IDLE_TIMEOUT	255//60	//20			// s

#define SYRINGE_5ML			48000L
#define SYRINGE_1ML			9600L


// SPECIAL FUNCTIONS FOR WHOI CUSTOM FLUIDICS SYSTEMS
//void InitWHOICustom(void);
//static bool SendWHOICustom(char *str, int syncMode);
//void DebubbleWHOICustom(void);
//void RunSampleWHOICustom(void);
//void FlushFlowcellWHOICustom(void);
//void MoveValveString(char *s, int strlen, int currentValvePort, int desiredValvePosition, int valveDirection);
void MoveValve(char *s, int strlen, int currentValvePosition, int desiredValvePosition);
void AdjustFocus(int focusStep);
//bool QueryWHOICustomOptoOpen(void);
//int QueryWHOICustomSyringeLocation(void);
//int QueryWHOICustom(int fluidicsQuery);

#endif