



//-----------------------------------------------------------------------------
//  IFCB Project
//	Fluidics.cpp
//
//	Handles serial commands to WHOI custom syringe pump & valve (submersible build)
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"

#ifdef WHOI_CUSTOM

#include "IfcbDlg.h"
#include "Fluids.h"
#include "FluidsWHOI.h"
#include "FileIo.h"
#include "Daq.h"

#include <time.h>
#include <math.h>

//for calling Labview executables for acoustic focusing ON/OFF
#include <shlobj.h>
#include <Shlwapi.h>
#include <objbase.h>



char	fluidicsPort[STRING_PARAM_LENGTH];
DCB dcbSerialParams = {0};

int syringeOffset;			//THIS IS IN Fluidics.h, 
int filterOffset;			//THIS IS IN Fluidics.h, 
int syringeLocation = 0;	// the location of the syringe (bottomed-out = 0)
int filterLocation = 0;	// the location of the syringe (bottomed-out = 0)
int encoderPosition = -1;
int encoderOffset = 0;
double beadSampleTotalVolume;
double fullSyringeVolume;// = 5.0; // now use syringeSize to calculate fullSyringeVolume.
//int syrOffset = 0; 

//int focus_direction;   // for focus motor
//int userparam;
//int step;

static HANDLE hSerialFluidics;
//static int r = 10;
//static CString str;
static int step2port = 256; // number of encoder steps between valve ports
//char valveStr[20];
static int step2fullSyr = 328200; // number of syringe motor encoder steps for whole syringe (empirical)
static int base_syr_sampling_speed = 8962;
/* distribution valve ports: 
	1 = needle
	2 = cone
	3 = biocide
	4 = Clorox
	5 = (plugged)
	6 = beads
	7 = outside
	8 = sample tube
*/
static void	AdjustFocus(int focusStep);
static void	AdjustLaser(int focusStep);
static void	AddStain();

//-----------------------------------------------------------------------------
// A general error handler
//-----------------------------------------------------------------------------
static void DisplayError(void) {

	CString str;

	str.Format(_T("%s: %d\r\n"), _T("Fluidics serial port error\r\n"), GetLastError());
	ERROR_MESSAGE_EXT(str);
}

//-----------------------------------------------------------------------------
// function to send commands to WHOI custom system & interpret error codes
//-----------------------------------------------------------------------------
static bool SendWHOICustom(char *str, int syncMode) {

	DWORD nBytes = 0;
	char readstr[20];
	time_t now, then;
//float loop;

	fluidSyncBusy = syncMode == FLUIDICS_SYNC;			// set busy flag if required

	//DEBUG_MESSAGE_EXT(_T("In SendWHOICustom\r\n"));
	// to print every command you send in the debug window
	if (fluidicsVerbose) {
		CString debugStr = _T("In SendWHOICustom, sending \r\n");
		debugStr += str;
		debugStr += _T("\r\n");
		DEBUG_MESSAGE_EXT(debugStr); 
	}
	CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, str, strlen(str), &nBytes, NULL), TRUE, _T("SendWHOICustom : Serial write error\r\n"));
	
	// wait a bit for the function to execute before asking for a status
	Sleep(15);

	readstr[0] = '\0';
	CALL_CHECKED_RETFALSE_EXT(ReadFile(hSerialFluidics, readstr, 7, &nBytes, NULL), TRUE, _T("SendWHOICustom : Serial read error\r\n"));

	// readstr[3] has the error/status byte
	if (fluidicsVerbose) {
		if ( (readstr[1] == '/') && (readstr[2] == '0') ) {	// parse the third character
			switch(readstr[3]) {
			case '@':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Status busy\r\n"));
				break;
			case 96:     // the ' char
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Status OK\r\n"));
				break;
			case 'a':
			case 'A':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Syringe not initialized\r\n"));
				break;
			case 'b':
			case 'B':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Invalid command\r\n"));
				break;
			case 'c':
			case 'C':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Invalid operand\r\n"));
				break;
			case 'd':
			case 'D':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Communication error\r\n"));
				break;
			case 'g':
			case 'G':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Device not initialized\r\n"));
				break;
			case 'i':
			case 'I':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Syringe overload\r\n"));
				break;
			case 'j':
			case 'J':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Valve overload\r\n"));
				break;
			case 'k':
			case 'K':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: No syringe move in valve bypass\r\n"));
				break;
			case 'o':
			case 'O':     
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Command buffer full\r\n"));
				break;
			default:
				DEBUG_MESSAGE_EXT(_T("SendWHOICustom: Other error\r\n"));

				CString str;
				readstr[nBytes] = '\0';			// terminate the string
				str = _T("returned bytes: ");
				str += readstr;
				DEBUG_MESSAGE_EXT(str);
				break;
			}
		} else {
			DEBUG_MESSAGE_EXT(_T("Send Kloehn: Bad readstr\r\n"));
			CString str;
			readstr[nBytes] = '\0';			// terminate the string
			str = _T("returned bytes: ");
			str += readstr;
			str += _T("\r\n");
			DEBUG_MESSAGE_EXT(str);
		}
	}

	if (syncMode == FLUIDICS_ASYNC) 					// pass through and return
		return true;
		
	// wait here for return OK from kloehn
	time(&then);
	//while (FluidicsQuery(IS_SYRINGE_IDLE) == SYRINGE_BUSY) {					// do this until idle
	while ((FluidicsQuery(IS_SYRINGE_IDLE) == SYRINGE_BUSY) | (FluidicsQuery(IS_VALVE_IDLE) == VALVE_BUSY)) {					// do this until idle
		
		// should be able to query positions here....
		encoderPosition = FluidicsQuery(ENCODER_POSITION);

		Sleep(1000);							// wait a sec

		time(&now);
		if (now - then > FLUIDICS_IDLE_TIMEOUT) {
			ERROR_MESSAGE_EXT(_T("WHOICustom Idle: Serial timeout\r\n"));
			return false;
		}
	}
	fluidSyncBusy = false;

	return true;
}

//-----------------------------------------------------------------------------
static void InitPumps(void) { // this inits syringe pump (motor1) and valve (motor2) to home (opto) positions

	char buffer[20];
	DWORD nBytes;
	int encoderPosition;
	extern int currentValvePosition;
//	int steps;
//	int j;
//	char *s;
	fullSyringeVolume = syringeSize; //define syringe size here since this always happens first.
	DEBUG_MESSAGE_EXT(_T("syringe size defined (InitPumps)... \r\n"));

	// define syr speed in terms of syringe size


	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "\r\n", 2, &nBytes, NULL), TRUE, _T("Serial error\r\n"));
	//Sleep(15);	// wait a few ms
	Sleep(100);	// wait a few ms

	// send a Terminate command, just to be sure
	// to stop the pump if running
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "/1T\r", 5, &nBytes, NULL), TRUE, _T("Can't write WHOICUSTOM\r\n"));
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "/2T\r", 5, &nBytes, NULL), TRUE, _T("Can't write WHOICUSTOM\r\n"));

	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);		//send an ASYNC near start of InitPumps to allow X'ing out if desired.

	// for the valve itself
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);		//external encoder, speed, power, hold power, zero it.
	//send it again
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);		//external encoder, speed, power, hold power, zero it.
	//SendWHOICustom("/2m5R\r", FLUIDICS_SYNC);		//current limit to 5% (AllMotion can do 2A@24V=48W and motor spec=2.5W; 2.5/48=0.052).
	SendWHOICustom("/2m25R\r", FLUIDICS_SYNC);		//current limit to 5% (AllMotion can do 2A@24V=48W and motor spec=2.5W; 2.5/48=0.052).
	SendWHOICustom("/2V3000m75h0Z2200R\r", FLUIDICS_SYNC);	//external encoder, speed, power, hold power, zero it.
	//SendWHOICustom("/2V1000m75h0Z2200R\r", FLUIDICS_SYNC);	//external encoder, speed, power, hold power, zero it.
	Sleep(10);
/*
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);			// valve motor has external encoder
	SendWHOICustom("/2V3000R\r", FLUIDICS_SYNC);		// valve motor velocity -- slow for encoder homing
	SendWHOICustom("/2m75R\r", FLUIDICS_SYNC);			// valve motor power = 75%
	SendWHOICustom("/2h0R\r", FLUIDICS_SYNC);			// valve motor hold power = 0%
	//SendWHOICustom("/2v1000R\r", FLUIDICS_SYNC);			// v = "homing velocity" ??? NO effect
	//encoderPosition = FluidicsQuery(ENCODER_POSITION);	
	//SendWHOICustom("/2z100R\r", FLUIDICS_SYNC);			// set encoder to 0.... just to test
	encoderPosition = FluidicsQuery(ENCODER_POSITION);	
	SendWHOICustom("/2Z4200R\r", FLUIDICS_SYNC);		// valve rotate position to zero (port 7)
	//SendWHOICustom("/2z100R\r", FLUIDICS_SYNC);			// manually set to 'near zero' to avoid wraparound...
*/
	encoderPosition = FluidicsQuery(ENCODER_POSITION);
	
/*	// move a bit to try fix problem of missing port in move from 7 to 8; (256 steps between ports).  
	steps = 0; // try moving INIT valve position CW ~8% of distance between ports...
	j = sprintf_s(s, 20, "/2P%d", steps);
	//j = sprintf_s(s, strlen, "/2z%dD%d", steps, steps);	// reset encoder upwards to allow it to decrement;  D for going ccw (e.g., 7 ro 6)
	SendWHOICustom(s, FLUIDICS_SYNC); // do the valve move
*/

	////SendWHOICustom("/2z0R\r", FLUIDICS_SYNC);			// set valve encoder position to zero (at port 7)
	//DEBUG_MESSAGE_EXT(_T("Valve Initialized to port 7\r\n"));

	//SendWHOICustom("/1m10R\r", FLUIDICS_SYNC);	// set syr max current allowed to 10%(default = 50%=1A)
	SendWHOICustom("/1V150000R\r", FLUIDICS_SYNC);	// set syr speed
	//SendWHOICustom("/1m5R\r", FLUIDICS_SYNC);		//current limit to 5% (AllMotion can do 2A@24V=48W and motor spec=2.5W; 2.5/48=0.052).
	SendWHOICustom("/1m25R\r", FLUIDICS_SYNC);		//current limit to 5% (AllMotion can do 2A@24V=48W and motor spec=2.5W; 2.5/48=0.052).
	if (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_OPEN)	// check opto status -- if it is open, need to move down; otherwise, up 
	{
		//DEBUG_MESSAGE_EXT(_T("opto is open\r\n"));
		SendWHOICustom("/1Z4000000R\r", FLUIDICS_SYNC);	// if open, just home to opto
	}
	else if (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_CLOSED)	// check opto status -- if it is open, need to move down; otherwise, up 
	{
		SendWHOICustom("/1P20000R\r", FLUIDICS_ASYNC);	// if closed, move up (positive) slowly while checking opto
		//DEBUG_MESSAGE_EXT(_T("syringe opto was closed\r\n"));
		while (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_CLOSED); 
		//// when not closed anymore, send it a stop command
		SendWHOICustom("/1T\r", FLUIDICS_SYNC);	// when it gets open, stop
		SendWHOICustom("/1Z4000000R\r", FLUIDICS_SYNC);	// home just a little to opto
		//DEBUG_MESSAGE_EXT(_T("STOPPED -- opto open TRUE\r\n"));				
	}
	//DEBUG_MESSAGE_EXT(_T("syringe pump homed to opto\r\n"));

	//// Now, set location of opto to be syringeOffset, so that bottom = 0.
	sprintf_s(buffer, 20, "/1z%dR\r", syringeOffset); // the %d is where the value of syringeOffset gets written (in ASCII)
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	////encoderPosition = FluidicsQuery(ENCODER_POSITION);	
	//DEBUG_MESSAGE_EXT(_T("syringe and valve are at INIT positions\r\n"));

	currentValvePosition = 7; //valve home = outside (port 7)

	return;

}

