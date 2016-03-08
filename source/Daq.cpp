//-----------------------------------------------------------------------------
//  IFCB Project
//	Daq.cpp
//	Martin Cooper
//
//	Handles Acces IO card interface
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"
#include "Daq.h"
#include "ACCES32.H"


char	daqHardware[STRING_PARAM_LENGTH];


ACCES_SETTINGS accesSettings;

//-----------------------------------------------------------------------------
void TriggerPulse(void) {

	int i;

	// very crude way of generating a pulse
	for (i = 0; i < 10; i++)
		OutPortB(AIO_PORTB, 1);
	OutPortB(AIO_PORTB, 0);
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
void DAC(unsigned char DACnum, double dacV) {

	unsigned int count;
	
	count = (unsigned int)(dacV / ((DACnum ? accesSettings.dacArange : accesSettings.dacBrange) ? 5.0L : 10.0L) * 4096);
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
bool Set_ACCESIO_DigOut(unsigned short pin, int state) {

	unsigned char DIO_pinstate;
	
	if (pin > 8 || pin < 0)	{
		// throw a fit - wrong pin	
		MessageBox(0, _T(""), _T("Pin setting error in Set_ACCESIO_DigOut"), MB_OK | MB_ICONSTOP);
		return false;
	}

	// compute new pinstate for this port - state is HI or LO
	if (state == HI)
		DIO_pinstate = DIO_pinstate | (1 << pin);
	else 
		DIO_pinstate = DIO_pinstate & ~(1 << pin);

	OutPortB(AIO_BASE + 0x10, DIO_pinstate);

	return true;
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
bool DAQInit(void) {

	// set the PMT HVs to zero
	DAC(0, 0.0);

	// get info from ACCESIO card
	accesSettings.jumpers = (unsigned char)InPortB(AIO_BASE+0x08);		// jumper configuration readback
	accesSettings.adrange = accesSettings.jumpers & 0x07;				// d2,d1,d0 are 5/10, uni/bip, 16/8
	accesSettings.dacBrange = (accesSettings.jumpers>>3) & 0x01;		// d4,d3 are 5/10 for daca,b
	accesSettings.dacArange = (accesSettings.jumpers>>4) & 0x01;


	// INIT THE ACCESIO CARD
    InPortB(AIO_BASE + 0x1D);											// master reset

	// set up the ADC channels
	OutPortB(AIO_BASE + 2,0xF0);	//end scan at CH15, start scan at CH0
	// no need to program gain at AIO_BASE+4, AIO_BASE+5 as it would be invalid
	// oversampling defaults to x1
	// now OK to go to enabling/disabling scan in actual routine


	// disarm the comparator just in case
	Set_ACCESIO_DigOut(0, LO);

	return true;
}
