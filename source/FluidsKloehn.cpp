//-----------------------------------------------------------------------------
//  IFCB Project
//	Fluidics.cpp
//
//	Handles serial commands to WHOI custom syringe pump & valve (submersible build)
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"

#ifdef KLOEHN

#include "IfcbDlg.h"
#include "Fluids.h"
#include "FluidsKloehn.h"
#include "FileIo.h"
#include <time.h>
#include <math.h>

char	fluidicsPort[STRING_PARAM_LENGTH];
DCB dcbSerialParams = {0};

int syringeOffset;			//THIS IS IN Fluidics.h, 
int syringeLocation = 0;	// the location of the syringe (bottomed-out = 0)
int encoderPosition = -1;
//int syrOffset = 0; 

static HANDLE hSerialFluidics;
//static int r = 10;
//static CString str;
static int step2port = 256; // number of encoder steps between valve ports
//char valveStr[20];
static int step2fullSyr = 328200; // number of syringe motor encoder steps for 5 ml (empirical)

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

//-----------------------------------------------------------------------------
// A general error handler
//-----------------------------------------------------------------------------
static void DisplayError(void) {

	CString str;

	str.Format(_T("%s: %d"), _T("Fluidics serial port error\r\n"), GetLastError());
	ERROR_MESSAGE_EXT(str);
}

//-----------------------------------------------------------------------------
// function to see if Kloehn is still busy or not
// returns true while idle, false otherwise
//-----------------------------------------------------------------------------
static bool QueryIdle(void) {

	char *outstr = "/1\r\n";
	char readstr[10];
	DWORD dwBytesRead = 0, dwBytesWritten = 0;	// for all file reads & writes

	// send CRLF
	if(!WriteFile(hSerialFluidics, outstr, strlen(outstr), &dwBytesWritten, NULL)) {
		DEBUG_MESSAGE_EXT(_T("QueryKloehnIdle: Serial write error\r\n"));		
		return FALSE;
	}

	// wait a few ms (12 or so min)
	Sleep(15);
	// read serial response from Kloehn - it returns true even if it gets no response (?!)
	if(!ReadFile(hSerialFluidics, &readstr, 7, &dwBytesRead, NULL)) {
		DEBUG_MESSAGE_EXT(_T("QueryKloehnIdle: Serial read error\r\n"));		
		return FALSE;
	}
	if (!dwBytesRead) {
//		DEBUG_MESSAGE_EXT(_T("QueryKloehnIdle: Serial read error\r\n"));		
		return FALSE;
	}

	return (readstr[2] == 96);		// 96 is ASCII decimal for ' sign == idle, 64 is @ sign = busy

	
}