//-----------------------------------------------------------------------------
static void FilterSliderOUT(void) { // this inits filter slider (motor5) to home (opto) position

	DWORD nBytes;

	// send a Terminate command, just to be sure
	// to stop the pump if running
	
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "/5T\r", 5, &nBytes, NULL), TRUE, _T("Can't write WHOICUSTOM\r\n"));
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "\r\n", 2, &nBytes, NULL), TRUE, _T("Serial error\r\n"));
	Sleep(100);	// wait a few ms
	//SendWHOICustom("/5m5R\r", FLUIDICS_SYNC);	// set max current to <2.5W (ezsv10 max=2A*24V = 48W... so 5%...seems low?) 
	SendWHOICustom("/5m25R\r", FLUIDICS_SYNC);	// set max current to <2.5W (ezsv10 max=2A*24V = 48W... so 5%...seems low?) 
	Sleep(100);	// wait a few ms
	SendWHOICustom("/5V10000R\r", FLUIDICS_SYNC);	// set speed for homing...
	Sleep(100);	// wait a few ms
	SendWHOICustom("/5T\r", FLUIDICS_SYNC);	//
	Sleep(100);	// wait a few ms
	SendWHOICustom("/5Z20000R\r", FLUIDICS_SYNC); // home to opto (= OUT position)
	Sleep(20000);
	return;
}

static void FilterSliderIN(void) { // this inits filter slider (motor5) to home (opto) position

	DWORD nBytes;

	// send a Terminate command, just to be sure
	// to stop the pump if running
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "/5T\r", 5, &nBytes, NULL), TRUE, _T("Can't write WHOICUSTOM\r\n"));
	CALL_CHECKED_EXT(WriteFile(hSerialFluidics, "\r\n", 2, &nBytes, NULL), TRUE, _T("Serial error\r\n"));
	Sleep(100);	// wait a few ms
	//SendWHOICustom("/5m5R\r", FLUIDICS_SYNC);	// set max current to <2.5W (ezsv10 max=2A*24V = 48W... so 5%...seems low?) 
	SendWHOICustom("/5m25R\r", FLUIDICS_SYNC);	// set max current to <2.5W (ezsv10 max=2A*24V = 48W... so 5%...seems low?) 
	Sleep(100);	// wait a few ms
	SendWHOICustom("/5V10000R\r", FLUIDICS_SYNC);	// set speed for homing...
	Sleep(100);	// wait a few ms
	SendWHOICustom("/5T\r", FLUIDICS_SYNC);	//
	Sleep(100);	// wait a few ms
	//SendWHOICustom("/5A4500R\r", FLUIDICS_SYNC); // move to IN position
	SendWHOICustom("/5A3500R\r", FLUIDICS_SYNC); // move to IN position
	Sleep(20000);

	return;
}

//-----------------------------------------------------------------------------
// Top-level function to pull in and begin dispensing new sample
// includes any debubbling, biocide prep, etc if needed
// include ONLY the fluidics aspects here
//-----------------------------------------------------------------------------
void FluidicsPullNewSample(void) {

//Filter IN position = 555 dmsp – for PE (orange fluor) measurements, using 530 nm band for camera
	if (replaceFromSec)
		FluidicsRoutine(FLUIDICS_REPLACEFROMSEC, 0);
	if (debubbleWithSample)
		FluidicsRoutine(FLUIDICS_DEBUBBLE_AND_REFILL, 0);
	if (backflushWithSample)
		FluidicsRoutine(FLUIDICS_FLUSH_SAMPLETUBE, 0);
	if (primeSampleTube)
		FluidicsRoutine(FLUIDICS_PRIME_SAMPLETUBE, 0);

	//if (StainAuto && runType == ALT) {//staining alt samples, and on ALT sample, so filter to OUT.
	if (StainAuto && runType == ALT && LockFilterSlider == false) {//staining alt samples, and on ALT sample, so filter to OUT.
		FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_OUT, 0); //Filter in OUT position = 595 dmsp – for FDA (green fluor) measurements, using 575 nm band for camera
		//DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n")); // stop recirculating sheath when RinseStain starts, not before.
		ChangeHWParam(STIRRER, OFF); //stop mixing stirrer
		ChangeHWParam(FLASH_DAC, flashVoltage2);
		DEBUG_MESSAGE_EXT(_T("Set flash lamp V for Alt (Stain) dichroic \r\n"));
		}
	//else if (StainAuto && runType == NORMAL)	{ //staining alt samples; move filter IN for NORMAL ones
	else if (StainAuto && runType == NORMAL && LockFilterSlider == false)	{ //staining alt samples; move filter IN for NORMAL ones
		FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_IN, 0); //no-stain NORMAL so filter in IN position = 555 dmsp – for PE (orange fluor) measurements, using 530 nm band for camera
		if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
			ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
			ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
		}
		else {
			ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
			ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
		}
		ChangeHWParam(FLASH_DAC, flashVoltage);
		DEBUG_MESSAGE_EXT(_T("Set flash lamp V for Normal dichroic \r\n"));
		}
	else if (stainSample && runType == NORMAL && LockFilterSlider == false) { //manually stained sample, and not on ALT sample, so filter to OUT.
		ChangeHWParam(FLASH_DAC, flashVoltage2);
		DEBUG_MESSAGE_EXT(_T("Set flash lamp V for Alt (Stain) dichroic \r\n"));
		FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_OUT, 0);
		//ChangeHWParam(FILL2bypassFilter, ON); //non-recirculating sheath flow
		//ChangeHWParam(FILL3newSheath, ON); //non-recirculating sheath flow
		if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
			ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
			ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
		}
		else {
			ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
			ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
		}
		ChangeHWParam(FLASH_DAC, flashVoltage2);
		DEBUG_MESSAGE_EXT(_T("Set flash lamp V for Alt (Stain) dichroic \r\n"));
		}
	else if (stainSample == false && StainAuto == false && syringesDone == 0 && LockFilterSlider == false)	{ // if only unstained NORMAL samples are being run, only move filter on first sample...
		FluidicsRoutine(FLUIDICS_SET_FILTER_SLIDER_IN, 0); //no-stain NORMAL so filter in IN position = 555 dmsp – for PE (orange fluor) measurements, using 530 nm band for camera
		if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
			ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
			ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
		}
		else {
			ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
			ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
		}
		//ChangeHWParam(FILL2bypassFilter, OFF); //normal (recirculating) sheath flow
		//ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow

		ChangeHWParam(FLASH_DAC, flashVoltage);
		DEBUG_MESSAGE_EXT(_T("Set flash lamp V for Normal dichroic \r\n"));
		}	
	//else { 
	//	DEBUG_MESSAGE_EXT(_T("If you see this msg, it is an unknown case...  \r\n"));
	//	}
	FluidicsRoutine(FLUIDICS_RUN_SAMPLE, 0);
}

