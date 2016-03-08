//-----------------------------------------------------------------------------
//  IFCB Project
//	DaqMcc.cpp
//	Martin Cooper
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"

#ifdef DAQ_MCC

#include "Daq.h"
#include "IfcbDlg.h"
#include "GraphTab.h"
#include "Fluids.h"
#include "FluidsWHOI.h"
#include "Process.h"
#include "tcp.h"

char				daqPort[STRING_PARAM_LENGTH];

static CString		debugStr;				
static char			command[COMMAND_LENGTH];
static char			response[RESPONSE_LENGTH];
static HANDLE		hSerialDaq;
static bool			serialRunning = false;
static volatile bool			serialBusy = false;

static void			SetDac(VALUE val, uint16 channel);
static void			SetPumpVolts(VALUE val, uint16 addr);
static void			SetAdcRate(VALUE val, uint16 addr);
static void			SetPga(VALUE val, uint16 addr);
static void			WriteEeprom(VALUE val, uint16 addr);
static void			SetDigital(VALUE val, uint16 flag);
static double		PumpCountToVolts(uint8 count);

//-----------------------------------------------------------------------------
//	This array is a copy of all the hardware settings, 
//	independant of the .cfg file variables.
//	To change one, simply call ChangeHWParam(PARAM, value)
//	where PARAM is taken from the HW_PARAMS list defined in DaqMcc.h
//	If intrument is not running, the parameter is changed instantly,
//	otherwise it happens in the callback funtion
//
//	To add a new parameter, add a new entry in HW_PARAMS and in hardware[]
//-----------------------------------------------------------------------------
HARDWARE_PARAM hardware[] = {
	{SetDac,		0,	false, PMTA_DAC		},	// PMTA
	{SetDac,		0,	false, PMTB_DAC		},	// PMTB
	{SetDac,		0,	false, PMTC_DAC		},	// PMTC
	{SetPumpVolts,	0,	false, 0			},	// PUMP_DAC
	{SetDac,		0,	false, TRIGA_DAC	},	// TRIGA
	{SetDac,		0,	false, TRIGB_DAC	},	// TRIGB
	{SetDac,		0,	false, TRIGC_DAC	},	// TRIGC
	{SetDac,		0,	false, TRIGD_DAC	},	// TRIGD
	{SetAdcRate,	0,	false, 0			},	// ADC_RATE
	{WriteEeprom,	0,	false, 108			},	// INTEGRATOR_SETTLE
	{WriteEeprom,	0,	false, 110			},	// TRIG_MASK
	{SetPga,		0,	false, 0			},	// PGA
	{WriteEeprom,	0,	false, 20			},	// FAN_THRESH
	{WriteEeprom,	0,	false, 22			},	// FAN_HYSTERESIS
	{WriteEeprom,	0,	false, 16			},	// HUMID_THRESH
	{WriteEeprom,	0,	false, 18			},	// HUMID_HYSTERESIS
	{SetDigital,	0,	false, HUMID_AUTO	},	// HUMID_AUTO,
	{WriteEeprom,	0,	false, 24			},	// VALVE_DELAY
	{SetDigital,	0,	false, FLASH		},	// FLASH
	{SetDac,		0,	false, DAC_FLASH	},	// FLASH_DAC
	{WriteEeprom,	0,	false, 26			},	// FLASH_DELAY
	{WriteEeprom,	0,	false, 28			},	// FLASH_LENGTH
	{WriteEeprom,	0,	false, 30			},	// CAMERA_LENGTH
	{WriteEeprom,	0,	false, 116			},	// TRIG_TIMEOUT
	{WriteEeprom,	0,	false, 118			},	// PULSE_STRETCH
	{WriteEeprom,	0,	false, 120			},	// MAX_PULSE_STRETCH
	{WriteEeprom,	0,	false, 122			},	// HAS_CAMERA
	{SetDigital,	0,	false, FILL1microinjector		},	// FILL1microinjector
	{SetDigital,	0,	false, FILL2bypassFilter		},	// FILL2bypassFilter
	{SetDigital,	0,	false, FILL3newSheath		},	// FILL3newSheath
	{SetDigital,	0,	false, ISOL			},	// ISOL
	{SetDigital,	0,	false, SPARE		},	// SPARE
	{SetDigital,	0,	false, FANS			},	// FANS
	{SetDigital,	0,	false, STIRRER		},	// STIRRER
	{SetDigital,	0,	false, LASER		},	// LASER
	{SetDigital,	0,	false, CAMERA		},	// CAMERA
	{SetDigital,	0,	false, PUMP1		},	// PUMP1
	{SetDigital,	0,	false, PUMP2		}	// PUMP2
};

//-----------------------------------------------------------------------------
//	Comms note: all commands beginning:
//		'*' go to the housekeeping board
//		'#' go to the analog board
//		'$' go to the pump driver board
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static inline void WaitForSerial(void) {

	while (serialBusy);
}

