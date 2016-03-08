//-----------------------------------------------------------------------------
//  IFCB Project
//	Daq_accesio.cpp
//	Martin Cooper
//
//	Handles Acces IO card interface
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"
#include "Daq.h"
//#include "Data.h"

#ifdef ACCESIO
#include "Daq_accesio.h"
#include "ACCES32.H"
#include "stdafx.h"
#include "IfcbDlg.h"

char	daqPort[STRING_PARAM_LENGTH];		// dummy - not used

static ACCES_SETTINGS accesSettings;

//-----------------------------------------------------------------------------
// calls all the different functions to set up various ACCESIO operations
//-----------------------------------------------------------------------------
bool DAQInit(void) {

	// set the PMT HVs to zero
	SetDac(PMT1_DAC, 0.0);
	SetDac(PMT2_DAC, 0.0);
	SetDac(PMT3_DAC, 0.0);

	// get info from ACCESIO card
	accesSettings.jumpers = (unsigned char)InPortB(AIO_BASE+0x08);		// jumper configuration readback
	accesSettings.adrange = accesSettings.jumpers & 0x07;				// d2,d1,d0 are 5/10, uni/bip, 16/8
	accesSettings.dacBrange = (accesSettings.jumpers>>3) & 0x01;		// d4,d3 are 5/10 for daca,b
	accesSettings.dacArange = (accesSettings.jumpers>>4) & 0x01;

	// INIT THE ACCESIO CARD
	InPortB(AIO_BASE + 0x1D);											// master reset

	OutPortB(AIO_BASE + 2, 0xF0);			// end scan at CH15, start scan at CH0
	EnableTriggeredScan();					// set up the ADC channels

	// disarm the comparator just in case
	Set_ACCESIO_DigOut(0, LO);

	return true;
}

//-----------------------------------------------------------------------------
// calls all the different functions to shutdown any ACCESIO things
//	important because it helps keep lines from being high, etc., after program ends
//-----------------------------------------------------------------------------
bool DAQShutdown(void) {

	DEBUG_MESSAGE_EXT(_T(" NO CODE YET FOR DAQShutdown in Daq_accesio.cpp\r\n"));

	return true;
}

//-----------------------------------------------------------------------------
// sends a pulse on a digital line that can be piped into the camera to trigger it
//-----------------------------------------------------------------------------
void GenerateTrigger(void) {

	int i;

	// very crude way of generating a pulse
	for (i = 0; i < 10; i++)
		OutPortB(AIO_PORTB, 1);
	OutPortB(AIO_PORTB, 0);
}

//-----------------------------------------------------------------------------
// sets the single digital line out that allows the integrator board to listen to triggers
//	called from Process.cpp, AcqInit()
//-----------------------------------------------------------------------------
void DaqArmTrigger(bool armState) {

	Set_ACCESIO_DigOut(6, !armState);

	return;
}

//-----------------------------------------------------------------------------
unsigned int GetTrigInhibitTime(void) {

	return 0;
}

//-----------------------------------------------------------------------------
void ResetTrigInhibitTime() {
}

//-----------------------------------------------------------------------------
void MoveFocusMirror(int step) {
}

//-----------------------------------------------------------------------------
bool SetFlashlamp(bool on) {
	return on;
}

//-----------------------------------------------------------------------------
// sets the PMT high voltages 
//	input is pmtNum 0 or 1, and a double with the PMT high voltage in volts
//  remember to initialize AO task in DAQInit()
//-----------------------------------------------------------------------------
void SetPMTHV(uint8 pmtNum, double pmtHV) {

	pmtHV *= 5.0;	 // circuit has a 1:5 V divider, so multiply by 5 before applying. 

	if ((pmtNum != PMT1_DAC) && (pmtNum != PMT2_DAC))		// only allow PMT1 & PMT2
		return;

	if ((pmtHV < 0.0) || (pmtHV > 5.0))		// check for illegal voltage
		return;

	SetDac(pmtNum, pmtHV);

	return;	
}