//-----------------------------------------------------------------------------
bool FluidicsRoutine(int task, int userparam) {

//	char buffer[20];
	
	int strlen = 0;
	//extern int current_filter_slider_position = -1; //0=in, 1=out.
	//char *s;

//	char *valueStr = "";
	// need this active for initialization at startup
	if (!fluidsActive) {
//		DEBUG_MESSAGE_EXT(_T("Error in FluidicsRoutine: fluidsActive is FALSE\r\n"));
		return true;
	}

	switch (task) {

		case FLUIDICS_INIT:	// this inits the syringe + valve
			DEBUG_MESSAGE_EXT(_T("Called FLUIDICS_INIT in FluidicsRoutine\r\n"));
			InitPumps();
			break;

		case FLUIDICS_SET_SYRINGE_OFFSET:	
			// this is where you compute the offset between opto position and dead bottom of syringe

			// with power OFF, manually set syringe location to DEAD BOTTOM; then power pump and click "Set Syringe 0" button
			SendWHOICustom("/1V10000R\r", FLUIDICS_SYNC);		// motor velocity = slow
			//SendWHOICustom("/1m100R\r", FLUIDICS_SYNC);		// motor power = 100%

			DEBUG_MESSAGE_EXT(_T("Called FLUIDICS_SET_OFFSET\r\n"));

			SendWHOICustom("/1z0R\r\n", FLUIDICS_SYNC);		// zero syringe at bottom
			//SendWHOICustom("/1P30000R\r\n", FLUIDICS_ASYNC);	// have it move slowly up to some large # (more than expected offset)
			// use some code from Init syringe section to find opto....
			if (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_OPEN)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("opto is open; set syringe to bottom and retry\r\n"));
				break; // 
			}
			else if (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_CLOSED)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				SendWHOICustom("/1P20000R\r", FLUIDICS_ASYNC);	// if closed, move up (positive) slowly while checking opto
				DEBUG_MESSAGE_EXT(_T("syringe opto was closed\r\n"));
				while (FluidicsQuery(IS_SYRINGE_OPTO_OPEN) == SYRINGE_OPTO_CLOSED); 
				SendWHOICustom("/1T\r", FLUIDICS_SYNC);	// when it gets open, stop
				DEBUG_MESSAGE_EXT(_T("STOPPED -- opto open TRUE\r\n"));				
			}

			// when it hits opto, read the current location into syringeLocation
			syringeOffset = FluidicsQuery(SYRINGE_LOCATION);					
			WriteCfgFile();				// write it out
						
			DEBUG_MESSAGE_EXT(_T("syringe pump bottom-out value recorded, and syringe moved to opto\r\n"));
			break;

		case FLUIDICS_SET_FILTER_SLIDER_OUT:	
			//check to see if it is OUT (i.e., opto 5 closed).
			if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_OPEN)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("FILTER opto is open; moving slider OUT. \r\n"));
				FilterSliderOUT(); // move slider out to opto
			}
			else if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_CLOSED)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("filter slider is already in OUT position. \r\n"));
			}
			break;

		case FLUIDICS_SET_FILTER_SLIDER_IN:	
			//check to see if it is OUT (i.e., opto 5 closed).
			if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_OPEN)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("FILTER opto is open; moving slider out to opto and then to IN. \r\n"));
				//SendWHOICustom("/5P20000R\r", FLUIDICS_ASYNC); //start it moving out and watch opto.
				//while (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_OPEN); 
				//SendWHOICustom("/5T\r", FLUIDICS_SYNC);	// when it gets closed, stop

				FilterSliderOUT(); // move slider out to opto
				FilterSliderIN(); // move slider in
			}
			else if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_CLOSED)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("filter slider is in OUT position; moving slider IN. \r\n"));
				FilterSliderIN(); // move slider in
			}
			break;

		case FLUIDICS_SET_FILTER_OFFSET:	
			// this is where you compute the offset between opto position and OUT position of filter slider
			//InitFilter(); // move slider out to opto

			// with power OFF, manually move filter slider to OUT position; then power pump and click "Set Filter Out" button
			/*SendWHOICustom("/5V10000R\r", FLUIDICS_SYNC);		// motor velocity = slow
			SendWHOICustom("/5m100R\r", FLUIDICS_SYNC);		// motor power = 100%

			DEBUG_MESSAGE_EXT(_T("Called FLUIDICS_SET_FILTER_OFFSET\r\n"));

			SendWHOICustom("/5z0R\r\n", FLUIDICS_SYNC);		// zero syringe at bottom
			//SendWHOICustom("/1P30000R\r\n", FLUIDICS_ASYNC);	// have it move slowly up to some large # (more than expected offset)
			// use some code from Init syringe section to find opto....
			if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_OPEN)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				DEBUG_MESSAGE_EXT(_T("opto is open; set syringe to bottom and retry\r\n"));
				break; // 
			}
			else if (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_CLOSED)	// check opto status -- if it is open, need to move down; otherwise, up 
			{
				SendWHOICustom("/5P20000R\r", FLUIDICS_ASYNC);	// if closed, move up (positive) slowly while checking opto
				DEBUG_MESSAGE_EXT(_T("FILTER opto was closed\r\n"));
				while (FluidicsQuery(IS_FILTER_OPTO_OPEN) == FILTER_OPTO_CLOSED); 
				SendWHOICustom("/5T\r", FLUIDICS_SYNC);	// when it gets open, stop
				DEBUG_MESSAGE_EXT(_T("STOPPED -- opto open TRUE\r\n"));				
			}

			// when it hits opto, read the current location into syringeLocation
			syringeOffset = FluidicsQuery(SYRINGE_LOCATION);					
			WriteCfgFile();				// write it out
						
			DEBUG_MESSAGE_EXT(_T("FILTER -out value recorded, and FILTER moved to opto\r\n"));
			*/break;

		case FLUIDICS_DEBUBBLE:
			Debubble();
			break;

		//case FLUIDICS_BACKFLUSH_WITH_SAMPLE:
		//	FlushSampleTube();
		//	break;

		case FLUIDICS_DEBUBBLE_AND_REFILL:
			DebubbleAndRefill();
			break;
		case FLUIDICS_DEBUBBLE_AND_REFILL2:
			DebubbleCartridge();
			break;

		case FLUIDICS_RUN_SAMPLE:					
			if (manualBeads & AcousticFocusingOnALT) //rob
				Beads();  //rob
			else  //rob
				RunSample();
			break;

		case FLUIDICS_SAMPLE2CONE:	
			Sample2Cone();
			break;

		case FLUIDICS_MIX_STAIN:	
			MixStain();
			break;

		case FLUIDICS_RINSE_STAIN:	
			RinseStain();
			break;

		case FLUIDICS_EMPTY_MIXING_CHAMBER:	
			EmptyMixingChamber();
			break;

		case FLUIDICS_FLUSH_SAMPLETUBE:
			FlushSampleTube();
			break;

		case FLUIDICS_PRIME_SAMPLETUBE:
			PrimeSampleTube();
			break;

		case FLUIDICS_REPLACEFROMSEC:
			ReplaceFromSec();
			break;

		case FLUIDICS_ADJUST_FOCUS:
			AdjustFocus(userparam);
			break;

		case FLUIDICS_ADJUST_LASER:
			AdjustLaser(userparam);
			break;

		case FLUIDICS_ADD_STAIN:
			AddStain();
			break;

		case FLUIDICS_STOP:
			// stop command for EZ servo
//			SendWHOICustom(".......
			SendWHOICustom("/1T\r\n", FLUIDICS_SYNC);
			break;

		case FLUIDICS_AZIDE:
			Biocide(); 
			break;

		case FLUIDICS_CLOROX:
			Clorox(); 
			break;

		case FLUIDICS_BEADS:
			Beads(); 
			break;

		case FLUIDICS_SET_VALVE_PORT:
			//	where you put in valve code, use passed valve argument to go to right location
			break;

		default:
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Closing the serial port to the fluidics system
//-----------------------------------------------------------------------------
void FluidicsClose(void) {

	CloseHandle(hSerialFluidics);								// close serial port
}

//-----------------------------------------------------------------------------
// Opening the serial port to the fluidics system AND
//	initializing the pump
//-----------------------------------------------------------------------------
bool FluidicsSerialInit(void) {

	CString str;
	COMMTIMEOUTS timeouts={0};
	DWORD nBytes = 0;

	DEBUG_MESSAGE_EXT(_T("Initializing fluidics serial port\r\n"));

	// open the serial port to the fluidics system
	str = fluidicsPort;
	hSerialFluidics = CreateFileW(str,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hSerialFluidics == INVALID_HANDLE_VALUE) {
		DisplayError();
		return false;
	}

	// set the serial port parameters
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerialFluidics, &dcbSerialParams)) {		// error getting state
		DisplayError();
		FluidicsClose();											// close serial port
		return false;
	}
	dcbSerialParams.BaudRate = FLUIDICS_BAUD;
	dcbSerialParams.ByteSize = FLUIDICS_BITS;
	dcbSerialParams.StopBits = FLUIDICS_STOP_BITS;
	dcbSerialParams.Parity = FLUIDICS_PARITY;

	if(!SetCommState(hSerialFluidics, &dcbSerialParams)) {		// error setting serial port state
		DisplayError();
		FluidicsClose();											// close serial port
		return false;
	}
	
	// set some timeouts
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;

	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerialFluidics, &timeouts)) {
		//MessageBox(0, str, "Error setting timeout of Kloehn serial port", MB_OK | MB_ICONSTOP);
		DisplayError();
		FluidicsClose();											// close serial port
		return false;
	}
	
	// don't put debug messages or real pump commands here because the app window is not yet visible

	return true;
}


//-----------------------------------------------------------------------------
void Debubble(void) {

	char buffer[20]; //, *valveDirection;
	int k;
	int valveDirection;
	int init_syr_sampling_speed;
	int steps;
	double init_syringeVolume;
	// test commit additional line in Debubble
	if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
		ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
		ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
	}
	else {
		ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
		ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - in Debubble -- please wait\r\n"));
	InitPumps();
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);		//send an ASYNC  to allow X'ing out if desired.

	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;

	//SendWHOICustom("/2z2200R\r", FLUIDICS_SYNC);			// set valve encoder to zero (at port 7) [actually use 2200 to avoid wraparound]
	//cone
	desiredValvePosition = 2; // valve to cone (7 to 2, CW=1)
	valveDirection = 1; //"CW"; 
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000);

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));	// set syr speed fast (0.25 min/syr) to debubble
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	syringeVolume = 5.0 / 3.0;	
	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
//	k = sprintf_s(buffer, 20, "/1A%d", step2fullSyr/3); // don't remove too much
//	k += sprintf_s(buffer+k, 20-(k), "R\r");				// aspirate some
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	//needle
	desiredValvePosition = 1; // valve to needle (2 to 1, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000);

	SendWHOICustom(buffer, FLUIDICS_SYNC);

	syringeVolume = 8.0 / 3.0;	
	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
