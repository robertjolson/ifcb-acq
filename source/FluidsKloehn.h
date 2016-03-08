//-----------------------------------------------------------------------------
//  IFCB Project
//	FluidsKloehn.h
//
//	Details specific to the Kloehn fluidics system
//-----------------------------------------------------------------------------
#pragma once

#include "config.h"

#ifdef KLOEHN

// serial port settings
#define FLUIDICS_BAUD		CBR_9600
#define FLUIDICS_PARITY		NOPARITY
#define FLUIDICS_STOP_BITS	ONESTOPBIT
#define FLUIDICS_BITS			8
#define FLUIDICS_HANDSHAKE	'None'

#define FLUIDICS_IDLE_TIMEOUT		20		// s
#define FLUIDICS_SERIAL_TIMEOUT		10		// ms * 15

#define SYRINGE_5ML			48000L
#define SYRINGE_1ML			9600L

// SPECIAL FUNCTIONS FOR KLOEHN FLUIDICS SYSTEMS
/*void InitKloehn(void);
static bool SendKloehn(char *str, int syncMode);
void DebubbleKloehn(void);
void FlushFlowcellKloehn(void);
void RunSampleKloehn(void);
bool QueryKloehnIdle(void);*/

#endif