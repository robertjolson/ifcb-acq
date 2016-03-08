//-----------------------------------------------------------------------------
//  IFCB Project
//	Fluidics.h
//
//	This file contains items common to all fluidics systems interfaces
//	system-specific details go in individual .h files 
//-----------------------------------------------------------------------------
#pragma once

#include "config.h"

enum {FLUIDICS_ASYNC, FLUIDICS_SYNC};
enum {CW, CCW};

enum {	FLUIDICS_INIT,		// the choices you have for FluidicsRoutine
		FLUIDICS_DEBUBBLE,
		FLUIDICS_DEBUBBLE_AND_REFILL,
		FLUIDICS_DEBUBBLE_AND_REFILL2,
		FLUIDICS_DRAIN_FILTERS,
		FLUIDICS_RUN_SAMPLE,
		FLUIDICS_FLUSH_SAMPLETUBE,
		FLUIDICS_PRIME_SAMPLETUBE,
		FLUIDICS_REPLACEFROMSEC,
		FLUIDICS_ADJUST_FOCUS,
		FLUIDICS_ADJUST_LASER,
		FLUIDICS_ADD_STAIN,
		FLUIDICS_SAMPLE2CONE,
		FLUIDICS_MIX_STAIN,
		FLUIDICS_RINSE_STAIN,
		FLUIDICS_EMPTY_MIXING_CHAMBER,
		FLUIDICS_STOP,
		FLUIDICS_AZIDE,
		FLUIDICS_CLOROX,
		FLUIDICS_BEADS,
		FLUIDICS_SET_VALVE_PORT,
		FLUIDICS_SET_SYRINGE_OFFSET,
		FLUIDICS_SET_FILTER_SLIDER_OUT,
		FLUIDICS_SET_FILTER_OFFSET,
		FLUIDICS_SET_FILTER_SLIDER_IN,
};

enum {	QUERY_FLUIDICS_ERROR,
		IS_SYRINGE_IDLE,		// the choices you have for fluidicsQuery
		IS_SYRINGE_OPTO_OPEN,
		IS_FILTER_SLIDER_OPTO_OPEN,
		IS_FILTER_OPTO_OPEN,
		SYRINGE_LOCATION,
		FILTER_LOCATION,
		IS_VALVE_IDLE,
		ENCODER_POSITION
};
// some answers for these questions
enum { SYRINGE_IDLE_ERR, SYRINGE_IDLE, SYRINGE_BUSY, VALVE_IDLE, VALVE_BUSY, VALVE_IDLE_ERR, SYRINGE_OPTO_ERR, SYRINGE_OPTO_OPEN, SYRINGE_OPTO_CLOSED, FILTER_OPTO_ERR, FILTER_OPTO_OPEN, FILTER_OPTO_CLOSED };

bool	FluidicsSerialInit(void);
void	FluidicsClose(void);
bool	FluidicsRoutine(int task, int userparam);
void	FluidicsVerbosity(bool verbosity);
void	FluidicsPullNewSample(void);
void	RunSample(void);
void	RunSampleFast(void);
//void	AcousticFocusing(void);
void	Sample2Cone(void);
void	MixStain(void);
void	RinseStain(void);
void	EmptyMixingChamber(void);
void	FlushSampleTube(void);
void	PrimeSampleTube(void);
void	ReplaceFromSec(void);
void	DrainFilters(void);
//void	MoveValveString(char *s, int strlen, int currentValvePosition, int desiredValvePosition, int valveDirection);
//void	MoveValve(int currentValvePosition, int desiredValvePosition, int valveDirection);
void	MoveValve(char *s, int strlen, int currentValvePosition, int desiredValvePosition, int valveDirection);
int		FluidicsQuery(int fluidicsQuery);
void	Debubble(void);
void	DebubbleAndRefill(void);
void	DebubbleCartridge(void);
void	Biocide(void);
void	Clorox(void);
void	Beads(void);
void	StainSample5(void);

#ifdef KLOEHN
#include "FluidsKloehn.h"
#endif

#ifdef WHOI_CUSTOM
#include "FluidsWHOI.h"
#endif