//-----------------------------------------------------------------------------
// function to send commands to Kloehn system & interpret error codes
//-----------------------------------------------------------------------------
static bool SendKloehn(char *str, int syncMode)
{
//	BOOL busy = TRUE;
//	char *outstr = "/1\r\n";
	char readstr[20];
bool verbose = FALSE;

DWORD dwBytesRead = 0, dwBytesWritten = 0;	// for all file reads & writes

/*
	int i;

	// make sure you can talk to the Kloehn
// if this is an ASYNC afer an ASYNC then there's no point in timing out - just sent a bunch of \r\n
	for (i=0; i< 10; i++)	// give it 100 times to wake up
	{
		if (Query_Kloehn_Idle(FALSE) == TRUE)		// this has a few ms delay built in
			break;
	}
	if (i == 10)
	{
		MessageBox(0, "Couldn't wake up Kloehn", "Send_Kloehn", MB_OK | MB_ICONSTOP);
		return FALSE;
	}
*/


	if(!WriteFile(hSerialFluidics, str, strlen(str), &dwBytesWritten, NULL))
	{
		DEBUG_MESSAGE_EXT(_T("SendKloehn: Serial write error\r\n"));
	}


/*
	itoa(dwBytesWritten, str, 10);
	MessageBox(0, str, 
                    "Bytes written", 
                     MB_OK | MB_ICONSTOP);
*/



	// wait a few ms (12 or so min) for reply
	Sleep(15);
	// read serial response from Kloehn
	if(!ReadFile(hSerialFluidics, &readstr, 7, &dwBytesRead, NULL))
	{
		DEBUG_MESSAGE_EXT(_T("SendKloehn: Serial error reading\r\n"));
		return FALSE;
	}

	if ( (readstr[0] == '/') && (readstr[1] == '0') )
	{	// parse the third character
		switch(readstr[2])
		{
		case '@':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Status busy\r\n"));
			break;
		case 96:     // the ' char
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Status OK\r\n"));
			break;

		case 'a':
		case 'A':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Syringe not initialized\r\n"));
			break;
		case 'b':
		case 'B':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Invalid command\r\n"));
			break;
		case 'c':
		case 'C':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Invalid operand\r\n"));
			break;
		case 'd':
		case 'D':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Communication error\r\n"));
			break;
		case 'g':
		case 'G':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Device not initialized\r\n"));
			break;
		case 'i':
		case 'I':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Syringe overload\r\n"));
			break;
		case 'j':
		case 'J':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Valve overload\r\n"));
			break;
		case 'k':
		case 'K':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: No syringe move in valve bypass\r\n"));
			break;
		case 'o':
		case 'O':     
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Command buffer full\r\n"));
			break;

		default:
			if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: Other error\r\n"));
			break;
		}

	}
	else
		if (verbose) DEBUG_MESSAGE_EXT(_T("SendKloehn: bad readstr\r\n"));


//****************
	// the old way of doing it
	// a little sleep just to make sure next cmd won't overrun
//	Sleep(50);

	if (syncMode == FLUIDICS_SYNC)	// then wait here for return OK from kloehn
	{
		while (QueryIdle() == FALSE);	// do this until not idle

// be smarter here with a timeout
	}

	// otherwise pass through and return
	return TRUE;
//****************


//****************
// the new way. with some s/w timeout
// something about this timeout logic isn't working yet. seems to return immediately.
/*
	if (syncMode != FLUIDICS_SYNC)
		return TRUE;

	// wait here for return OK from kloehn
	bool ok = FALSE;
	for (int t = FLUIDICS_SERIAL_TIMEOUT; t--; )  {
		if (QueryIdle())
			ok = TRUE;
	}

	return ok;
*/
}

//-----------------------------------------------------------------------------
void Biocide() {
}

//-----------------------------------------------------------------------------
void Clorox() {
}

//-----------------------------------------------------------------------------
void Beads() {
}

//-----------------------------------------------------------------------------
// Top-level function to pull in and begin dispensing new sample
// includes any debubbling, biocide prep, etc if needed
// include ONLY the fluidics aspects here
//-----------------------------------------------------------------------------
void FluidicsPullNewSample(void) {

// perhaps here is where eventually we add which port stuff is being pulled from
	FluidicsRoutine(FLUIDICS_DEBUBBLE, 0);
	FluidicsRoutine(FLUIDICS_RUN_SAMPLE, 0);

	
/*
	FluidicsRoutine(FLUIDICS_DEBUBBLE, 0);
	if (discreteSampleIntake) {
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_F, 0);
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_F, 0);
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_F, 0);
		FluidicsRoutine(FLUIDICS_RUN_SAMPLE_DRAW_F, SYRINGE_5ML); // pull from valve F
	} else {
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_H, 0);
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_H, 0);
		FluidicsRoutine(FLUIDICS_FLUSH_FLOWCELL_H, 0);
		FluidicsRoutine(FLUIDICS_RUN_SAMPLE_DRAW_H, SYRINGE_5ML);	// pull from valve H
	}
*/
}