//	k = sprintf_s(buffer, 20, "/1A%d", step2fullSyr/3+step2fullSyr/5); // don't remove too much
//	k += sprintf_s(buffer+k, 20-(k), "R\r");				// aspirate some more
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(5000);											// needle is narrow so wait to let it fill...


	/*//sample tube backflush with first part
	desiredValvePosition = 8; // valve to sample tube8 (1 to 8, CCW=2
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);		// backflush with first part (water): valve to sample tube (1 to 8, CW=1)
	k = sprintf_s(buffer, 20, "/1A%d", step2fullSyr/3);
	k += sprintf_s(buffer+k, 20-(k), "R\r");				// expel some
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	*/

	//to outside(7) with all of it (including air if present)
	desiredValvePosition = 7; // valve to outside (1 to 7, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000);

	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 

	syringeVolume = init_syringeVolume;
	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished Debubble()\r\n"));
	return;
}
//-----------------------------------------------------------------------------
void DebubbleAndRefill(void) {

	char buffer[20]; //, *valveDirection;
	int k;
	int valveDirection;
	int init_syr_sampling_speed;
	int steps;
	double coneVolume;
	double needleVolume;

	if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
		ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
		ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
	}
	else {
		ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
		ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - in DebubbleAndRefill (~2 min) \r\n"));
	InitPumps();
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);
	init_syr_sampling_speed = syr_sampling_speed;

	//cone
	DEBUG_MESSAGE_EXT(_T(" - debubbling from cone...\r\n"));
	desiredValvePosition = 2; // valve to cone (7 to 2, CW=1)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));	// set syr speed fast (0.25 min/syr) to debubble
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	

	coneVolume = 2; //mL to withdraw from cone to get air out
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/coneVolume)));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	//needle
	DEBUG_MESSAGE_EXT(_T(" - debubbling from needle...\r\n"));
	desiredValvePosition = 1; // valve to needle (2 to 1, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);

	needleVolume = 1; //mL to withdraw from needle to get air out
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(coneVolume + needleVolume))));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(3000);											// needle is narrow so wait to let it fill...

	/* //to outside(7) with all of it (including air if present)
	desiredValvePosition = 7; // valve to outside (1 to 7, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	// set syr speed slower (0.5 min/syr) because motor was stopping (and then going wrongly afterwards...) 
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 
*/

	if  (fill3newSheathState & 1) 
		return;
	else {
		
			if (!debubbleWithSample2) {
				DEBUG_MESSAGE_EXT(_T(" - 'Don't refill' option ... so Debubble is finished\r\n"));
			return;
			}

	// Now replace the volume lost during Debubble, with SW sample
			//FIRST finish debubble by expelling waste
	//to outside(7) with all of it (including air if present)
	desiredValvePosition = 7; // valve to outside (1 to 7, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	// set syr speed slower (0.5 min/syr) because motor was stopping (and then going wrongly afterwards...) 
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 

	DEBUG_MESSAGE_EXT(_T(" - replacing volume from primary intake...\r\n"));
	desiredValvePosition = samplePortNumber; // valve to SW sample (7 to 8, CW)
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if going to port 5 - is CCW best direction?
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	// note - take more refill than was lost via Debubble, so that not all need be sent to needle (in case there is some air)
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(coneVolume + needleVolume + 0.5))));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	// this delay should be a sample-tube-length variable... but for now go back to 2 s (if it doesn't fill completely that should be not so bad.
	Sleep(2000); // let it finish filling before moving valve
	// needle
	desiredValvePosition = 1; // valve to needle (8 to 1, CW=1)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	DEBUG_MESSAGE_EXT(_T(" - now Refilling\r\n"));
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(0.5))));  //leave 0.5 ml in syringe in case of air
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished DebubbleAndRefill()\r\n"));

	return;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DebubbleCartridge(void) {

	char buffer[20]; //, *valveDirection;
	int k;
	int valveDirection;
	int init_syr_sampling_speed;
	int steps;
	double coneVolume;
	//double needleVolume;

	if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
		ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
		ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
	}
	else {
		ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
		ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - in DebubbleCartridge  \r\n"));
	InitPumps();
	SendWHOICustom("/2N2R\r", FLUIDICS_SYNC);
	init_syr_sampling_speed = syr_sampling_speed;

	//cone
	DEBUG_MESSAGE_EXT(_T(" - debubbling from cartridge...\r\n"));
	desiredValvePosition = 5; // valve to cartridge (7 to 5, CW=1)
	valveDirection = 1; //"CW"; //don't go past beads port...
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));	// set syr speed fast (0.25 min/syr) to debubble
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	

	coneVolume = 4.5; //mL to withdraw from cartridge to get air out -- this should be a variable but for now assume 5 ml syr and take a little less than 5 so it 
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/coneVolume)));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	//needle
	/*DEBUG_MESSAGE_EXT(_T(" - debubbling from needle...\r\n"));
	desiredValvePosition = 1; // valve to needle (2 to 1, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);

	needleVolume = 1; //mL to withdraw from needle to get air out
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(coneVolume + needleVolume))));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(3000);	
	// needle is narrow so wait to let it fill...
	*/

	/* //to outside(7) with all of it (including air if present)
	desiredValvePosition = 7; // valve to outside (1 to 7, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	// set syr speed slower (0.5 min/syr) because motor was stopping (and then going wrongly afterwards...) 
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 
*/

	if  (fill3newSheathState & 1) 
		return;
	else {
		
			if (!debubbleWithSample2) {
				DEBUG_MESSAGE_EXT(_T(" - 'Don't refill' option ... so Debubble is finished\r\n"));
			return;
			}

	// Now replace the volume lost during Debubble, with SW sample
			//FIRST finish debubble by expelling waste
	//to outside(7) with all of it (including air if present)
	desiredValvePosition = 7; // valve to outside (1 to 7, CCW=2)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));	// set syr speed fast (0.25 min/syr) to debubble
	//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	// set syr speed slower (0.5 min/syr) because motor was stopping (and then going wrongly afterwards...) 
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 

	DEBUG_MESSAGE_EXT(_T(" - replacing volume from primary intake...\r\n"));
	desiredValvePosition = samplePortNumber; // valve to SW sample (7 to 8, CW)
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if going to port 5 - is CCW best direction?
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	// note - take more refill than was lost via Debubble, so that not all need be sent to needle (in case there is some air)
	//steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(coneVolume + needleVolume + 0.5))));  
	//steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(coneVolume))));  
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(5))));  //for now assume 5 ml syr
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	// this delay should be a sample-tube-length variable... but for now go back to 2 s (if it doesn't fill completely that should be not so bad.
	Sleep(2000); // let it finish filling before moving valve
	// needle
	desiredValvePosition = 1; // valve to needle (8 to 1, CW=1)
	//desiredValvePosition = 5; // valve to cartridge5 (8 to 5, CW=1)  Don't go out 5 because if 5 is plugged (as in normal instrument) it will break syringe.
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	DEBUG_MESSAGE_EXT(_T(" - now Refilling Cartridge\r\n"));
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(0.5))));  //leave 0.5 ml in syringe in case of air
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k+= sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	//SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);				// empty the syringe 
	//0
	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished DebubbleAndRefill()\r\n"));

	return;
	}
}

//-----------------------------------------------------------------------------
void RunSample(void)	{

	char buffer[20];				//, *valveDirectionStr;
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;
	//int testval;
	//double waittime;

	DEBUG_MESSAGE_EXT(_T(" - preparing to Run Sample -- please wait\r\n"));
	
	InitPumps();
	// move to syr bottom (not opto)
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	init_syr_sampling_speed = syr_sampling_speed;

	if (StainAuto && runType == ALT && StainRinseMixingChamber) {//staining alt samples, and on ALT sample, and have option Rinse checked.
		RinseStain(); // rinse from previous sample
	}

	//take in SW sample
	desiredValvePosition = samplePortNumber; // valve to SW sample (7 to 8, CW)
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if alt going to port 5 - is CCW best direction?
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));
	//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5 / (0.2*60));//0.1 aspirates 1/5 syringe in 5 sec; 0.2 gets whole syr in
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5 / (0.4*60));//slow down  from 0.2 because degassing during aspiration and syringe doesn't with sample before moving to needle (and so end of sample is actually sheath).
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	// set syr speed
	//steps = (int)floor((float)(step2fullSyr) / (fullSyringeVolume/(syringeVolume-syringeVolume2skip)));
	steps = (int)(step2fullSyr / (fullSyringeVolume/syringeVolume));
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");	
	SendWHOICustom(buffer, FLUIDICS_SYNC); // aspirate sample into syringe

	if (StainAuto && runType == ALT) {//staining alt samples, and on ALT sample.
		// DON'T recirculate sheath 
			ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
			ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
			DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
		//}
		MixStain(); // add stain and mix
		ChangeHWParam(STIRRER, OFF); //stop mixing stirrer
	}

	if (samplePortNumber == 8)
		Sleep(3000); //short sample tube, but larger samples need more time to aspirate (e.g., 30s for 10 ml). 
	else
		Sleep(60000); //long sample tube needs long time to fill syringe with sample

	//needle
	desiredValvePosition = 1; // valve to needle (8 to 1, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(2000);

	// try to avoid pulsing at low speed by adjusting Proportional (w) and Differential (y) gains (28Mar2011)
	// w250y500 give more pulsing than default (w=1000,y=2500)
	//SendWHOICustom("/1w1250y3000R\r", FLUIDICS_SYNC);			// mayber better than defaults?		
	//SendWHOICustom("/1w1500y3500R\r", FLUIDICS_SYNC);			// worse
	//SendWHOICustom("/1w1350y3250R\r", FLUIDICS_SYNC);			// bad
	//SendWHOICustom("/1w1000y2500R\r", FLUIDICS_SYNC);			// default again -- bad
	//SendWHOICustom("/1w2000y5000R\r", FLUIDICS_SYNC);			// better?
	//SendWHOICustom("/1w3000y5000R\r", FLUIDICS_SYNC);			// more erratic?
	//SendWHOICustom("/1w5000y7500R\r", FLUIDICS_SYNC);			// same?
	//SendWHOICustom("/1w10000y15000R\r", FLUIDICS_SYNC);			// better
	//SendWHOICustom("/1w10000y25000R\r", FLUIDICS_SYNC);			// same?
	//SendWHOICustom("/1w15000y37500R\r", FLUIDICS_SYNC);			// less regular? hung acq program when exiting?
	//SendWHOICustom("/1w1000y2500R\r", FLUIDICS_SYNC);			// default again --= definitely worse than prev setting.
	//SendWHOICustom("/1w10000y25000R\r", FLUIDICS_SYNC);			// this one works best so far... but still some irregular pulsing?
	//syr_sampling_speed = int(32.768 * step2fullSyr / (20*60));	// syr_sampling_speed is an int, so can't use 32.768 - MC
	
	if (runSampleFast) 
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (20.0*60 / runFastFactor));	// e.g., change 1 to 4 to go 4x slower for testing
	else
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (20.0*60));	// e.g., change 1 to 4 to go 4x slower for testing
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);

 /*
	// send command here to start acoustic focusing application (copied C:\rob\proposals\ImageBasedSorting_with_EmulsionMicrofluidics\
	labviewfiles\WHOI_3\application\WHOI_3.exe to C:\Windows\aaaWHOI_3.cmd)
		LPITEMIDLIST pidlWinFiles = NULL;
    LPITEMIDLIST pidlItems = NULL;
    IShellFolder *psfWinFiles = NULL;
    IShellFolder *psfDeskTop = NULL;
    LPENUMIDLIST ppenum = NULL;
    STRRET strDispName;
    TCHAR pszParseName[MAX_PATH];
    ULONG celtFetched;
    SHELLEXECUTEINFO ShExecInfo;
    HRESULT hr;
    BOOL fBitmap = FALSE;

    hr = SHGetFolderLocation(NULL, CSIDL_WINDOWS, NULL, NULL, &pidlWinFiles);

    hr = SHGetDesktopFolder(&psfDeskTop);

    hr = psfDeskTop->BindToObject(pidlWinFiles, NULL, IID_IShellFolder, (LPVOID *) &psfWinFiles);
    hr = psfDeskTop->Release();

    hr = psfWinFiles->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &ppenum);

    while( hr = ppenum->Next(1,&pidlItems, &celtFetched) == S_OK && (celtFetched) == 1)
    {
        psfWinFiles->GetDisplayNameOf(pidlItems, SHGDN_FORPARSING, &strDispName);
        StrRetToBuf(&strDispName, pidlItems, pszParseName, MAX_PATH);
        CoTaskMemFree(pidlItems);
        if(StrCmpI(PathFindExtension(pszParseName), TEXT( ".com")) == 0)
//        if(StrCmpI(PathFindExtension(pszParseName), TEXT( ".exe")) == 0)
//        if(StrCmpI(PathFindFileName( "C:\\Windows\\aaexplor.exe")) == 0)
        {
            fBitmap = TRUE;
            break;
        }
    }

    ppenum->Release();

    if(fBitmap)
    {
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = NULL;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = pszParseName;
        ShExecInfo.lpParameters = NULL;
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_MAXIMIZE;
        ShExecInfo.hInstApp = NULL;

        ShellExecuteEx(&ShExecInfo);
    }

    CoTaskMemFree(pidlWinFiles);
    psfWinFiles->Release();
	//
	*/ // end of shell to call labview exe

	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					// set syr speed slow to run sample
	//SendWHOICustom("/1m30R\r", FLUIDICS_SYNC);	// set syr max current allowed to 30%(init = 100%=2A) -- try to prevent breakage
	//SendWHOICustom("/1m5R\r", FLUIDICS_SYNC);	// set syr max current allowed to 5%(init = 100%=2A, so at 24V = 48W; motor spec=2.5W; 2.5/48=0.052) -- try to prevent breakage
	steps = (int)(step2fullSyr / (step2fullSyr - (fullSyringeVolume/(syringeVolume-syringeVolume2skip))));
	//steps = 1 + (int)floor((float)(step2fullSyr) / (fullSyringeVolume/syringeVolume-syringeVolume2skip)); // push to 1 step before vol2skip 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");				// push sample 
	SendWHOICustom(buffer, FLUIDICS_ASYNC);

	DEBUG_MESSAGE_EXT(_T(" - Running Sample -- now interruptible\r\n"));
	
	syr_sampling_speed = init_syr_sampling_speed;

	return;
}