//-----------------------------------------------------------------------------
static void FlushComms(void) {

	DWORD nBytes;
	char c;

	do {
		ReadFile(hSerialDaq, &c, 1, &nBytes, NULL);
	} while (nBytes);
}

//-----------------------------------------------------------------------------
//	sends command down the serial port, puts the reply into response
//-----------------------------------------------------------------------------
static bool SendCommand(void) {

	if (!serialRunning)	{						// no comms until it's all been set up
		ERROR_MESSAGE_EXT(_T("DAQ error - serial not running\r\n"));
		return false;
	}

	WaitForSerial();

	DWORD nBytes;
	serialBusy = true;
	bool ok = true;

	strcat_s(command, COMMAND_LENGTH, "\r\n");		// append "\r\n"

	// send the command
	WriteFile(hSerialDaq, command, strlen(command), &nBytes, NULL);
	ok = (nBytes == strlen(command));

	if (ok) {
		// get the response a byte at a time until I get '\n'
		int i;
		for (i = 0; i < RESPONSE_LENGTH; i++)  {
			do {
				ReadFile(hSerialDaq, response + i, 1, &nBytes, NULL);
			} while (nBytes == 0);

/*			ReadFile(hSerialDaq, response + i, 1, &nBytes, NULL);
			// ReadFile returns true if there's a timeout
			if (!nBytes) 								// there was a timeout
				break;*/

			if (response[i] == '\n') {					// success
				ok = true;
				break;
			}
		}
		response[i + 1] = '\0';							// terminate string
	} else
		ERROR_MESSAGE_EXT(_T("SendCommand error\r\n"));


	// report any comms errors
	if (!ok) {
		serialRunning = false;
		switch (command[0]) {
			case '#':
				ERROR_MESSAGE_EXT(_T("Housekeeping board comms error\r\n"));
				break;
			case '*':
				ERROR_MESSAGE_EXT(_T("Analog board comms error\r\n"));
				break;
			case '$':
				ERROR_MESSAGE_EXT(_T("Pump drive board comms error\r\n"));
				break;
		}
	}

	serialBusy = false;
	return ok;
}

//-----------------------------------------------------------------------------
bool DAQShutdown(void) {

	ChangeHWParam(PUMP1, OFF);
	ChangeHWParam(PUMP2, OFF);
	ChangeHWParam(CAMERA, OFF);
	//ChangeHWParam(FANS, OFF);
	ChangeHWParam(LASER, OFF);
	ChangeHWParam(STIRRER, OFF);
	SetHWParams();

	CloseHandle(hSerialDaq);								// close serial port

	return true;
}

//-----------------------------------------------------------------------------
void StartAcquisition(void) {

	sprintf_s(command, COMMAND_LENGTH, "*S1");
	SendCommand();
}

//-----------------------------------------------------------------------------
void StopAcquisition(void) {

	unsigned int run, inhibit;

	sprintf_s(command, COMMAND_LENGTH, "*S0");
	SendCommand();
	Sleep(100);
	FlushComms();
	
	// read the trigger stats
	SendCommand();
	char *p = strchr(response, 'S');					// skip any possible capture characters at start
	if (p) {
		sscanf_s(p, "S=%u,%u\r", &run, &inhibit);		
		gRunTime = (double)run / 460800.0;					// convert to seconds
		gInhibitTime = (double)inhibit / 460800.0;			// convert to seconds
	} else
		gRunTime = gInhibitTime = 0.0;
	Sleep(100);
	FlushComms();
}

//-----------------------------------------------------------------------------
//	Sends a single special character to re-arm the trigger
//-----------------------------------------------------------------------------
void DaqArmTrigger(void) {

	static const char arm[] = {0x18, '\0'};
	static DWORD nBytes;

	WriteFile(hSerialDaq, arm, 1, &nBytes, NULL);
}

//-----------------------------------------------------------------------------
void GenerateTrigger(void) {

	sprintf_s(command, COMMAND_LENGTH, "*X");
	SendCommand();
}

//-----------------------------------------------------------------------------
static void WriteEeprom(VALUE val, uint16 addr) {

	unsigned int addr2, value2;

	debugStr.Format(_T("Writing E2: %d=%d\r\n"), addr, val.i);
	DEBUG_MESSAGE_EXT(debugStr);

	command[0] = addr >= 100 ? '*' : '#';			// select which board - if addr >=100 then it's analog, else hk
	sprintf_s(command + 1, COMMAND_LENGTH, "E%d,%d", addr, val.i);
	SendCommand();
	sscanf_s(response + 1, "E%d=%d\r", &addr2, &value2);

	if ((addr2 != addr) || (value2 != val.i)) {
		debugStr.Format(_T("Error writing E2: wrote %d=%d read %d=%d\r\n"), addr, val.i, addr2, value2);
		ERROR_MESSAGE_EXT(debugStr);
		FlushComms();
	}
}