//-----------------------------------------------------------------------------
static void KloehnInit(void) {

	DEBUG_MESSAGE_EXT(_T("KloehnInit: beginning initialization\r\n"));
	// simple initialization
	SendKloehn("/1~V10R\r\n", FLUIDICS_SYNC);	// tell the thing it's a 8-way distribution valve
	SendKloehn("/1~Y7\r\n", FLUIDICS_SYNC);		// set init valve position to 7
	SendKloehn("/1V1600\r\n", FLUIDICS_SYNC);	// set top speed
	SendKloehn("/1Y4R\r\n", FLUIDICS_SYNC);		// init syringe after moving to port
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// go to absolute zero
	DEBUG_MESSAGE_EXT(_T("KloehnInit: completed initialization\r\n"));
	return;
}

//-----------------------------------------------------------------------------
bool FluidicsRoutine(int task, int userparam) {

//	char buffer[20];

//	char *valueStr = "";
	// need this active for initialization at startup
	if (!fluidsActive) {
		DEBUG_MESSAGE_EXT(_T("Error in FluidicsRoutine: fluidsActive is FALSE\r\n"));
		return true;
	}

	switch (task) {

		case FLUIDICS_INIT:	// this inits the syringe + valve
			DEBUG_MESSAGE_EXT(_T("Called FLUIDICS_INIT in FluidicsRoutine\r\n"));
			KloehnInit();
			break;

		case FLUIDICS_SET_SYRINGE_OFFSET:	
			// this is where you compute the offset between opto position and dead bottom of syringe
			// only called for WHOICustom syringe
			ERROR_MESSAGE_EXT(_T("Error in FluidicsRoutine: calling a non-Kloehn function in Kloehn mode\r\n"));
			return true;
			break;

		case FLUIDICS_DEBUBBLE:
			Debubble();
			break;

		case FLUIDICS_RUN_SAMPLE:					
			RunSample();
			break;

		case FLUIDICS_DRAIN_FILTERS:	// pull from valve F
			DrainFilters();
			break;

		case FLUIDICS_FLUSH_FLOWCELL:
			FlushFlowcell();
			break;

		case FLUIDICS_FILL_FILTERS:
			for (int i = 0; i < 10; i++)
				FlushFlowcell();
			break;

		case FLUIDICS_STOP:
			SendKloehn("/1T\r\n", FLUIDICS_SYNC);
			break;

		case FLUIDICS_AZIDE:
	//		SendKloehn("/1T\r\n", FLUIDICS_SYNC);		// 
			break;

		case FLUIDICS_SET_VALVE_PORT:
			//	where you put in valve code, use passed valve argument to go to right location
			break;

		default:
			return false;
	}

	return true;
}


// notes for KLOEHN:
// no 'R' after ~Y commands
// no 'R' after V commands

//-----------------------------------------------------------------------------
void Debubble(void) {

	DEBUG_MESSAGE_EXT(_T("DebubbleKloehn: beginning debubbling\r\n"));
	SendKloehn("/1~Y7\r\n", FLUIDICS_SYNC);		// set init valve position to 7
	SendKloehn("/1V800\r\n", FLUIDICS_SYNC);	// set top speed
	SendKloehn("/1Y4R\r\n", FLUIDICS_SYNC);		// init syringe after moving to port A, then init port
	SendKloehn("/1o-2R\r\n", FLUIDICS_SYNC);		// move CCW to port 2
	SendKloehn("/1V3200\r\n", FLUIDICS_SYNC);		// set top speed
	SendKloehn("/1A15000R\r\n", FLUIDICS_SYNC);		// got absolute 15000 (for the cone)
	SendKloehn("/1V1600\r\n", FLUIDICS_SYNC);		// set slower speed
	SendKloehn("/1o7R\r\n", FLUIDICS_SYNC);		// move valve to 7 CW
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// go to abs 0
	SendKloehn("/1V400\r\n", FLUIDICS_SYNC);		// slow down
	SendKloehn("/1o-1R\r\n", FLUIDICS_SYNC);		// move valve CCW to port 1
	SendKloehn("/1A5000R\r\n", FLUIDICS_SYNC);		// draw syringe to 5000 (for the needle)
	SendKloehn("/1M1000R\r\n", FLUIDICS_SYNC);		// wait 1000ms
	SendKloehn("/1V800\r\n", FLUIDICS_SYNC);		// slow down
	SendKloehn("/1o7R\r\n", FLUIDICS_SYNC);		// 
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// 
	SendKloehn("/1M1000R\r\n", FLUIDICS_SYNC);		// 
	DEBUG_MESSAGE_EXT(_T("DebubbleKloehn: completed debubbling\r\n"));

	return;
}