//-----------------------------------------------------------------------------
void Biocide(void) {

	char buffer[20];
	int k;
	int init_syr_sampling_speed;
	int valveDirection;	
	int steps;
	double init_syringeVolume;
	double biocideVolume;

	DEBUG_MESSAGE_EXT(_T(" - adding Biocide to sheath flow (~ 1 min) \r\n"));
	
	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;

	InitPumps();											// move to valve port7 and syringe opto
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	desiredValvePosition = 3; // valve to biocide (7 to 3, CCW)
	valveDirection = 2; //CCW;
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (1*60));	// set syr speed fast (1 min/5 ml)
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	

	biocideVolume = 1.0; // mL of biocide to inject
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/biocideVolume)));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	
	Sleep(2000);											// let it fill...

	desiredValvePosition = 1; // valve to needle (3 to 1, CCW)
	valveDirection = 2;										
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished Biocide()\r\n"));

	return;
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void Sample2Cone(void) {

	char buffer[20];				//, *valveDirectionStr;
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	

	DEBUG_MESSAGE_EXT(_T(" - in Sample2Cone -- please wait\r\n"));
	InitPumps();
	
	init_syr_sampling_speed = syr_sampling_speed;

	// move to syr bottom (not opto)
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	//take in sample
	desiredValvePosition = 8; // valve to SW sample (7 to 8, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));	// syr speed as fast as possible (2x prev) to suck in Alexandrium cells
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);			
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/sampleVolume)));  
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");				// aspirate 
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	//to cone
	desiredValvePosition = 2; // valve to cone (8 to 2, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					// set syr speed slow to run sample
	SendWHOICustom("/1A1R\r\n", FLUIDICS_ASYNC);			// push syringe down to 1 (won't go to 0)...

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished Sample2Cone()\r\n"));

	return;
}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void RinseStain(void) { // to rinse out tubing to mixing chamber and chamber after staining a sample. 

	char buffer[20];				//, *valveDirectionStr;
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	

	DEBUG_MESSAGE_EXT(_T(" - in RinseStain -- please wait\r\n"));
	InitPumps();
	
	init_syr_sampling_speed = syr_sampling_speed;

	// move to syr bottom (not opto)
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	//take in sample
	desiredValvePosition = 8; // valve to SW sample (7 to 8, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);			
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/sampleVolume2)));// rinse with same volume as sample (ALT)
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");				// aspirate 
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	//to mixing chamber
	desiredValvePosition = 5; // valve to mixing chamber 5 (8 to 5, CCW)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	ChangeHWParam(STIRRER, ON);
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);	// push syringe down to 1 (won't go to 0)...

	// aspirate back from mixing chamber to syringe 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r"); 
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	ChangeHWParam(STIRRER, OFF);
	ChangeHWParam(FILL2bypassFilter, ON);
	ChangeHWParam(FILL3newSheath, ON);
	DEBUG_MESSAGE_EXT(_T(" - using new SW for sheath... \r\n"));
	 
	Sleep(7000); // wait to try to prevent dye from entering sheath filter

	desiredValvePosition = 7; // valve to overboard 7 (5 to 7, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(8000); //3000); // maybe FDA contamination in sheath was because valve was not moved fully away from mix chamber and/or beads before expelling rinse.
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (1.5*60));	// noticed some buildup in mix chamber during expel, so try 3x slower 
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);			

	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished RinseStain()\r\n"));

	return;
}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void EmptyMixingChamber(void) { // to empty mixing chamber to waste 

	char buffer[20];				//, *valveDirectionStr;
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	
	double init_syringeVolume;

	DEBUG_MESSAGE_EXT(_T(" - in EmptyMixingChamber -- please wait\r\n"));
	InitPumps();
	
	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;

	// move to syr bottom (not opto)
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	
	desiredValvePosition = 5; // valve to mixing chamber 5 (7 to 5, CCW)
	valveDirection = 2; //1=CW, 2=CCW;
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);			

	syringeVolume = sampleVolume2;  // empty same volume as sample (ALT)
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/sampleVolume2)));// rinse with same volume as sample (ALT)
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");// aspirate 
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	desiredValvePosition = 7; // valve to overboard 7 (5 to 7, CW)
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);

	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);					// set syr speed slow to run sample
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished EmptyMixingChamber()\r\n"));

	return;
}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void MixStain(void) {

	char buffer[20];				//, *valveDirectionStr;
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	

	init_syr_sampling_speed = syr_sampling_speed;

	//sample is in syringe now and valve is at port 8, so move valve to 5 (mixing chamber)
	DEBUG_MESSAGE_EXT(_T(" - starting MixStain();  \r\n"));
	desiredValvePosition = 5; // valve to mixing chamber (8 to 5, CCW)
	valveDirection = 2; //1= "CW", 2="CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(2000);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60)); //same as for sample aspiration
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);			
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC); //ASYNC to allow sleep next; push syringe down to 1 (won't go to 0)...
	Sleep(3000); // wait 3 sec
	ChangeHWParam(STIRRER, ON); //start stirrer before adding stain
	FluidicsRoutine(FLUIDICS_ADD_STAIN, 0);
	Sleep(stainTime*1000);//incubate 
	DEBUG_MESSAGE_EXT(_T(" - let it stain ... \r\n"));

	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/(sampleVolume2 + 0.02))));// 20 ul stain was added...
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r"); 
	SendWHOICustom(buffer, FLUIDICS_SYNC);	

	syr_sampling_speed = init_syr_sampling_speed;

	DEBUG_MESSAGE_EXT(_T(" - finished MixStain(); back to RunSample() \r\n"));

	return;
}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void AddStain(void) {

	int stainSolenoidOpenTime;// ms to keep stain solenoid open 
	DEBUG_MESSAGE_EXT(_T(" - in AddStain -\r\n"));

	//add stain by opening solenoid1 for stainSolenoidOpenTime ms
	stainSolenoidOpenTime = 500;//100;//2000; 
	ChangeHWParam(FILL1microinjector, ON);  
	Sleep(stainSolenoidOpenTime);
	ChangeHWParam(FILL1microinjector, OFF);
	DEBUG_MESSAGE_EXT(_T(" - finished AddStain()\r\n"));
	//Sleep(stainTime*1000);//Sleep(30000);//incubate 30 sec
	//DEBUG_MESSAGE_EXT(_T(" - let it stain ... \r\n"));

	return;
}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void Clorox(void) {

	char buffer[20];
	int k;
	//int i;
	//int init_syr_sampling_speed;
	int valveDirection;
	int steps;
	//double init_syringeVolume;
	double cloroxVolume; // mL of Clorox to fill sample tubing (1 mL for short tube)
	double rinseVolume; // mL of new sample to rinse Clorox from sample tubing (2.5 mL for short tube)
	//int Sleeptime; // calculate time needed for moves???
	DEBUG_MESSAGE_EXT(_T(" - Clorox to sample tube; 60 s; rinse with samples \r\n"));
	cloroxVolume = 1.0;
	rinseVolume = 2.0;

	InitPumps();											// move to valve port7 and syringe opto
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	desiredValvePosition = 4; // valve to Clorox (7 to 4, CCW)
	valveDirection = 2; //CCW;
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (1*60));	// set syr speed fast (1 min/5 ml)
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/cloroxVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");	 
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(2000);											// let it fill...
	desiredValvePosition = 8; // valve to SAMPLE TUBE (4 to 8, CCW=2)
	valveDirection = 2;										// valve to SAMPLE TUBE (4 to 8, CCW=0)
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000); 
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	// let Clorox sit in sample tubing for 60 sec...
	Sleep(60000);
	
	// now rinse syringe with new sample(s)
	int i;
	DEBUG_MESSAGE_EXT(_T(" - rinsing sample tube with 3 samples... \r\n"));
	for (i = 0; i < 3; i++) {
		if (i > 0) { //after the first time, move valve from 7 to 8 and then aspirate sample
			desiredValvePosition = 8; // valve to sample tube(7 to 8, CW=1)
			valveDirection = 1;	
			MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		}
		steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/rinseVolume))); 
		k = sprintf_s(buffer, 20, "/1A%d", steps);
		k += sprintf_s(buffer+k, 20-(k), "R\r");
		SendWHOICustom(buffer, FLUIDICS_SYNC);
		Sleep(2000);

		desiredValvePosition = 7; // valve to outside(8 to 7, CCW=2)
		valveDirection = 2;	
		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(2000);
		SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);// push syringe down to 1 (won't go to 0)...
	}
	return;
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void Beads(void) {

	if (!fluidsActive)
		return;

	char buffer[20];
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;
	double init_syringeVolume; 
	double seawaterVolume1;
	double beadsVolume;
	double seawaterVolume2;


	DEBUG_MESSAGE_EXT(_T(" - starting Beads -- please wait\r\n"));

	ChangeHWParam(STIRRER, ON);

	if  (fill3newSheathState & 1) { // if fill3newSheath is checked, DON'T recirculate sheath 
		ChangeHWParam(FILL3newSheath, ON); //new SW for sheath
		ChangeHWParam(FILL2bypassFilter, ON); //don't filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Non-recirculating sheath \r\n"));
	}
	else {
		ChangeHWParam(FILL3newSheath, OFF); //normal (recirculating) sheath flow
		ChangeHWParam(FILL2bypassFilter, OFF); //filter after flow cell
		DEBUG_MESSAGE_EXT(_T("Recirculating sheath \r\n"));
	}

	InitPumps();											// move to valve port7 and syringe opto
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...

	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;

	//SendWHOICustom("/1V50000\r", FLUIDICS_SYNC);			// set syr speed fast 
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (1*60));	// set syr speed fast (1 min/5 ml)
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
		// first get some filtered SW from cone
	desiredValvePosition = 2; // valve to cone2 (SW first) (7 to 2, CW)
	valveDirection = 1; //CW;
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	//seawaterVolume1 = 0.4; // mL SW to get from the cone for an internal bead sample
	seawaterVolume1 = 1; // mL SW to get from the cone for an internal bead sample
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/seawaterVolume1))); 
	k = sprintf_s(buffer, 20, "/1P%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r"); 
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(2000);											
		// then some beads
	desiredValvePosition = 6; // valve from cone2 to beads6, CW
	valveDirection = 1;										// valve from cone2 to beads6, CW
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	beadsVolume = 0.4; // mL to get from bead reservoir
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/beadsVolume))); 
	k = sprintf_s(buffer, 20, "/1P%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(2000);											
		//then more SW
	desiredValvePosition = 2; // valve from beads6 to cone2 , CCW
	valveDirection = 2;										// valve from beads6 to cone2 , CCW
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	//seawaterVolume2 = 0.4; // mL SW to get from cone to mix with beads
	seawaterVolume2 = 1; // mL SW to get from cone to mix with beads
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/seawaterVolume2))); 
	k = sprintf_s(buffer, 20, "/1P%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
	Sleep(2000);											
	ChangeHWParam(STIRRER, OFF);
		// then to needle to run as sample
	desiredValvePosition = 1; // valve from cone2 to needle1 , CCW
	valveDirection = 2;										
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (20.0*60));	// set syr speed slow (20 min/5 ml)
	if (runSampleFast) 
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (20.0*60 / runFastFactor));	// e.g., change 1 to 4 to go 4x slower for testing
	else
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (20.0*60));	// e.g., change 1 to 4 to go 4x slower for testing
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);

	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);	

	SendWHOICustom("/1A1R\r\n", FLUIDICS_ASYNC);			// push syringe down to 1 (won't go to 0)...
				
	syringeVolume = init_syringeVolume;
	beadSampleTotalVolume = (seawaterVolume1 + beadsVolume + seawaterVolume2);
	syr_sampling_speed = init_syr_sampling_speed;

	//DEBUG_MESSAGE_EXT(_T(" - finished Beads()\r\n"));

	return;
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void FlushSampleTube(void) {
	//Get 1 ml filtered SW from cone2 and backflush Nitex with it.
	//Refill system by getting 1.5 ml sample8 and sending 1 ml of it fast to flow cell. 
	//Discard the rest to waste7 (in case air bubble has been acquired).
	//This leaves sample tube filled with new sample.

	if (!fluidsActive)
		return;
	char buffer[20];
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	
	double init_syringeVolume;

	DEBUG_MESSAGE_EXT(_T(" - in FlushSampleTube -- please wait\r\n"));
	InitPumps();											// move to valve port7 and syringe opto
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	
	
	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;
	
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));// syr speed as fast as possible (2x prev) to suck in Alexandrium cells
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	desiredValvePosition = 2; // outside7 to cone2 (7 to 2, CCW)
	valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	syringeVolume = 1; 
	//steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	steps = (int)(step2fullSyr / (fullSyringeVolume/(syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC); //aspirate 1 ml from cone2

	desiredValvePosition = samplePortNumber; //8; // cone2 to sampletube8, CW)
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if alt going to port 5 - is CCW best direction?
	//valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to backflush Nitex
	if (samplePortNumber == 5)
		Sleep(15000);

	syringeVolume = 1.5;
	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); // pull in 1.5 ml of sample
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");				
	SendWHOICustom(buffer, FLUIDICS_SYNC); //pull in 1.5 ml sample
	if (samplePortNumber == 5)
		Sleep(15000);

	desiredValvePosition = 1; // sampletube8 to flowcell1, CW)
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if alt going to port 5 - is CCW best direction?
	//valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	syringeVolume = 0.5;
	//steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	steps = (int)(step2fullSyr / (fullSyringeVolume/(syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r"); 
	SendWHOICustom(buffer, FLUIDICS_SYNC); //refill system by sending 1 ml to flowcell

	desiredValvePosition = 7; // flowcell1 to outside7, CCW)
	if (samplePortNumber == 8)
		valveDirection = 2; //"CW";
	else
		valveDirection = 1; //"CCW"; check this... if alt going to port 5 - is CCW best direction?
	//valveDirection = 2; //"CCW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000);
	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to leave sample tubing filled with new SW

	Sleep(1000);
	DEBUG_MESSAGE_EXT(_T(" - finished FlushSampleTube()\r\n"));

	syringeVolume = init_syringeVolume;
	syr_sampling_speed = init_syr_sampling_speed;

	return;

}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void PrimeSampleTube(void) {
	//Get syringeVolume from sample tube and push to exhaust (port 7)
	//re-add filling of needle(1) with sample before expelling dummy sample (Dec 2015)
	if (!fluidsActive)
		return;
	char buffer[20];
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	
	double init_syringeVolume;
	int sleeptime;

	DEBUG_MESSAGE_EXT(_T(" - in PrimeSampleTube -- please wait\r\n"));
	InitPumps();											// move to valve port7 and syringe opto
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	Sleep(5000); 
	
	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;
	
//	if (samplePortNumber == 8)
//		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));// syr speed as fast as possible (2x prev) to suck in Alexandrium cells
//	else
//		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (1.25*60));// syr speed not so fast, to avoid high vacuum that sucks beads from adjacent port6
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));// syr speed not so fast, to avoid high vacuum that sucks beads from adjacent port6
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	desiredValvePosition = samplePortNumber; // outside7 to sample port (7 to 8normal or 5alt)
	//valveDirection = 2; //"CCW";
	if (samplePortNumber == 8)
		valveDirection = 1; //"CW";
	else
		valveDirection = 2; //"CCW"; check this... if alt going to port 5 - is CCW best direction? [Rob - it seems that it passes by 6=beads port...]
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000); 
	//syringeVolume = 1; 
	//steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	steps = (int)(step2fullSyr / (fullSyringeVolume/(syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC); //Pull SW from sample port to syringe
	sleeptime = 1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed));
	Sleep(sleeptime); 
