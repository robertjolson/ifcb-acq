//-----------------------------------------------------------------------------
//  IFCB Project
//	Daq_niusb6009.cpp
//
//	Handles NIUSB6009 IO interface
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"
#include "Daq.h"

#ifdef NIUSB6009
#include "IfcbDlg.h"
//#include "NIDAQmx.h"

char	daqPort[STRING_PARAM_LENGTH];			// dummy - not used

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static 	TaskHandle	taskHandleDigTrig=0, 
					taskHandlePMTAnalogOut0=0, 
					taskHandlePMTAnalogOut1=0, 
					taskHandleDigArm=0, 
					taskHandleAnalogIn=0;

static CString	debugStr;



//-----------------------------------------------------------------------------
// Dummy functions used only by DAQ_MCC
//-----------------------------------------------------------------------------
//unsigned int GetTrigInhibitTime(void) { return 0; }
//void ResetTrigInhibitTime() {}
//bool SetFlashlamp(bool on) { return on; }
void StartAcquisition(void) {}
void StopAcquisition(void) {}
//void TimeSensitiveConfig(void) {}

//-----------------------------------------------------------------------------
// calls all the different functions to set up various NIDAQmx tasks
//-----------------------------------------------------------------------------
void DAQInit(void) {

	SetupNI6009TriggerTask();
	SetupNI6009ArmTask();
//	SetupNI6009AnalogOutputTask();
	SetupNI6009AnalogOutputTask(0);
	SetupNI6009AnalogOutputTask(1);
	SetupNI6009AnalogInputTask();
}

//-----------------------------------------------------------------------------
// calls all the different functions to shutdown various NIDAQmx tasks
//	important because it helps keep lines from being high, etc., after program ends
//-----------------------------------------------------------------------------
bool DAQShutdown(void) {


	DAQmxStopTask(taskHandlePMTAnalogOut0);
	DAQmxClearTask(taskHandlePMTAnalogOut0);
	DAQmxStopTask(taskHandlePMTAnalogOut1);
	DAQmxClearTask(taskHandlePMTAnalogOut1);

	DAQmxStopTask(taskHandleAnalogIn);
	DAQmxClearTask(taskHandleAnalogIn);


	return true;
}




//-----------------------------------------------------------------------------
// sets up the analog input NIDAQmx task for reading the analog integrators
//-----------------------------------------------------------------------------
void SetupNI6009AnalogInputTask(void) {
	int32       error=0;
//	TaskHandle  taskHandle=0;
//	int32       read;
//	float64     data[1000];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandleAnalogIn));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleAnalogIn,"Dev1/ai0:3","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));

	// need to set sample clock other than "on demand"
	// you can apparently clock samples up to 48e3 samps per sec with this guy, per channel
// can crank this up to 1e5 samples but then it gets wacky in how it reads out, later
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandleAnalogIn,"OnboardClock",12e3,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2));

	// set the start trigger as a digital input on PFI0, rising edge (from analog board "enable" on TP11)
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(taskHandleAnalogIn, "PFI0", DAQmx_Val_Falling));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandleAnalogIn));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
//	DAQmxErrChk (DAQmxReadAnalogF64(taskHandleAnalogIn,1000,10.0,DAQmx_Val_GroupByChannel,data,1000,&read,NULL));



Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleAnalogIn!=0 )  {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleAnalogIn);
//		DAQmxClearTask(taskHandleAnalogIn);
	}
	if( DAQmxFailed(error) )
	{
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - initialized taskHandleAnalogIn\r\n"));

	return;

}







//-----------------------------------------------------------------------------
// sets up the analog output NIDAQmx task for controlling the PMTs
//-----------------------------------------------------------------------------
void SetupNI6009AnalogOutputTask(int DACchannel) {

	int			error=0;
	TaskHandle	taskHandle=0;
	char		errBuff[2048]={'\0'};
	float64     data[1] = {0.0};


	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	if (DACchannel == 0)
	{
		DAQmxErrChk (DAQmxCreateTask("",&taskHandlePMTAnalogOut0));
		DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandlePMTAnalogOut0,"Dev1/ao0","",0.0,5.0,DAQmx_Val_Volts,""));
		taskHandle = taskHandlePMTAnalogOut0;
	}
	if (DACchannel == 1)
	{
		DAQmxErrChk (DAQmxCreateTask("",&taskHandlePMTAnalogOut1));
		DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandlePMTAnalogOut1,"Dev1/ao1","",0.0,5.0,DAQmx_Val_Volts,""));
		taskHandle = taskHandlePMTAnalogOut1;
	}
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandlePMTAnalogOut);
//		DAQmxClearTask(taskHandlePMTAnalogOut);
	}
	if( DAQmxFailed(error) )
	{
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - initialized taskHandle for analog output\r\n"));

	return;
}