//-----------------------------------------------------------------------------
void RunSample(void) {

	SendKloehn("/1~Y7\r\n", FLUIDICS_SYNC);		// set init valve position to 7 (which is G - waste))
	SendKloehn("/1V1000\r\n", FLUIDICS_SYNC);	// set top speed
	SendKloehn("/1Y4R\r\n", FLUIDICS_SYNC);		// init syringe after moving to port set above
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// go to abs 0
	
	// change valve to whichever sample intake port is needed
	if (discreteSampleIntake)					
		SendKloehn("/1o6R\r\n", FLUIDICS_SYNC);		// Klohen port F
	else
		SendKloehn("/1o8R\r\n", FLUIDICS_SYNC);		// Klohen port H



	SendKloehn("/1M2000\r\n", FLUIDICS_SYNC);		// wait
	SendKloehn("/1V800\r\n", FLUIDICS_SYNC);		// change speed to slower

	// here compute how much to draw in from userparam "sampleVolume" - a double
	char string[30];

	sprintf_s(string, 29, "/1A%dR\r\n", (int)floor(syringeVolume * 9600));
	SendKloehn(string, FLUIDICS_SYNC);
	
//	SendKloehn("/1A5000R\r\n", FLUIDICS_SYNC);		// draw syringe to 15000
//		SendKloehn("/1A15000R\r\n", FLUIDICS_SYNC);		// draw syringe to 15000
//	SendKloehn("/1A48000R\r\n", FLUIDICS_SYNC);		// draw syringe in

		
	SendKloehn("/1o1R\r\n", FLUIDICS_SYNC);		// change valve to flow cell
	SendKloehn("/1V40\r\n", FLUIDICS_SYNC);		// change speed SLOW

	// this next one is async so that it just goes ahead and falls through
	SendKloehn("/1A0R\r\n", FLUIDICS_ASYNC);		// push to abs pos zero
	
	return;	
	
}
	
//-----------------------------------------------------------------------------
void FlushFlowcell(void) {

	SendKloehn("/1~Y7\r\n", FLUIDICS_SYNC);		// set init valve position to 7 (which is G - waste))
	SendKloehn("/1V1000\r\n", FLUIDICS_SYNC);	// set top speed
	SendKloehn("/1Y4R\r\n", FLUIDICS_SYNC);		// init syringe after moving to port set above
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// go to abs 0

	// change valve to whichever sample intake port is needed
	if (discreteSampleIntake)					
		SendKloehn("/1o6R\r\n", FLUIDICS_SYNC);		// Klohen port F
	else
		SendKloehn("/1o8R\r\n", FLUIDICS_SYNC);		// Klohen port H

	SendKloehn("/1V2000\r\n", FLUIDICS_SYNC);		// chnage speed to slower

	// here compute how much to draw in from userparam "syringeVolume" - a double
	char string[30];

	sprintf_s(string, 29, "/1A%dR\r\n", (int)floor(syringeVolume * 9600));
	SendKloehn(string, FLUIDICS_SYNC);

	SendKloehn("/1o1R\r\n", FLUIDICS_SYNC);		// change valve to flow cell
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// push to abs pos zero
}