//	if (samplePortNumber == 8)
//		Sleep(3000); //short sample tube
		Sleep(8000); //short sample tube
		//SendWHOICustom("/1M3000R\r", FLUIDICS_SYNC);	// wait M ms (max = 32000)	
//	else
//		Sleep(20000); //long sample tube needs long time to fill syringe with sample
		//SendWHOICustom("/1M32000R\r", FLUIDICS_SYNC);	// wait M ms (max = 32000)	

		// fill the needle with sample before expelling
	desiredValvePosition = 1; // sampletube(8) to needle(1), CW
	valveDirection = 1; //"CW";
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	//coneVolume = 2; //mL to withdraw from cone to get air out
	steps = (int)floor(((float)(step2fullSyr) / (fullSyringeVolume/1)));  //1 ml dummy sample pushed out needle to fill it before real sample.
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");
	SendWHOICustom(buffer, FLUIDICS_SYNC);
		// end of needle-filling 

	//desiredValvePosition = 7; // cone2 to outside 7, CW)
	desiredValvePosition = 7; // needle(1) to outside 7, CCW)
	valveDirection = 2; //"CCW";
	//valveDirection = 1; //"CW";
	if (samplePortNumber == 8)
		valveDirection = 2; //"CCW";
	else
		valveDirection = 1; //"CW"; 

	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000); 

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.25*60));// syr speed fast again to empty
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe
	sleeptime = 10000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
	Sleep(sleeptime); 

//	syringeVolume = 1.5;
//	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); // pull in 1.5 ml of sample
//	k = sprintf_s(buffer, 20, "/1A%d", steps);
//	k += sprintf_s(buffer+k, 20-(k), "R\r");				
//	SendWHOICustom(buffer, FLUIDICS_SYNC); //pull in 1.5 ml sample
//
//	desiredValvePosition = 1; // sampletube8 to flowcell1, CW)
//	valveDirection = 1; //"CW";
//	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
//	syringeVolume = 0.5;
//	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
//	k = sprintf_s(buffer, 20, "/1A%d", steps);
//	k += sprintf_s(buffer+k, 20-(k), "R\r"); 
//	SendWHOICustom(buffer, FLUIDICS_SYNC); //refill system by sending 1 ml to flowcell

//	desiredValvePosition = 7; // flowcell1 to outside7, CCW)
//	valveDirection = 2; //"CCW";
//	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
//	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to leave sample tubing filled with new SW

	Sleep(1000);
	DEBUG_MESSAGE_EXT(_T(" - finished PrimeSampleTube()\r\n"));

	syringeVolume = init_syringeVolume;
	syr_sampling_speed = init_syr_sampling_speed;

	return;

}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void ReplaceFromSec(void) {
	//Get syringeVolume from sample tube and push to exhaust (port 7)

	if (!fluidsActive)
		return;
	char buffer[20];
	int k;
	int init_syr_sampling_speed;
	int valveDirection;
	int steps;	
	double init_syringeVolume;
	int sleeptime;

	DEBUG_MESSAGE_EXT(_T(" - in ReplaceFromSec -- please wait\r\n"));
	InitPumps();
	// move to valve port7 and syringe opto
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));
	SendWHOICustom("/1A1R\r\n", FLUIDICS_SYNC);			// push syringe down to 1 (won't go to 0)...
	Sleep(5000); 
	