//-----------------------------------------------------------------------------
static void SetDac(VALUE val, uint16 channel) {

	double dacV = val.d;

	debugStr.Format(_T("Setting DAC %d to %fV\r\n"), channel, dacV);
	DEBUG_MESSAGE_EXT(debugStr);

	if (channel > TRIGD_DAC)
		return;

	uint16 dacVal;

	switch (channel) {
		case PMTA_DAC:
		case PMTB_DAC:
		case PMTC_DAC:
			dacVal = (uint16)(dacV * 4095.0 * 4.6 / 5.0);			// convert to DAC value (12 bit) - allow for pot
			*command = '#';
			break;
		case DAC_FLASH:
			dacVal = (uint16)(dacV * 4095.0 / 5.0);					// convert to DAC value (12 bit)
			*command = '#';
			break;
		case TRIGA_DAC:
		case TRIGB_DAC:
		case TRIGC_DAC:
		case TRIGD_DAC:
			dacVal = (uint16)(dacV * 4095.0 / 3.3);					// convert to DAC value (12 bit)
			*command = '*';
			channel -= TRIGA_DAC;
			break;
		default:
			return;
	}

	dacVal = min(dacVal, 4095);
	sprintf_s(command + 1, COMMAND_LENGTH, "D%d,%d", channel, dacVal);	// and send
	SendCommand();

	int channel2, dacVal2;
	sscanf_s(response + 1, "D%d=%d\r", &channel2, &dacVal2);			// check response

	if (dacVal2 != dacVal) {
		debugStr.Format(_T("Error writing DAC: sent %s - read %s\r\n"), command, response);
		ERROR_MESSAGE_EXT(debugStr);
		FlushComms();
	}
}

//-----------------------------------------------------------------------------
static double ReadDac(uint8 channel) {

	unsigned int dacVal;

	if (channel < TRIGA_DAC) {		// hk board
		sprintf_s(command, COMMAND_LENGTH, "#D%d", channel);
		SendCommand();
		sscanf_s(response, "#D%d=%d\r", &channel, &dacVal);
		return (double)dacVal * 5.0 / 4095.0;
	} else {
		sprintf_s(command, COMMAND_LENGTH, "*D%d", channel - TRIGA_DAC);
		SendCommand();
		sscanf_s(response, "*D%d=%d\r", &channel, &dacVal);
		return (double)dacVal * 3.3 / 4095.0;
	}
}

//-----------------------------------------------------------------------------
void ReadHumidTemp(void) {

	unsigned int a[2];
	double x;

	sprintf_s(command, COMMAND_LENGTH, "#A");
	SendCommand();
	if (sscanf_s(response, "#A=%d,%d", a, a + 1) != 2)
		Temperature = Humidity = 0.0;

	x = (double)a[1] * 5.0 / 65535.0 * 1000.0;
//	Adc[1] = (double)a[1] * 5.0 / 65535.0;
//	x = (x - 2.87) / 0.559; 
//	Temperature = -0.22 * x * x * x - 0.0437 * x * x - 11.7 * x + 17.1; 
	Temperature = -0.0218 * x + 79.87;

	x = (double)a[0] * 5.0 / 65535.0 * 1000.0;
	//Humidity = -1.564e-9 * x * x * x + 1.205e-5 * x * x + 8.22e-3 * x - 15.6; 
	Humidity = 0.03892 * x - 41.98; 

}

//-----------------------------------------------------------------------------
bool CheckHumidity(void) {

	ReadHumidTemp();

	if (Humidity < humidityThreshold)
		return true;

	char message[40];
	sprintf_s(message, 40, "H%.1lf\r\n", Humidity);
	//SendTcpMessage(message);// rem this out re Martin's advice.

	ERROR_MESSAGE_EXT(_T("Humidity too high\r\n"));

	return false;
}

