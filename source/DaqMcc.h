//-----------------------------------------------------------------------------
//  IFCB Project
//	DaqMcc.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once

#ifdef DAQ_MCC

#define CAPTURED		0x19		// sent by Analog board after trigger when operating without camera

#define AIO_BASE	0x300
#define AIO_PORTA	AIO_BASE + 0x10
#define AIO_PORTB	AIO_BASE + 0x11
enum { LO = 0, HI = 1 };

// DAC channels
#define	PMTA_DAC	2		// hk board
#define PMTB_DAC	1
#define PMTC_DAC	0
#define DAC_FLASH	5
#define TRIGA_DAC	6		// analog board
#define TRIGB_DAC	7
#define TRIGC_DAC	8
#define TRIGD_DAC	9

// ADC channels
#define HUMID_ADC	0
#define TEMP_ADC	1

#define COMMAND_LENGTH		30				// command string length
#define RESPONSE_LENGTH		200				// response string length
#define SERIAL_TIMEOUT		50				// ms
#define DAQ_BAUD			CBR_115200

typedef enum {HK, ANALOG, PUMP} BOARD;

typedef union {
	double		d;
	uint16		i;
} VALUE;

typedef struct {
	void			(*Set)(VALUE val, uint16 arg);		// setter function
	VALUE			value;
	bool			changed;
	uint16			arg;								// not always required
} HARDWARE_PARAM;

typedef enum {
	PMTA,
	PMTB,
	PMTC,
	PUMP_DAC,
	TRIGA,
	TRIGB,
	TRIGC,
	TRIGD,
	ADC_RATE,
	INTEGRATOR_SETTLE,
	TRIG_MASK,
	PGA,
	FAN_THRESH,
	FAN_HYSTERESIS,
	HUMID_THRESH,
	HUMID_HYSTERESIS,
	HUMID_AUTO,
	VALVE_DELAY,
	FLASH,
	FLASH_DAC,
	FLASH_DELAY,
	FLASH_LENGTH,
	CAMERA_LENGTH,
	TRIG_TIMEOUT,
	PULSE_STRETCH,
	MAX_PULSE_STRETCH,
	HAS_CAMERA,
	FILL1microinjector,
	FILL2bypassFilter,
	FILL3newSheath,
	ISOL,
	SPARE,
	FANS,
	STIRRER,
	LASER,
	CAMERA,
	PUMP1,
	PUMP2
} HW_PARAMS;

extern HARDWARE_PARAM hardware[];

void		ReadHumidTemp(void);
bool		CheckHumidity(void);
void		SetHWParams(void);
void		ChangeHWParam(int param, uint16 val);
void		ChangeHWParam(int param, int val);
void		ChangeHWParam(int param, double val);

#endif