//-----------------------------------------------------------------------------
// performs all the low-level operations to grab analog data from ACCESIO card
//	called from Process.cpp, ProcessTrigger()
//-----------------------------------------------------------------------------
void ReadDAQIntegrated(DataStruct *data) {

	CString str;
	double intAdc[16];

	DEBUG_MESSAGE_EXT(_T(" - in ReadDAQIntegrated\r\n"));


	for (int i = 0; i < 16; i++) {
		intAdc[i] = ((double)(InPort(AIO_BASE)) / 65536.0
			- ((accesSettings.adrange&0x02)?0.5:0.0))		// Adjust for polarity jumper  0.5
			* ((accesSettings.adrange&0x04)?10.0:20.0);		// Adjust for gain jumper  20.0
	}

	data->integAlo = (float)intAdc[0];
	data->integAhi = (float)intAdc[1];
	data->integClo = (float)intAdc[2];
	data->integChi = (float)intAdc[3];

	return;
}

//-----------------------------------------------------------------------------
//sbitd takes data and bit, then returns 0x81 or 0x01 based on data[bit]
//-----------------------------------------------------------------------------
static unsigned char sbitd(unsigned int data, unsigned char bit) {
	
	return (data & (1 << bit)) ? 0x81 : 0x01;
}

//-----------------------------------------------------------------------------
//	DAC will write to the selected DAC
//-----------------------------------------------------------------------------
void SetDac(unsigned char DACnum, double dacV) {

	unsigned int count;
	
	count = (unsigned int)(dacV / ((DACnum ? accesSettings.dacArange : accesSettings.dacBrange) ? 5.0 : 10.0) * 4096);
	count &= 0x0FFF;								// 12-bit DAC
	DACnum++;										// To the hardware, the DACs are 1-based

	OutPortB(AIO_BASE + 9, 0x81);					// DAC load mode = software
	OutPortB(AIO_BASE + 9, sbitd(DACnum, 1) | 2);	// DACnum bit 1
	OutPortB(AIO_BASE + 9, sbitd(DACnum, 0) | 2);	// DACnum bit 0
	OutPortB(AIO_BASE + 9, 0x01);					// unused bit
	OutPortB(AIO_BASE + 9, sbitd(count, 11));		// data bit 11
	OutPortB(AIO_BASE + 9, sbitd(count, 10));		// data bit 10
	OutPortB(AIO_BASE + 9, sbitd(count, 9));		// data bit 9
	OutPortB(AIO_BASE + 9, sbitd(count, 8));		// data bit 8
	OutPortB(AIO_BASE + 9, sbitd(count, 7));		// data bit 7
	OutPortB(AIO_BASE + 9, sbitd(count, 6));		// data bit 6
	OutPortB(AIO_BASE + 9, sbitd(count, 5));		// data bit 5
	OutPortB(AIO_BASE + 9, sbitd(count, 4));		// data bit 4
	OutPortB(AIO_BASE + 9, sbitd(count, 3));		// data bit 3
	OutPortB(AIO_BASE + 9, sbitd(count, 2));		// data bit 2
	OutPortB(AIO_BASE + 9, sbitd(count, 1));		// data bit 1
	OutPortB(AIO_BASE + 9, sbitd(count, 0));		// data bit 0
	OutPortB(AIO_BASE + 9, 0x00);					// end trans
	OutPortB(AIO_BASE + 9, 0x02);					// Load DACs
}

//-----------------------------------------------------------------------------
//	Sets individual digital output lines
//	0 <= pin <= 15
//	STATE = HI or LO
//-----------------------------------------------------------------------------
bool Set_ACCESIO_DigOut(unsigned short pin, int state) {

	if (pin > 15)	{
		// throw a fit - wrong pin	
		MessageBox(0, _T(""), _T("Pin setting error in Set_ACCESIO_DigOut"), MB_OK | MB_ICONSTOP);
		return false;
	}

	// compute new pinstate for this port - state is HI or LO
	unsigned long port;

	if (pin < 8) {
		port = AIO_PORTA;
	} else {
		port = AIO_PORTB;
		pin -= 8;
	}

	unsigned char DIO_pinstate = (char)InPortB(port);				// read the current state
	if (state == HI)
		DIO_pinstate |= (1 << pin);
	else 
		DIO_pinstate &= ~(1 << pin);

	OutPortB(port, DIO_pinstate);

	return true;
}