// this one pulls the correct ml from the sheath & ejects on port G
//-----------------------------------------------------------------------------
void DrainFilters(void) {

	SendKloehn("/1~Y7\r\n", FLUIDICS_SYNC);		// set init valve position to 7 (which is G - waste))
	SendKloehn("/1V1000\r\n", FLUIDICS_SYNC);	// set top speed
	SendKloehn("/1Y4R\r\n", FLUIDICS_SYNC);		// init syringe after moving to port set above
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// go to abs 0

	// change valve to whichever sample intake port is needed
	SendKloehn("/1o1R\r\n", FLUIDICS_SYNC);		// Klohen port F

	SendKloehn("/1V2000\r\n", FLUIDICS_SYNC);		// chnage speed to slower

	// here compute how much to draw in from userparam "syringeVolume" - a double
	char string[30];

	sprintf_s(string, 29, "/1A%dR\r\n", (int)floor(syringeVolume * 9600));
	SendKloehn(string, FLUIDICS_SYNC);

	SendKloehn("/1o7R\r\n", FLUIDICS_SYNC);		// change valve to waste out
	SendKloehn("/1A0R\r\n", FLUIDICS_SYNC);		// push to abs pos zero
}




/*
//-----------------------------------------------------------------------------
// function to see if Kloehn opto is open (unobstructed)
// returns true while open, false otherwise
//-----------------------------------------------------------------------------
static bool QueryOptoOpen(void) {

//	if (!fluidsActive)
//		return true;

	DWORD nBytes = 0;
	char readstr[12];
DEBUG_MESSAGE_EXT(_T("In QueryWHOICustomOpto"));

	// Query the EZServo
	//CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1?4\r\n", 6, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error writing"));
	CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1?4\r", 6, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error writing"));

	Sleep(15);							// wait a few ms (12 or so min)
	
	readstr[0] = '\0';
	CALL_CHECKED_RETFALSE_EXT(ReadFile(hSerialFluidics, &readstr, 10, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error reading"));
	//CALL_CHECKED_RETFALSE_EXT(ReadFile(hSerialFluidics, &readstr, 9, &nBytes, NULL), TRUE, _T("Query Kloehn Opto Open: Serial error reading"));

	if (fluidicsVerbose) {
		CString str;
		readstr[nBytes] = '\0';			// terminate the string
		str = _T("QueryWHOICustomOpto: ");
		str += readstr;
		DEBUG_MESSAGE_EXT(str);
	}

	// a kludge: look at 5th character. if a '5' then opto closed, if a '1' opto is open - unobstructed
// BE SMARTER THAN THIS, later
	if ( readstr[5] == 53)	// it's a '5'
		return FALSE;
	else if ( readstr[5] == 49) // it's a '1'
		return TRUE;
	else {
		DEBUG_MESSAGE_EXT(_T("ERROR IN QueryOptoOpen: unexpected return string"));
		return FALSE;
	}

	
}



*/


/*
//-----------------------------------------------------------------------------
// function to see current location of Kloehn syringe 
// returns this value
//-----------------------------------------------------------------------------
static int QuerySyringeLocation(void) {

//	if (!fluidsActive)
//		return true;

	DWORD nBytes = 0;
	char readstr[20];
DEBUG_MESSAGE_EXT(_T("In QuerySyringeLocation"));

	// Query the EZServo
	CALL_CHECKED_RETFALSE_EXT(WriteFile(hSerialFluidics, "/1?8\r\n", 6, &nBytes, NULL), TRUE, _T("Query Kloehn Syringe Location: Serial error writing"));

	Sleep(15);							// wait a few ms (12 or so min)
	
	readstr[0] = '\0';
	CALL_CHECKED_RETFALSE_EXT(ReadFile(hSerialFluidics, &readstr, 20, &nBytes, NULL), TRUE, _T("Query Kloehn Syringe Location: Serial error reading"));

	if (fluidicsVerbose) {
		CString str;
		readstr[nBytes] = '\0';			// terminate the string
		str = _T("QuerySyringeLocation: ");
		str += readstr;
		DEBUG_MESSAGE_EXT(str);
	}

return(0);
	
}
*/
//-----------------------------------------------------------------------------
int	FluidicsQuery(int fluidicsQuery) {

	return SYRINGE_IDLE;
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

#endif