//-----------------------------------------------------------------------------
void ReadDAQIntegrated(DataStruct *data) {

	static int t = 0;
	int adc[8], flight;
	DWORD nBytes, totBytes;

	// read analogue board ADC values and convert to meaningful units

	// parse the ADC data
	if (hasCamera) {
		sprintf_s(command, COMMAND_LENGTH, "*A");
		SendCommand();
		sscanf_s(response, "*A=%d,%d,%d,%d,%d,%d,%d,%d\r\n", adc, adc + 1, adc + 2, adc + 3, adc + 4, adc + 5, adc + 6, adc + 7);
	} else {											// data is in binary for extra speed
		sprintf_s(command, COMMAND_LENGTH, "*A\r\n");
		WriteFile(hSerialDaq, command, strlen(command), &nBytes, NULL);
		for (totBytes = 0; totBytes < 37;) {
			ReadFile(hSerialDaq, response + totBytes, 1, &nBytes, NULL);
			totBytes += nBytes;
		}

		char *p = response + 3;
		union {
			char	byte[4];
			int		word;
		}a;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 4; j++)
				a.byte[j] = *p++;
			adc[i] = a.word;
		}
	}

	// read time of flight and convert to meaningful units
	sprintf_s(command, COMMAND_LENGTH, "*W");
	SendCommand();
	if (sscanf_s(response, "*W=%d\r\n", &flight) == 1)
		data->time_of_flight = (float)flight / (float)0.46;				// time of flight in us, if the hardware measures it

	// ADC full scale is +-5V which comes in as +- 8388608
	// then compensate for i/p divider (/ 4) and PGA
	// don't compensate for PGA (Rob)
	data->peakA = (float)adc[0] / (419430.4f);// * (float)adcPga);			// scaled to volts @ i/p
	data->peakB = (float)adc[1] / (419430.4f);// * (float)adcPga);
	data->peakC = (float)adc[2] / (419430.4f);// * (float)adcPga);
	data->peakD = (float)adc[3] / (419430.4f);// * (float)adcPga);
	data->integA = (float)adc[4] / (1677721.6f);// * (float)adcPga);		// scaled to volts @ PMTx INT
	data->integB = (float)adc[5] / (1677721.6f);// * (float)adcPga);
	data->integC = (float)adc[6] / (1677721.6f);// * (float)adcPga);
	data->integD = (float)adc[7] / (1677721.6f);// * (float)adcPga);
	
	// read analogue board run and inhibit timers
	sprintf_s(command, COMMAND_LENGTH, "*M");
	SendCommand();
	unsigned int run, inhibit;
	if (sscanf_s(response + 1, "M=%u,%u\r", &run, &inhibit) == 2) {
		data->runTime = (double)run / 460800.0;					// convert to seconds
		data->inhibitTime = (double)inhibit / 460800.0;			// convert to seconds
	} else {
		data->runTime = data->inhibitTime = 0.0;
	}

	if ((GraphData.GraphType == GraphData.GRAPH_NONE) || !GraphData.running)
		return;

	// alignment graph plotting
	switch (GraphData.GraphType) {
		case GraphData.GRAPH_ROI_ADC:												// x data from ADC, y data set in Analyse()
			GraphData.data[0][GraphData.nextIn] = data->integA;						// x = CHA
			GraphData.data[1][GraphData.nextIn] = data->integB;						// x = CHB
			GraphData.data[2][GraphData.nextIn] = data->integC;						// x = CHC
			GraphData.data[3][GraphData.nextIn] = data->integD;						// x = CHD
			debugStr.Format(_T("Y = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), data->integA, data->integB, data->integC, data->integD); 		

/*			// Invented data
			GraphData.data[0][GraphData.nextIn] = (double)t * 0.01;					// x = CHA low
			GraphData.data[1][GraphData.nextIn] = (double)t * 0.015;				// x = CHC low
			GraphData.data[2][GraphData.nextIn] = (double)t * 0.02;					// x = CHA high
			GraphData.data[3][GraphData.nextIn] = (double)t * 0.025;				// x = CHC high
			t++;*/

			debugStr.Format(_T("X = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), 
					GraphData.data[0][GraphData.nextIn], GraphData.data[1][GraphData.nextIn], GraphData.data[2][GraphData.nextIn], GraphData.data[3][GraphData.nextIn]);
			break;
		case GraphData.GRAPH_ROI_PEAK:												// x data from ADC, y data set in Analyse()
			GraphData.data[0][GraphData.nextIn] = data->peakA;						// x = CHA
			GraphData.data[1][GraphData.nextIn] = data->peakB;						// x = CHB
			GraphData.data[2][GraphData.nextIn] = data->peakC;						// x = CHC
			GraphData.data[3][GraphData.nextIn] = data->peakD;	
			debugStr.Format(_T("Y = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), data->peakA, data->peakB, data->peakC, data->peakD); 		

			debugStr.Format(_T("X = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), 
					GraphData.data[0][GraphData.nextIn], GraphData.data[1][GraphData.nextIn], GraphData.data[2][GraphData.nextIn], GraphData.data[3][GraphData.nextIn]);
			break;
		case GraphData.GRAPH_ADCB_ADCA:
			GraphData.data[0][GraphData.nextIn] = data->integA;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->integB;		// y = CHB
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;
		case GraphData.GRAPH_ADCC_ADCA:
			GraphData.data[0][GraphData.nextIn] = data->integA;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->integC;		// y = CHC
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;
		case GraphData.GRAPH_ADCC_ADCB:
			GraphData.data[0][GraphData.nextIn] = data->integB;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->integC;		// y = CHD
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;		
		case GraphData.GRAPH_PEAKB_PEAKA:
			GraphData.data[0][GraphData.nextIn] = data->peakA;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->peakB;		// y = CHD
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;
		case GraphData.GRAPH_PEAKC_PEAKA:
			GraphData.data[0][GraphData.nextIn] = data->peakA;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->peakC;		// y = CHD
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;
		case GraphData.GRAPH_PEAKC_PEAKB:
			GraphData.data[0][GraphData.nextIn] = data->peakB;							// x = CHA
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->peakC;		// y = CHD
			debugStr.Format(_T("X = %0.2f, Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn], GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
			break;
		case GraphData.GRAPH_INT_T:														// y data from integrators, x data increments
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->integA;		// y = CHA
			GraphData.data[1][GraphData.nextIn + GraphData.nPoints] = data->integB;		// y = CHB
			GraphData.data[2][GraphData.nextIn + GraphData.nPoints] = data->integC;		// y = CHC
			GraphData.data[3][GraphData.nextIn + GraphData.nPoints] = data->integD;		// y = CHD
			GraphData.data[0][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[1][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[2][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[3][GraphData.nextIn] = (double)t;							// x = count
			debugStr.Format(_T("X = %d, Y = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), t, data->integA, data->integB, data->integC, data->integD); 
			break;
		case GraphData.GRAPH_PEAK_T:													// x data from peak, y data increments
			GraphData.data[0][GraphData.nextIn + GraphData.nPoints] = data->peakA;		// y = CHA
			GraphData.data[1][GraphData.nextIn + GraphData.nPoints] = data->peakB;		// y = CHB
			GraphData.data[2][GraphData.nextIn + GraphData.nPoints] = data->peakC;		// y = CHC
			GraphData.data[3][GraphData.nextIn + GraphData.nPoints] = data->peakD;		// y = CHD
			GraphData.data[0][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[1][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[2][GraphData.nextIn] = (double)t;							// x = count
			GraphData.data[3][GraphData.nextIn] = (double)t;							// x = count
			debugStr.Format(_T("Y = %0.2f, %0.2f, %0.2f, %0.2f\r\n"), data->peakA, data->peakB, data->peakC, data->peakD); 
			break;
		default:
			break;
	}
	t++;
	DEBUG_MESSAGE_EXT(debugStr);
}

//-----------------------------------------------------------------------------
void ReadHousekeeping(void) {

	if (!serialRunning)	{						// no comms until it's all been set up
		DEBUG_MESSAGE_EXT(_T("DAQ error - serial not running\r\n"));
		return;
	}

	ReadHumidTemp();

	unsigned int x, y, z;

	sprintf_s(command, COMMAND_LENGTH, "#V%d", 0);		// fill1microinjector
	SendCommand();
	sscanf_s(response, "#V%d=%d,%d\r\n", &x, &y, &z);
	hardware[FILL1microinjector].value.i = (y == 1);

	sprintf_s(command, COMMAND_LENGTH, "#V%d", 1);		// fill2bypassFilter
	SendCommand();
	sscanf_s(response, "#V%d=%d,%d\r\n", &x, &y, &z);
	hardware[FILL2bypassFilter].value.i = (y == 1);

	sprintf_s(command, COMMAND_LENGTH, "#V%d", 2);		// fill3newSheath
	SendCommand();
	sscanf_s(response, "#V%d=%d,%d\r\n", &x, &y, &z);
	hardware[FILL3newSheath].value.i = (y == 1);

	sprintf_s(command, COMMAND_LENGTH, "#V%d", 3);		// isol
	SendCommand();
	sscanf_s(response, "#V%d=%d,%d\r\n", &x, &y, &z);
	hardware[ISOL].value.i = (y == 1);
	hardware[HUMID_AUTO].value.i = (z == 1);				// humid controls isol and spare

	sprintf_s(command, COMMAND_LENGTH, "#V%d", 4);		// spare
	SendCommand();
	sscanf_s(response, "#V%d=%d,%d\r\n", &x, &y, &z);
	hardware[SPARE].value.i = (y == 1);

	sprintf_s(command, COMMAND_LENGTH, "#F");				// fans
	SendCommand();
	sscanf_s(response, "#F=%d,%d\r", &x, &y);
	hardware[FANS].value.i = (x == 1);
//	hardware[FANS_AUTO].value.i = (y == 1);

	sprintf_s(command, COMMAND_LENGTH, "#L");				// laser
	SendCommand();
	sscanf_s(response, "#L=%d\r", &x);
	hardware[LASER].value.i = (x == 1);

	sprintf_s(command, COMMAND_LENGTH, "#C");				// camera
	SendCommand();
	sscanf_s(response, "#C=%d\r", &x);
	hardware[CAMERA].value.i = (x == 1);

	sprintf_s(command, COMMAND_LENGTH, "#S");				// stirrer
	SendCommand();
	sscanf_s(response, "#S=%d\r", &x);
	hardware[STIRRER].value.i = (x == 1);

//	fanThresh = ReadEeprom(20);								// fanThresh
//	humidThresh = ReadEeprom(16);							// humidThresh

	// pumps
	if (pumpDrive) {										// pump drive board
		sprintf_s(command, COMMAND_LENGTH, "$P1");			// pump1
		SendCommand();
		sscanf_s(response, "$P1=%d\r", &x);
		hardware[PUMP1].value.i = (x == 1);

		sprintf_s(command, COMMAND_LENGTH, "$P2");			// pump2
		SendCommand();
		sscanf_s(response, "$P2=%d\r", &x);
		hardware[PUMP2].value.i = (x == 1);

		sprintf_s(command, COMMAND_LENGTH, "$E");			// pump voltage
		SendCommand();
		sscanf_s(response, "$E=%d\r", &x);
		pumpVolts = PumpCountToVolts(x);
	} else {												// HK board
		sprintf_s(command, COMMAND_LENGTH, "#P");			// pump1
		SendCommand();
		sscanf_s(response, "#P=%d\r", &x);
		hardware[PUMP1].value.i = (x == 1);

		sprintf_s(command, COMMAND_LENGTH, "#Q");			// pump2
		SendCommand();
		sscanf_s(response, "#Q=%d\r", &x);
		hardware[PUMP2].value.i = (x == 1);
	}
}

//-----------------------------------------------------------------------------
static void SetDigital(VALUE val, uint16 flag) {

	int state = val.i;

	debugStr = _T("Setting ");
	switch (flag) {
		case FILL1microinjector:
			debugStr += _T("Fill1microinjector valve");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%d", 0, state);		// fill1microinjector
			break;
		case FILL2bypassFilter:
			debugStr += _T("Fill2bypassFilter valve");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%d", 1, state);		// fill2bypassFilter
			break;
		case FILL3newSheath:
			debugStr += _T("Fill3newSheath valve");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%d", 2, state);		// fill3newSheath
			break;
		case ISOL:
			debugStr += _T("Isolation valve");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%d", 3, state);		// isol - OFF, ON or AUTO
			break;
		case SPARE:
			debugStr += _T("Spare valve");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%d", 4, state);		// spare - OFF, ON or AUTO
			break;
		case HUMID_AUTO:
			debugStr += _T("Humidity auto");
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%c", 3, state ? '7' : '6');		// isol - AUTO or MANUAL
			SendCommand();
			sprintf_s(command, COMMAND_LENGTH, "#V%d,%c", 4, state ? '7' : '6');		// spare - AUTO or MANUAL
			break;
		case FANS:
			debugStr += _T("Fans");
			sprintf_s(command, COMMAND_LENGTH, "#F%d", state);				// fans - ON or OFF
			break;
		case LASER:
			debugStr += _T("Laser");
			sprintf_s(command, COMMAND_LENGTH, "#L%d", state);				// laser
			break;
		case CAMERA:
			debugStr += _T("Camera");
			sprintf_s(command, COMMAND_LENGTH, "#C%d", state);				// camera
			break;
		case STIRRER:
			debugStr += _T("Stirrer");
			sprintf_s(command, COMMAND_LENGTH, "#S%d", state);				// stirrer
			break;
		case PUMP1:
			debugStr += _T("Pump1");
			if (pumpDrive)
				sprintf_s(command, COMMAND_LENGTH, "$P1,%d", state);		// pump1 on pump drive board
			else
				sprintf_s(command, COMMAND_LENGTH, "#P%d", state);			// pump1 on HK board
			break;
		case PUMP2:
			debugStr += _T("Pump2");
			if (pumpDrive)
				sprintf_s(command, COMMAND_LENGTH, "$P2,%d", state);		// pump2 on pump drive board
			else
				sprintf_s(command, COMMAND_LENGTH, "#Q%d", state);			// pump2 on HK board
			break;
		case FLASH:
			debugStr += _T("Flashlamp");
			sprintf_s(command, COMMAND_LENGTH, "#N%d", state);				// flashlamp
			break;
	}

	switch (state) {
		case OFF:
			debugStr += _T(" OFF\r\n");
			break;
		case ON:
			debugStr += _T(" ON\r\n");
			break;
		case AUTO:
			debugStr += _T(" AUTO\r\n");
			break;
		default:
			debugStr += _T(" to UNKNOWN\r\n");
			break;
	}
	DEBUG_MESSAGE_EXT(debugStr);

	SendCommand();
}

//-----------------------------------------------------------------------------
static void SetAdcRate(VALUE val, uint16 addr) {

	sprintf_s(command, "*J%d", val.i);
	SendCommand();
}

//-----------------------------------------------------------------------------
static void SetPga(VALUE val, uint16 addr) {

	sprintf_s(command, "*I%d", val.i);
	SendCommand();
//	sscanf_s(response, "*I=%d\r", &pga);
}

//-----------------------------------------------------------------------------
static uint8 PumpVoltsToCount(double volts) {

	if (volts >= 24.0)
		return 255;

	if (volts < 4.5)
		return 27;

	return (uint8)((volts - 1.8) * 10.0);
}

//-----------------------------------------------------------------------------
static double PumpCountToVolts(uint8 count) {

	if (count <= 27)
		return 4.5;

	if (count >= 222)
		return 24.0;

	return (double)count / 10.0 + 1.8;
}

//-----------------------------------------------------------------------------
static void SetPumpVolts(VALUE val, uint16 addr) {

	if (!pumpDrive)
		return;

	debugStr.Format(_T("Setting pump voltage to %f\r\n"), val.d);
	DEBUG_MESSAGE_EXT(debugStr);

	sprintf_s(command, "$E%d", PumpVoltsToCount(val.d));	// pump drive volts
	SendCommand();
}

//-----------------------------------------------------------------------------
static bool SyncComms(BOARD board) {

	char cmd[COMMAND_LENGTH];
	bool ok = false;

	switch (board) {
		case HK:
			sprintf_s(cmd, "#T");
			break;
		case ANALOG:
			sprintf_s(cmd, "*Q");
			break;
		case PUMP:
			sprintf_s(cmd, "$T");
			break;
	}

	strcpy_s(command, COMMAND_LENGTH, cmd);
	SendCommand();
	serialRunning = true;
	strcpy_s(command, COMMAND_LENGTH, cmd);
	ok = SendCommand();
	if (strcmp(command, response))
		ok = false;

	return ok;
}

//-----------------------------------------------------------------------------
static bool OpenPort(void) {

	CString str;
	DWORD nBytes = 0;
	DCB dcbSerialParams = {0};

	serialRunning = false;
	str = daqPort;
	hSerialDaq = CreateFileW(str,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hSerialDaq == INVALID_HANDLE_VALUE)
		return false;

	// set the serial port parameters
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerialDaq, &dcbSerialParams)) {		// error getting state
		CloseHandle(hSerialDaq);							// close serial port
		return false;
	}
	dcbSerialParams.BaudRate = DAQ_BAUD;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fOutxCtsFlow = FALSE;
	dcbSerialParams.fOutxDsrFlow = FALSE;
	dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
	dcbSerialParams.fOutX = FALSE;
	dcbSerialParams.fInX = FALSE;
	dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

	if(!SetCommState(hSerialDaq, &dcbSerialParams)) {		// error setting serial port state
		CloseHandle(hSerialDaq);							// close serial port
		return false;
	}
	
	COMMTIMEOUTS timeouts = {0};

	// set some timeouts - need to be long because the analogue board can be slow to respond
/*	timeouts.ReadIntervalTimeout = 100;						// between characters
	timeouts.ReadTotalTimeoutMultiplier = 100;				// per character...
	timeouts.ReadTotalTimeoutConstant = 5000;				// ... plus this*/
	timeouts.ReadIntervalTimeout = MAXDWORD;				// ReadFile will return immediately
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;

	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (!SetCommTimeouts(hSerialDaq, &timeouts)) {			// timeouts
		CloseHandle(hSerialDaq);							// close serial port
		return false;
	}

	serialRunning = true;
	return true;
}

//-----------------------------------------------------------------------------
static int GetFirmwareVersion(BOARD board) {

	switch (board) {
		case HK:
			command[0] = '#';
			break;
		case ANALOG:
			command[0] = '*';
			break;
		default:
			return 0;
	}

	int version = 0;
	command[1] = 'U';
	command[2] = '\0';
	SendCommand();
	sscanf_s(response + 1, "U%d", &version);

	return version;
}

//-----------------------------------------------------------------------------
void SetHWParams(void) {

	for (int i = 0; i < sizeof(hardware) / sizeof(HARDWARE_PARAM); i++) {
		if (!hardware[i].changed)
			continue;
		(*hardware[i].Set)(hardware[i].value, hardware[i].arg);	// call the setter function
		hardware[i].changed = false;
	}
}

//-----------------------------------------------------------------------------
void ChangeHWParam(int param, uint16 val) {

	hardware[param].value.i = val;
	hardware[param].changed = true;
	SetHWParams();
}

//-----------------------------------------------------------------------------
void ChangeHWParam(int param, int val) {

	hardware[param].value.i = (uint16)val;
	hardware[param].changed = true;
	SetHWParams();
}

//-----------------------------------------------------------------------------
void ChangeHWParam(int param, double val) {

	hardware[param].value.d = val;
	hardware[param].changed = true;
	SetHWParams();
}

//-----------------------------------------------------------------------------
//	Only used when hasCamera == false
//	checks to see if capture character has appeared on serial port
//-----------------------------------------------------------------------------
bool CheckForCapture(void) {

/*	DWORD bytesRed, bytesAvail, bytesLeft, error;
	int buff[10];

	if (!PeekNamedPipe(hSerialDaq, buff, 1, &bytesRed, &bytesAvail, &bytesLeft))
		error = GetLastError();*/

	if (serialBusy)
		return false;

	int byte;
	DWORD bytesRead;

	ReadFile(hSerialDaq, &byte, 1, &bytesRead, NULL);

	return ((bytesRead == 1) && ((uint8)byte == CAPTURED));
}

//-----------------------------------------------------------------------------
void DAQInit(void) {

	// setup serial port
	for (uint8 i = 0; i < 2; i++) {
		if (!OpenPort())									// open port
			return;

		if (SyncComms(ANALOG))								// sync the analogue board comms
			break;
		
		CloseHandle(hSerialDaq);							// close serial port and try again
		Sleep(100);
	}
	if (hSerialDaq == INVALID_HANDLE_VALUE)
		serialRunning = false;
	if (serialRunning) {
		DEBUG_MESSAGE_EXT(_T("DAQ serial port opened successfully\r\n"));
	} else {
		ERROR_MESSAGE_EXT(_T("Failed to open DAQ serial port\r\n"));
	}

	if (!SyncComms(ANALOG)) {								// sync the analogue board comms
		ERROR_MESSAGE_EXT(_T("Failed to communicate with Analog board\r\n"));
		return;
	}
	if (!SyncComms(HK)) {									// sync the housekeeping board comms
		ERROR_MESSAGE_EXT(_T("Failed to communicate with Housekeeping board\r\n"));
		serialRunning = false;
		return;
	}

	// stop acquisition
	sprintf_s(command, "*S0");
	SendCommand();
	SendCommand();
	Sleep(100);

	FlushComms();

	// initialise housekeeping board
	HKFirmwareVersion = GetFirmwareVersion(HK);
	ChangeHWParam(HUMID_THRESH, humidThresh);
	ChangeHWParam(HUMID_HYSTERESIS, humidHysteresis);
	ChangeHWParam(FAN_THRESH, fanThresh);
	ChangeHWParam(FAN_HYSTERESIS, fanHysteresis);
	ChangeHWParam(HUMID_AUTO, OFF);
	ChangeHWParam(VALVE_DELAY, valveDelay);
	ChangeHWParam(FLASH_DAC, flashVoltage);			// flash control voltage NORMAL
	ChangeHWParam(FLASH_DELAY, flashDelay);			// flash delay
	ChangeHWParam(FLASH_LENGTH, flashPulseLength);	// flash pulse
	ChangeHWParam(CAMERA_LENGTH, cameraPulseLength);
	ChangeHWParam(STIRRER, OFF);					// stirrer off
	ChangeHWParam(CAMERA, ON);						// camera on
	ChangeHWParam(LASER, ON);						// laser on
	//ChangeHWParam(FANS, OFF);						// fans off
	ChangeHWParam(FILL1microinjector, OFF);						// valves off
	ChangeHWParam(FILL2bypassFilter, OFF);		// N.O. solenoid -- ON bypasses filter cartridge after flow cell 
	ChangeHWParam(FILL3newSheath, OFF);		// N.C. solenoid -- ON allows new SW for sheath
	ChangeHWParam(ISOL, OFF);
	ChangeHWParam(SPARE, OFF);
	ChangeHWParam(FLASH, ON);						// flashlamp
	ChangeHWParam(PMTA, 0.0);						// PMTs
	ChangeHWParam(PMTB, 0.0);
	ChangeHWParam(PMTC, 0.0);
	IfcbHandle->dacsOn = false;

	// pump driver board
	if (pumpDrive) {
		ChangeHWParam(PUMP1, ON);					// flashlamp
		Sleep(100);
		if (!SyncComms(ANALOG)) return;				// sync the analogue board comms
		if (!SyncComms(HK)) return;					// sync the housekeeping board comms
		if (!SyncComms(PUMP)) return;				// sync the pump board comms

		ChangeHWParam(PUMP_DAC, pumpVolts);
	}
	if (laserState & 1)								// laser power on/off	
		ChangeHWParam(LASER, ON);
	else
		ChangeHWParam(LASER, OFF);

	if (fill3newSheathState & 1)	{							// recirculate sheath or not
		ChangeHWParam(FILL3newSheath, ON); // 
		ChangeHWParam(FILL2bypassFilter, ON); // 
	}
	else {
		ChangeHWParam(FILL3newSheath, OFF);
		ChangeHWParam(FILL2bypassFilter, OFF);
	}

	if (pumpState & 1)								// pump1	
		ChangeHWParam(PUMP1, ON);
	else
		ChangeHWParam(PUMP1, OFF);
	if (pumpState & 2)								// pump2	
		ChangeHWParam(PUMP2, ON);
	else
		ChangeHWParam(PUMP2, OFF);

	// configure analog board
	AnalogFirmwareVersion = GetFirmwareVersion(ANALOG);
	ChangeHWParam(TRIGA, trigThreshA);
	ChangeHWParam(TRIGB, trigThreshB);
	ChangeHWParam(TRIGC, trigThreshC);
	ChangeHWParam(TRIGD, trigThreshD);
	ChangeHWParam(ADC_RATE, adcDrate);
	ChangeHWParam(INTEGRATOR_SETTLE, integratorSettleTime);
	ChangeHWParam(TRIG_MASK, triggerPulseMask);
	ChangeHWParam(PGA, adcPga);
	ChangeHWParam(TRIG_TIMEOUT, triggerTimeout);
	ChangeHWParam(PULSE_STRETCH, pulseStretchTime);
	ChangeHWParam(MAX_PULSE_STRETCH, maxPulseStretch);
	ChangeHWParam(HAS_CAMERA, hasCamera);
}

#endif