//-----------------------------------------------------------------------------
//	Reads individual digital output lines
//	0 <= pin <= 15
//-----------------------------------------------------------------------------
static bool Read_ACCESIO_DigOut(unsigned short pin) {

/*	if (pin > 15)	{
		// throw a fit - wrong pin	
		MessageBox(0, _T(""), _T("Pin setting error in Set_ACCESIO_DigOut"), MB_OK | MB_ICONSTOP);
		return false;
	}*/

	// compute new pinstate for this port - state is HI or LO
	unsigned long port;

	if (pin < 8) {
		port = AIO_PORTA;
	} else {
		port = AIO_PORTB;
		pin -= 8;
	}

	unsigned char DIO_pinstate = (char)InPortB(port);				// read the current state

	return ((DIO_pinstate & (1 << pin)) > 0);
}

//-----------------------------------------------------------------------------
// reads the next available value from the FIFO and converts it to volts before returning the reading
//-----------------------------------------------------------------------------
double Read_ACCESIO_AD(void) {

	//scale counts into AD Range Volts
	unsigned short data;

	data = InPort(AIO_BASE);


	return
		((double)(data) / 65536.0
		- ((accesSettings.adrange&0x02)?0.5:0.0))		// Adjust for polarity jumper
		* ((accesSettings.adrange&0x04)?10.0:20.0);	// Adjust for gain jumper

}

//-----------------------------------------------------------------------------
void EnableTriggeredScan(void) {

	OutPortB(AIO_BASE + 1, 0x00);						//reset FIFO
	OutPortB(AIO_BASE + 0x1A, 0x10);					// enable triggered scan mode
	OutPortB(AIO_BASE + 0x1E, 0x40);
	
	// arm the comparator
	Set_ACCESIO_DigOut(0, HI);
}

//-----------------------------------------------------------------------------
void ReadHousekeeping(void) {

	// ADCs
	OutPortB(AIO_BASE + 0x1A, 0x00);					// disable triggered scan mode
	OutPortB(AIO_BASE + 0x1E, 0x00);
	OutPortB(AIO_BASE + 1, 0);							// reset FIFO

	// here force 16 acq when s/w triggering
	for (uint8 i = 0; i < 16; i++) {
		OutPortB(AIO_BASE, 0);
		while (!(InPortB(AIO_BASE + 0x08) & 0xF0));		// wait for FIFO not empty
		Adc[i] = Read_ACCESIO_AD();
	}

	OutPortB(AIO_BASE + 1, 0);							// reset FIFO
	EnableTriggeredScan();

	// digital IO lines
	Flags[FILL1_F] = Read_ACCESIO_DigOut(FILL1);
	Flags[FILL2_F] = Read_ACCESIO_DigOut(FILL2);
	Flags[FILL3_F] = Read_ACCESIO_DigOut(FILL3);
	Flags[STIRRER_F] = Read_ACCESIO_DigOut(STIRRER);
	Flags[PUMP1_F] = Read_ACCESIO_DigOut(PUMP1);
	Flags[PUMP2_F] = Read_ACCESIO_DigOut(PUMP2);
}

//-----------------------------------------------------------------------------
void SetDigital(uint8 flag, uint8 state) {

	CString debugStr;

	debugStr = _T("Setting ");
	switch (flag) {
		case FILL1_F:
			debugStr += _T("Fill1 valve");
			Set_ACCESIO_DigOut(FILL1, state);
			break;
		case FILL2_F:
			debugStr += _T("Fill2 valve");
			Set_ACCESIO_DigOut(FILL2, state);
			break;
		case FILL3_F:
			debugStr += _T("Fill3 valve");
			Set_ACCESIO_DigOut(FILL3, state);
			break;
		case PUMP1_F:
			debugStr += _T("Pump1");
			Set_ACCESIO_DigOut(PUMP1, state);
			break;
		case PUMP2_F:
			debugStr += _T("Pump2");
			Set_ACCESIO_DigOut(PUMP2, state);
			break;
		case STIRRER_F:
			debugStr += _T("Stirrer");
			Set_ACCESIO_DigOut(STIRRER, state);
			break;

		case LASER_F:
		case CAMERA_F:
		case ISOL_F:
		case SPARE_F:
		case FANS_F:
		case FANS_AUTO_F:
		case HUMID_AUTO_F:
			debugStr += _T("illegal digital IO\r\n");
			DEBUG_MESSAGE_EXT(debugStr);
			return;
			break;
	}
	if (state)
		debugStr += _T(" ON\r\n");
	else
		debugStr += _T(" OFF\r\n");
	DEBUG_MESSAGE_EXT(debugStr);
}

#endif