// replace sample volume
	DEBUG_MESSAGE_EXT(_T(" - replacing sample volume...\r\n"));

	init_syringeVolume = syringeVolume;
	init_syr_sampling_speed = syr_sampling_speed;
	
	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));// syr speed not so fast, to avoid high vacuum that sucks beads from adjacent port6
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	desiredValvePosition = secValvePort; // to secondary intake from ??? - does it matter?
	if (samplePortNumber == 8)
		valveDirection = 2; //"CCW";
	else
		valveDirection = 1; //"CW"; 
	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000); 
	//syringeVolume = 1; 
	steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
	k = sprintf_s(buffer, 20, "/1A%d", steps);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC); //Pull SW from sample port to syringe
	//sleeptime = 3000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
	sleeptime = (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
	Sleep(sleeptime); 
	//long sample tube needs long time to fill syringe with sample
	//SendWHOICustom("/1M32000R\r", FLUIDICS_SYNC);	// wait M ms (max = 32000)	

	desiredValvePosition = samplePortNumber; // cone2 to outside 7, CW)
	//valveDirection = 1; //"CW";
	if (samplePortNumber == 8)
		valveDirection = 2; //"CCW";
	else
		valveDirection = 1; //"CW"; 

	MoveValve(buffer, 20, desiredValvePosition, valveDirection);
	Sleep(5000); 

	syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));// syr speed
	k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
	k += sprintf_s(buffer+k, 20-(k), "R\r");			
	SendWHOICustom(buffer, FLUIDICS_SYNC);

	SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to the primary port
	//sleeptime = 3000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
	sleeptime = (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
	Sleep(sleeptime); 
	//Sleep(15000); // 15 sec for complete expulsion of media

	// if prime sample tube, replace that volume
	if (primeSampleTube) {
		DEBUG_MESSAGE_EXT(_T(" - replacing volume from priming...\r\n"));
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));// syr speed not so fast, to avoid high vacuum that sucks beads from adjacent port6
		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
		k += sprintf_s(buffer+k, 20-(k), "R\r");			
		SendWHOICustom(buffer, FLUIDICS_SYNC);

		desiredValvePosition = secValvePort; // to secondary intake from ??? - does it matter?
		if (samplePortNumber == 8)
			valveDirection = 2; //"CCW";
		else
			valveDirection = 1; //"CW"; 
		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000); 
		//syringeVolume = 1; 
		steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
		k = sprintf_s(buffer, 20, "/1A%d", steps);
		k += sprintf_s(buffer+k, 20-(k), "R\r");			
		SendWHOICustom(buffer, FLUIDICS_SYNC); //Pull SW from sample port to syringe
		//sleeptime = 3000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
		sleeptime = (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
		Sleep(sleeptime); 
		//Sleep(15000); //long sample tube needs long time to fill syringe with sample
		//SendWHOICustom("/1M32000R\r", FLUIDICS_SYNC);	// wait M ms (max = 32000)	

		desiredValvePosition = samplePortNumber; // cone2 to outside 7, CW)
		//valveDirection = 1; //"CW";
		if (samplePortNumber == 8)
			valveDirection = 2; //"CCW";
		else
			valveDirection = 1; //"CW"; 

		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000); 

		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));// syr speed
		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
		k += sprintf_s(buffer+k, 20-(k), "R\r");			
		SendWHOICustom(buffer, FLUIDICS_SYNC);

		SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to the primary port
		//sleeptime = 3000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
		sleeptime = (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
		Sleep(sleeptime);
		//Sleep(15000); // 15 sec for complete expulsion of media
	}

	// if debubble and refill, replace that volume
	if (debubbleWithSample) {
		if (!debubbleWithSample2) {
			DEBUG_MESSAGE_EXT(_T(" - debubble but don't refill...\r\n"));
			return;
		}
		if (debubbleWithSample2) {
		DEBUG_MESSAGE_EXT(_T(" - replacing volume from debubble and refill...\r\n"));
		}
		// code below modified from debubble and refill
		desiredValvePosition = secValvePort; //samplePortNumber; // valve to SW sample (7 to 8, CW)
		if (primValvePort == 8)
			valveDirection = 2; //"CCW";
		else
			valveDirection = 1; //"CW"; check this... if going to port 5 - is CCW best direction?
		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(10000);

		// note - take more refill than was lost via Debubble, so that not all need be sent to needle (in case there is some air)
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));
		//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	
		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
		k += sprintf_s(buffer+k, 20-(k), "R\r");			
		SendWHOICustom(buffer, FLUIDICS_SYNC);

		syringeVolume = 11.0 / 3.0;	
		steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
		k = sprintf_s(buffer, 20, "/1A%d", steps);
		k += sprintf_s(buffer+k, 20-(k), "R\r");
//	k = sprintf_s(buffer, 20, "/1A%d", (step2fullSyr/3)+(step2fullSyr/5)+(step2fullSyr/5)); // ---more volume than Debubble
//	k += sprintf_s(buffer+k, 20-(k), "R\r");				
		SendWHOICustom(buffer, FLUIDICS_SYNC);
		Sleep(15000); // let it finish filling before moving valve
	
		// to primValvePort
		desiredValvePosition = 1; // valve to needle (8 to 1, CW=1)
		if (primValvePort == 8)
			valveDirection = 1; //"CW";
		else
			valveDirection = 2; //"CCW"; check this... if going to port 5 - is CCW best direction?
		//valveDirection = 1; //"CW";
		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
		Sleep(5000);
		//DEBUG_MESSAGE_EXT(_T(" - now Refilling\r\n"));
//		k = sprintf_s(buffer, 20, "/1A%d", step2fullSyr/5); // don't need to push all to needle, so some is sent overboard next operation.
//		k += sprintf_s(buffer+k, 20-(k), "R\r");				
//		SendWHOICustom(buffer, FLUIDICS_SYNC);
		// note - take more refill than was lost via Debubble, so that not all need be sent to needle (in case there is some air)
		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.75*60));
		//syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));	
		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
		k += sprintf_s(buffer+k, 20-(k), "R\r");			
		SendWHOICustom(buffer, FLUIDICS_SYNC);
		
		SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to the primary port
//		Sleep(15000);
		sleeptime = (1000 * (int)floor((float)(10754458 * (syringeVolume/5.0) / syr_sampling_speed)));
		Sleep(sleeptime); 


		// old code below
//		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));// syr speed not so fast, to avoid high vacuum that sucks beads from adjacent port6
//		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
//		k += sprintf_s(buffer+k, 20-(k), "R\r");			
//		SendWHOICustom(buffer, FLUIDICS_SYNC);

//		desiredValvePosition = secValvePort; // to secondary intake from ??? - does it matter?
//		if (samplePortNumber == 8)
//			valveDirection = 2; //"CCW";
//		else
//			valveDirection = 1; //"CW"; 
//		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
//		Sleep(10000); 
		//syringeVolume = 1; 
//		syringeVolume = 11/3;
//		steps = (int)floor(((float)(step2fullSyr) / (5.0/syringeVolume))); 
//		k = sprintf_s(buffer, 20, "/1A%d", steps);
//		k += sprintf_s(buffer+k, 20-(k), "R\r");
//		SendWHOICustom(buffer, FLUIDICS_SYNC); //Pull SW from sample port to syringe
		//sleeptime = 10000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
//		sleeptime = 5000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
//		Sleep(sleeptime); 
		//Sleep(20000); //long sample tube needs long time to fill syringe with sample
		//SendWHOICustom("/1M32000R\r", FLUIDICS_SYNC);	// wait M ms (max = 32000)	

//		desiredValvePosition = primValvePort; // push to primary sample intake
		//valveDirection = 1; //"CW";
//		if (primValvePort == 8)
//			valveDirection = 1; //"CW";
//		else
//			valveDirection = 1; //"CW"; 

//		MoveValve(buffer, 20, desiredValvePosition, valveDirection);
//		Sleep(5000); 

//		syr_sampling_speed = int(32.768 * step2fullSyr/syringeSize*5.0 / (0.5*60));// syr speed
//		k = sprintf_s(buffer, 20, "/1V%d", syr_sampling_speed);
//		k += sprintf_s(buffer+k, 20-(k), "R\r");			
//		SendWHOICustom(buffer, FLUIDICS_SYNC);