//-----------------------------------------------------------------------------
// sets up the digital output NIDAQmx task for controlling the manual trigger
//-----------------------------------------------------------------------------
void SetupNI6009TriggerTask(void) {
	int         error=0;
//	TaskHandle	taskHandle=0;
	uInt32      data=0x0;
	char        errBuff[2048]={'\0'};
	int32		written;


	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandleDigTrig));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandleDigTrig,"Dev1/port0/line1","",DAQmx_Val_ChanForAllLines));



	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandleDigTrig));

	// write all zeros to this port
	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandleDigTrig,1,1,10.0,DAQmx_Val_GroupByChannel,&data,&written,NULL));


Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleDigTrig!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleDigTrig);
//		DAQmxClearTask(taskHandleDigTrig);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - initialized taskHandleDigTrig\r\n"));

	return;
}






//-----------------------------------------------------------------------------
// sets up the digital output NIDAQmx task for controlling the trigger reset pulse
//-----------------------------------------------------------------------------
void SetupNI6009ArmTask(void) {
	int         error=0;
//	TaskHandle	taskHandle=0;
	uInt32      data=0xffffffff;

	char        errBuff[2048]={'\0'};
	int32		written;


	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandleDigArm));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandleDigArm,"Dev1/port0/line0","",DAQmx_Val_ChanForAllLines));



	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandleDigArm));

	// write all ones to this port
	// needs to be active LO
	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandleDigArm,1,1,10.0,DAQmx_Val_GroupByChannel,&data,&written,NULL));


Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleDigArm!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleDigTrig);
//		DAQmxClearTask(taskHandleDigTrig);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - initialized taskHandleDigArm\r\n"));

	return;
}






//-----------------------------------------------------------------------------
// a low pulse on the single digital line out that resets the PIC to listen to triggers
//	called from Process.cpp, AcqInit()
//-----------------------------------------------------------------------------
void DaqArmTrigger(void) {

	uInt32      data1[3];
	int         error=0;
//	TaskHandle	taskHandle=0;
	char        errBuff[2048]={'\0'};
	int32		written;


	// very crude way of generating a pulse
data1[0] = 0xffffffff;
data1[1] = 0x0;
data1[2] = 0xffffffff;
// this makes a low-going pulse


	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandleDigArm,3,1,10.0,DAQmx_Val_GroupByChannel,&data1[0],&written,NULL));
	

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleDigArm!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleDigTrig);
//		DAQmxClearTask(taskHandleDigTrig);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - rearmed trigger\r\n"));

}











//-----------------------------------------------------------------------------
// sends a pulse on a digital line that can be piped into the camera to trigger it
// currently piped into the RB0 line of a PIC controller that oversees timing
//-----------------------------------------------------------------------------
void GenerateTrigger(void) {

	uInt32      data1[3];
	int         error=0;
//	TaskHandle	taskHandle=0;
	char        errBuff[2048]={'\0'};
	int32		written;

	// very crude way of generating a pulse
data1[0] = 0x0;
data1[1] = 0xffffffff;
data1[2] = 0x0;
// pulse on port 0 line 1
// this makes a high-going pulse

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandleDigTrig,3,1,10.0,DAQmx_Val_GroupByChannel,&data1[0],&written,NULL));
	

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleDigTrig!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleDigTrig);
//		DAQmxClearTask(taskHandleDigTrig);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - fired Trigger\r\n"));

}






//-----------------------------------------------------------------------------
// performs all the low-level operations to grab analog data from NI USB card
//	called from Process.cpp, ProcessTrigger()
//-----------------------------------------------------------------------------
void ReadDAQIntegrated(DataStruct *data) {

	int         error=0;
	int32       read;
	float64     value[4];
	char        errBuff[2048]={'\0'};
	int i;
	CString str;

	DEBUG_MESSAGE_EXT(_T(" - in ReadDAQIntegrated\r\n"));


	for (i = 0; i < 4; i++) value[i] = 0.0;

	// by this time the PFI0 line will have triggered the analog read. Can check this?

//DAQmxWaitUntilTaskDone (TaskHandle taskHandle, float64 timeToWait);

	// timeout = 0 makes the thing try once, then quit if unsuccessful
	// sometimes it fails to read data if you act too fast
	// wait for 50 ms max
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandleAnalogIn,1,0.05,DAQmx_Val_GroupByChannel,value,4,&read,NULL));
	for (i = 0; i < 4; i++) {
		str.Format(_T("chan %d: %f\r\n"), i, value[i]);
		DEBUG_MESSAGE_EXT(str);
	}
//	str.Format(_T("samps per chan read: %d\r\n"), read);
//	DEBUG_MESSAGE_EXT(str);


