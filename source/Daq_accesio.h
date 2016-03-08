//-----------------------------------------------------------------------------
//  IFCB Project
//	accesio.h
//	Martin Cooper
//
//	ACCESIO card-specific details
//-----------------------------------------------------------------------------
#pragma once


#ifdef ACCESIO
#include "config.h"

#define AIO_BASE	0x300
#define AIO_PORTA	AIO_BASE + 0x10
#define AIO_PORTB	AIO_BASE + 0x11

// ADC channels
#define	PMT1_ADC	6
#define PMT2_ADC	7
#define PMT3_ADC	14
#define TRIG_ADC	0
#define TEMP_ADC	4
#define HUMID_ADC	5
#define FLASH_ADC	0
#define SPARE_ADC	0

// DAC channels
#define	PMT1_DAC	0
#define PMT2_DAC	1
#define PMT3_DAC	2
#define TRIG_DAC	0xFF
#define FLASH_DAC	0xFF

// digital IO
#define FILL1			13
#define FILL2			14
#define FILL3			15
#define PUMP1			7
#define PUMP2			8
#define TRIG_INHIBIT	6
#define STIRRER			12

// settings structure
typedef struct {
	double			pmt1_volt; 
	double			pmt2_volt;
	unsigned char	jumpers;
	unsigned char	adrange;
	unsigned char	dacBrange;
	unsigned char	dacArange;
	double			crit_len;
} ACCES_SETTINGS;
extern ACCES_SETTINGS accesSettings;

enum { LO = 0, HI = 1 };

bool	Set_ACCESIO_DigOut(unsigned short pin, int state);
double	Read_ACCESIO_AD(void);
bool	DAQInit(void);
//void	SetPMTHV(unsigned int pmtNum, double pmtHV);
bool	DAQShutdown(void);
void	EnableTriggeredScan(void);
void	DaqArmTrigger(bool armState);
void	ReadDAQIntegrated(void);
void	DaqArmTrigger(bool armState); 
void	GenerateTrigger(void);
bool	Set_ACCESIO_DigOut(unsigned short pin, int state);
//void CtrMode(unsigned int addr, char cntr, char mode);

#endif