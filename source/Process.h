//-----------------------------------------------------------------------------
//  IFCB Project
//	Process.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

extern bool rqPZT;

// modes for AcqComplete()
#define END_MANUAL	1
#define END_AUTO	2
#define END_RETRY	3

typedef enum	{PZT_INIT, PZT_RUN, PZT_OFF, PZT_SCAN} PZTMODE;

bool AcqInit(void);
void AcqComplete(int mode);
void PZTcommand(PZTMODE mode);
void ProcessTrigger(unsigned int numRois);
void WriteArraysToFile(void);
void DoCleanCalib(void);