// make sure these are the right ones
	data->integAlo = (float)value[0];
	data->integAhi = (float)value[1];
	data->integClo = (float)value[2];
	data->integChi = (float)value[3];



Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleAnalogIn!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleAnalogIn);
//		DAQmxClearTask(taskHandleAnalogIn);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}


	// need to stop & restart task when triggering from PFI0
	DAQmxErrChk (DAQmxStopTask(taskHandleAnalogIn));		
	DAQmxErrChk (DAQmxStartTask(taskHandleAnalogIn));


	DEBUG_MESSAGE_EXT(_T(" - read ADC channels\r\n"));


	return;
}






//-----------------------------------------------------------------------------
void SetDac(uint8 DACnum, double voltage) {

	if (DACnum == PMTC_DAC)				// avoid #ifdefs all over the code
		return;

	float64     data[1] = {0.0};
	int error=0;
	char        errBuff[2048]={'\0'};
	CString str; 


	str.Format(_T(" - in SetDAC: dacnum %d voltage %f\r\n"), DACnum, voltage);
	DEBUG_MESSAGE_EXT(str);

	if (DACnum != PMTA_DAC && DACnum != PMTB_DAC)
	{
		str.Format(_T(" - in SetDAC: requesting bad dacnum %d \r\n"), DACnum);
		DEBUG_MESSAGE_EXT(str);
		return;
	}


	// check to make sure the voltage isn't > DAC_OUTPUT_MAX or less than 0
	if (voltage > DAC_OUTPUT_MAX|| voltage < 0)
	{
		str.Format(_T(" - in SetDAC: requesting bad voltage %f \r\n"), voltage);
		DEBUG_MESSAGE_EXT(str);
		return;
	}

	data[0] = (float64)(voltage);

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	if (DACnum == PMTA_DAC)
		DAQmxErrChk (DAQmxWriteAnalogF64(taskHandlePMTAnalogOut0,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));

	if (DACnum == PMTB_DAC)
		DAQmxErrChk (DAQmxWriteAnalogF64(taskHandlePMTAnalogOut1,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));

//	if (DACnum == PMTB_DAC)
//		DAQmxErrChk (DAQmxWriteAnalogF64(taskHandlePMTAnalogOut,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));


Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);

//	if( taskHandlePMTAnalogOut!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandlePMTAnalogOut);
//		DAQmxClearTask(taskHandlePMTAnalogOut);
//	}
	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}
	
	return;	
}







//-----------------------------------------------------------------------------
void ReadHousekeeping(void) {
// SRL: i would like to just call this function but it's got the embedded write-to-data array inside
//	ReadDAQIntegrated();	
	

	// so have to duplicate the actual code, here
	int         error=0;
	int32       read;
	float64     value[4];
	char        errBuff[2048]={'\0'};
	int i;
	CString str;

	DEBUG_MESSAGE_EXT(_T(" - in ReadHouskeeping\r\n"));


	for (i = 0; i < 4; i++) value[i] = 0.0;

	// timeout = 0 makes the thing try once, then quit if unsuccessful
	// timeout of 1 waits one second
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandleAnalogIn,1,1.0,DAQmx_Val_GroupByChannel,value,4,&read,NULL));
	for (i = 0; i < 4; i++) {
		str.Format(_T("chan %d: %f\r\n"), i, value[i]);
		DEBUG_MESSAGE_EXT(str);
	}
//	str.Format(_T("samps per chan read: %d\r\n"), read);
//	DEBUG_MESSAGE_EXT(str);





Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleAnalogIn!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
//		DAQmxStopTask(taskHandleAnalogIn);
//		DAQmxClearTask(taskHandleAnalogIn);
	}

	if( DAQmxFailed(error) ) {
		ERROR_MESSAGE_EXT(_T("DAQmx Error:\r\n"));
		ERROR_MESSAGE_EXT(CString(errBuff));
		ERROR_MESSAGE_EXT(_T("\r\n"));
	}

	DEBUG_MESSAGE_EXT(_T(" - read ADC channels\r\n"));


	return;

	
	
	
// from the accesio version	
/*
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
	Flags[FILL1microinjector_F] = Read_ACCESIO_DigOut(FILL1microinjector);
	Flags[FILL2bypassFilter_F] = Read_ACCESIO_DigOut(FILL2bypassFilter);
	Flags[FILL3newSheath_F] = Read_ACCESIO_DigOut(FILL3newSheath);
	Flags[STIRRER_F] = Read_ACCESIO_DigOut(STIRRER);
	Flags[PUMP1_F] = Read_ACCESIO_DigOut(PUMP1);
	Flags[PUMP2_F] = Read_ACCESIO_DigOut(PUMP2);

*/	
}




//-----------------------------------------------------------------------------
void SetDigital(uint8 flag, uint8 state) {
}




#endif