//		SendWHOICustom("/1A0R\r\n", FLUIDICS_SYNC);	// empty the syringe to the primary port
//		sleeptime = 5000 + (1000 * (int)floor((float)(10754458 * (syringeVolume/5) / syr_sampling_speed)));
//		Sleep(sleeptime); 
	//	Sleep(15000); // 15 sec for complete expulsion of media
	}

	DEBUG_MESSAGE_EXT(_T(" - finished ReplaceFromSec()\r\n"));

	syringeVolume = init_syringeVolume;
	syr_sampling_speed = init_syr_sampling_speed;

	return;

}
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
static void AdjustFocus(int focusStep) {

	int k;
	char buffer[20];

	if (focusStep < 0) {
		k = sprintf_s(buffer, 20, "/3z%d", -focusStep+1000);  // have to reset encoder upwards so it doesn't try to go below 0
		k += sprintf_s(buffer+k, 20-(k), "R\r");
		SendWHOICustom(buffer, FLUIDICS_ASYNC);
		Sleep(100);
		SendWHOICustom(buffer, FLUIDICS_ASYNC);
		Sleep(100);
		k = sprintf_s(buffer, 20, "/3V10000D%d", -focusStep); //move motor backward
		k += sprintf_s(buffer+k, 20-(k), "R\r");
	} else {
		k = sprintf_s(buffer, 20, "/3V10000P%d", focusStep); // move motor forward
		k += sprintf_s(buffer+k, 20-(k), "R\r");
	}

	SendWHOICustom(buffer, FLUIDICS_ASYNC);
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
static void AdjustLaser(int focusStep) {
	int k;
	char buffer[20];

	if (focusStep < 0) {
		k = sprintf_s(buffer, 20, "/4z%d", -focusStep+1000);  // have to reset encoder upwards so it doesn't try to go below 0
		k += sprintf_s(buffer+k, 20-(k), "R\r");
		SendWHOICustom(buffer, FLUIDICS_ASYNC);
		Sleep(100);
		SendWHOICustom(buffer, FLUIDICS_ASYNC);
		Sleep(100);
		k = sprintf_s(buffer, 20, "/4V50000D%d", -focusStep); //move motor backward
		k += sprintf_s(buffer+k, 20-(k), "R\r");
	} else {
		k = sprintf_s(buffer, 20, "/4V50000P%d", focusStep); // move motor forward
		k += sprintf_s(buffer+k, 20-(k), "R\r");
	}

	SendWHOICustom(buffer, FLUIDICS_ASYNC);
}

/*
//-----------------------------------------------------------------------------
// from matlab function move_valveStr.m  --- construct a string 
// [valveStr, current_valve_port] = move_valveStr(s, current_valve_port, desired_valve_port, valve_direction)
//-----------------------------------------------------------------------------
//static void MoveValveString(char *s, int strlen, int currentValvePort, int desiredValvePosition, int valveDirection) {
static void MoveValveString(char *s, int strlen) {

	int steps, diffPorts;	
	int j;
	int step2port = 256;
	


	switch(valveDirection) { 
		//case "CW" :
		//case "cw" :
		case 1 :
		diffPorts = desiredValvePosition - currentValvePosition;
		if (diffPorts < 0) {
			diffPorts += 8;}
		steps = step2port * diffPorts;		
		j = sprintf_s(s, strlen, "/2P%d", steps);
		break;
		//case "CCW" :
		//case "ccw" :
		case 0 :
		diffPorts =  currentValvePosition - desiredValvePosition;
		if (diffPorts < 0) {
			diffPorts += 8;}
		steps = step2port * diffPorts;		
		j = sprintf_s(s, strlen, "/2z%dD%d", steps, steps);	// reset encoder upwards to allow it to decrement;  D for going ccw (e.g., 7 ro 6)
		break;
	}
	// add R\r at end of command to Run it
	//j += sprintf_s(s+j, strlen-(j), "R\\r");
	j += sprintf_s(s+j, strlen-(j), "R\r");
	
	return;
}
*/

//-----------------------------------------------------------------------------
// this moves the valve (using MoveValveString to construct the command) and updates currentValvePosition. 
//-----------------------------------------------------------------------------
static void MoveValve(char *s, int strlen, int desiredValvePosition, int valveDirection) {
	int steps, diffPorts;	
	int j;
	int step2port = 256;
	
		switch(valveDirection) { 

		case 1 : //CW
		diffPorts = desiredValvePosition - currentValvePosition;
		if (diffPorts < 0) {
			diffPorts += 8;}
		steps = step2port * diffPorts;		
		//j = sprintf_s(s, strlen, "/2M30000P%d", steps); // test adding a delay as part of the string sent -- this way works
		j = sprintf_s(s, strlen, "/2P%d", steps);
		break;

		case 2 :  //CCW
		diffPorts =  currentValvePosition - desiredValvePosition;
		if (diffPorts < 0) {
			diffPorts += 8;}
		steps = step2port * diffPorts;		
		j = sprintf_s(s, strlen, "/2z%dD%d", steps, steps);	// reset encoder upwards to allow it to decrement;  D for going ccw (e.g., 7 ro 6)
		break;
	}	
	j += sprintf_s(s+j, strlen-(j), "R\r"); // add R\r at end of command to Run it
	currentValvePosition = desiredValvePosition; 
	SendWHOICustom(s, FLUIDICS_SYNC); // do the valve move
	
	return;
}

//-----------------------------------------------------------------------------
// function to query fluidics system for certain conditions
//-----------------------------------------------------------------------------
int FluidicsQuery(int fluidicsQuery) {

	if (!fluidsActive) {
		DEBUG_MESSAGE_EXT(_T("Error in FluidicsQuery: fluidsActive is FALSE\r\n"));
		return true;
	}

	DWORD nBytes = 0;
	int charbuflen = 20;
	char readstr[20];
	char tmpbuf[20];
	int i;
	CString str;

	switch(fluidicsQuery) {
		case IS_SYRINGE_IDLE:     
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Syringe idle?"));
			// Query the EZServo
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1Q\r", 4, &nBytes, NULL), TRUE, _T("Query Kloehn Idle: Serial error writing\r\n"));
			break;
		
		case IS_VALVE_IDLE:     
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Valve idle?"));
			// Query the EZServo
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/2Q\r", 4, &nBytes, NULL), TRUE, _T("Query Kloehn Idle: Serial error writing\r\n"));
			break;
		
		case IS_SYRINGE_OPTO_OPEN:     // the ' char
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Syringe opto open?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1?4\r", 5, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error writing\r\n"));
			break;

		case IS_FILTER_SLIDER_OPTO_OPEN:     // the ' char
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: filter slider opto open?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/5?4\r", 5, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error writing\r\n"));
			break;
		
		case SYRINGE_LOCATION:     
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Syringe location?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1?8\r", 5, &nBytes, NULL), TRUE, _T("Query Kloehn Syringe Location: Serial error writing\r\n"));
			break;

		case IS_FILTER_OPTO_OPEN:     // the ' char
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Filter opto open?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/5?4\r", 5, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error writing\r\n"));
			break;
		
		case FILTER_LOCATION:     
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Filter location?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/5?8\r", 5, &nBytes, NULL), TRUE, _T("Query Kloehn Syringe Location: Serial error writing\r\n"));
			break;

		case ENCODER_POSITION:     
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Encoder position?\r\n"));
			CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/2?8\r", 5, &nBytes, NULL), TRUE, _T("Query Encoder Position: Serial error writing\r\n"));
			break;

		default:
			if (fluidicsVerbose) DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Unrecognized input choice\r\n"));
			readstr[nBytes] = '\0';			// terminate the string
			str.Format(_T("returned bytes: %s\r\n"), readstr);
			break;
	}

	Sleep(15);							// wait a few ms (12 or so min)
	
	readstr[0] = '\0';
	// was gettign 7 bytes...
	CALL_CHECKED_RETFALSE_EXT(ReadFile(hSerialFluidics, &readstr, 18, &nBytes, NULL), TRUE, _T("Query Kloehn: Serial error reading\r\n"));

	if (fluidicsVerbose) {
		readstr[nBytes] = '\0';			// terminate the string
		str = _T("FluidicsQuery: ");
		str += readstr;
		str += _T("\r\n");
		DEBUG_MESSAGE_EXT(str);
	}

	switch(fluidicsQuery) {

	case IS_SYRINGE_IDLE: 
		//need to be smarter here & look for specific bit in status byte, not whole byte of ' or @
		if (readstr[3] == 96) return SYRINGE_IDLE;
		else if (readstr[3] == '@') return SYRINGE_BUSY;
		else return SYRINGE_IDLE_ERR;

		break;
	
	case IS_VALVE_IDLE: 
		//need to be smarter here & look for specific bit in status byte, not whole byte of ' or @
		if (readstr[3] == 96) return VALVE_IDLE;
		else if (readstr[3] == '@') return VALVE_BUSY;
		else return VALVE_IDLE_ERR;

		break;

	case IS_SYRINGE_OPTO_OPEN:
		if ( readstr[5] == '5')				// it's a '5'
			return SYRINGE_OPTO_CLOSED;
		else if ( readstr[5] == '1')		// it's a '1'
			return SYRINGE_OPTO_OPEN;
		else {
			DEBUG_MESSAGE_EXT(_T("ERROR IN FluidicsQuery: unexpected return string in opto\r\n"));
			return SYRINGE_OPTO_ERR;
		}

		break;
	
	case IS_FILTER_OPTO_OPEN:
		if ( readstr[5] == '5')				// it's a '5'
			return FILTER_OPTO_CLOSED;
		else if ( readstr[5] == '1')		// it's a '1'
			return FILTER_OPTO_OPEN;
		else {
			DEBUG_MESSAGE_EXT(_T("ERROR IN FluidicsQuery: unexpected return string in opto\r\n"));
			return FILTER_OPTO_ERR;
		}

		break;
	
	case SYRINGE_LOCATION:     
		// want all from readstr[4] to last one that is a digit
		// UGLY.
		if (readstr[1] != '/' || readstr[2] != '0')	// then the string is not good for some reason
		{
			DEBUG_MESSAGE_EXT(_T("ERROR IN FluidicsQuery: unexpected return string in syringe_location\r\n"));
			// need a better way to flag an error here. calling function can reasonably expect actual location to be zero too.
			return 0;
		}
		// otherwise parse out the string into an integer
		for (i = 4; i < charbuflen; i++)	// start at 5th element
		{
			if (!isdigit(readstr[i]) ) break;	// when not an integer, or end of array, break out of for loop
			else tmpbuf[i-4] = readstr[i];
		}
		// i-1 is the last digit in readstr
		tmpbuf[i-4] = '\0';	// terminate the char array

		syringeLocation = atoi(tmpbuf);

		if (fluidicsVerbose) {
			str.Format(_T("syringeLocation: %d\r\n"), syringeLocation);
			DEBUG_MESSAGE_EXT(str);
		}
		
		return syringeLocation;
		break;
	
	case FILTER_LOCATION:     
		// want all from readstr[4] to last one that is a digit
		// UGLY.
		if (readstr[1] != '/' || readstr[2] != '0')	// then the string is not good for some reason
		{
			DEBUG_MESSAGE_EXT(_T("ERROR IN FluidicsQuery: unexpected return string in syringe_location\r\n"));
			// need a better way to flag an error here. calling function can reasonably expect actual location to be zero too.
			return 0;
		}
		// otherwise parse out the string into an integer
		for (i = 4; i < charbuflen; i++)	// start at 5th element
		{
			if (!isdigit(readstr[i]) ) break;	// when not an integer, or end of array, break out of for loop
			else tmpbuf[i-4] = readstr[i];
		}
		// i-1 is the last digit in readstr
		tmpbuf[i-4] = '\0';	// terminate the char array

		filterLocation = atoi(tmpbuf);

		if (fluidicsVerbose) {
			str.Format(_T("filterLocation: %d\r\n"), syringeLocation);
			DEBUG_MESSAGE_EXT(str);
		}
		
		return filterLocation;
		break;

	case ENCODER_POSITION:     
		if (readstr[1] != '/' || readstr[2] != '0')	// then the string is not good for some reason
		{
			DEBUG_MESSAGE_EXT(_T("ERROR IN FluidicsQuery: unexpected return string in Encoder position\r\n"));
			// need a better way to flag an error here. calling function can reasonably expect actual location to be zero too.
			return 0;
		}
		// otherwise parse out the string into an integer
		for (i = 4; i < charbuflen; i++)	// start at 5th element
		{
			if (!isdigit(readstr[i]) ) break;	// when not an integer, or end of array, break out of for loop
			else tmpbuf[i-4] = readstr[i];
		}
		// i-1 is the last digit in readstr
		tmpbuf[i-4] = '\0';	// terminate the char array

		encoderPosition = atoi(tmpbuf);
		
		if (fluidicsVerbose) {
			str.Format(_T("EncoderPosition: %d\r\n"), encoderPosition);
			DEBUG_MESSAGE_EXT(str);
		}

		return encoderPosition;
		break;

	default:
		DEBUG_MESSAGE_EXT(_T("FluidicsQuery: Unrecognized input choice\r\n"));
		
		readstr[nBytes] = '\0';			// terminate the string
		str.Format(_T("returned bytes: %s\r\n"), readstr);
		DEBUG_MESSAGE_EXT(str);
		break;
	}

	// you should only get here if there is a problem. you should have returned before this.
	return QUERY_FLUIDICS_ERROR;
}


void DrainFilters(void) {

DEBUG_MESSAGE_EXT(_T("This function not yet installed for IFCB\r\n"));


}

#endif