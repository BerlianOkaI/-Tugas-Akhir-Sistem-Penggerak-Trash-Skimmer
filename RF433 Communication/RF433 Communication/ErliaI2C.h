/*
 * ErliaI2C.h
 *
 * Created: 12/05/2024 11:12:35
 *  Author: ASUS
 */ 


#ifndef ERLIAI2C_H_
#define ERLIAI2C_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "TypeDef.h"

#ifndef FREQ_CPU
#define FREQ_CPU		16000000L
#endif


/* Status Code Definition */
// Master Transmitter Mode
#define I2C_START_COMP				0x08
#define I2C_RSTART_COMP				0x10
#define I2C_MTSLAW_ACK				0x18
#define I2C_MTSLAW_NACK				0x20
#define I2C_MTDATA_ACK				0x28
#define I2C_MTDATA_NACK				0x30
#define I2C_MTARBL					0x38

// Master Receiver Mode (some has been defined in above definition)
#define I2C_MRARBL					0x38
#define I2C_MRSLAR_ACK				0x40
#define I2C_MRSLAR_NACK				0x48
#define I2C_MRDATA_ACK				0x50
#define I2C_MRDATA_NACK				0x58

// Slave Receiver Mode
#define I2C_SRSLAW_ACK				0x60
#define I2C_SRARBL_SLAW_ACK			0x68
#define I2C_SRGENCALL_ACK			0x70
#define I2C_SRARBL_GENCALL_ACK		0x78
#define I2C_SRPSLAW_DATA_ACK		0x80
#define I2C_SRPSLAW_DATA_NACK		0x88
#define I2C_SRPGENCALL_DATA_ACK		0x90
#define I2C_SRPGENCALL_DATA_NACK	0x98
#define I2C_SR_STOP					0xA0

// Slave Transmitter Mode
#define I2C_STSLAR_ACK				0xA8
#define I2C_STARBL_SLAR_ACK			0xB0
#define I2C_STDATA_ACK				0xB8
#define I2C_STDATA_NACK				0xC0
#define I2C_STLASTDATA_ACK			0xC8

// Miscellaneous state
#define I2C_NORELEVANT				0xF8
#define I2C_ILLEGALSTARTSTOP		0x00

#define I2C_RECEIVEBUFFSIZE			20
#define I2C_TRANSMITBUFFSIZE		20

typedef enum 
{
	MT_MODE,	READY_M_TRANSMIT,
	MR_MODE,	READY_M_RECEIVE,
	ST_MODE,	ST_MODE_GENCALL,
	SR_MODE,	SR_MODE_GENCALL,
	FREE
} I2CMode;

typedef enum
{
	NOLAST, NEXTLAST, LAST	
} MR_DataStatus;

class tagI2C
{
public:
	static void		begin(ULONG ulSCLFreq, UCHAR address);

	static void		commStart(void);
	static void		commStop(void);
	
	// Master Transmitter Mode
	static void		commSLAW(UCHAR address);
	static void		commTrData(UCHAR data);
	static BOOL		MTransmit(char* str, UINT str_size, UCHAR targetAdr);
	static BOOL		print(char* str, UCHAR targetAdr);
	
	// Master Receiver Mode
	static void		commSLAR(UCHAR address);
	static UCHAR	commRecData(MR_DataStatus position);
	static BOOL		MReceiveReq(char* msgBuff, UINT msgSize, UCHAR targetAdr);
	
	// Slave Receiver Mode
	static void		commBeginSlaveReceiving(void);
	static void		slaveMode(UCHAR myAddress, BOOL genCallEn);
	static void		attachSRDataRoutine(void (*SRproc)(UCHAR ucData));
	static BOOL		pullRecData(char* destination);
	static void		flushRecData(void);
	
	// Slave Transmitter Mode
	static void		addSTData(char* str, UINT strSize);
	
	static void		attachExtCase(void (*proc)(UCHAR status));
	static void		I2C_ISR(void);
private:
	static I2CMode	mode;
	static UCHAR	devAddr;
	static UCHAR	status;
	static void		(*ExtProc)(UCHAR status);
	
	// Buffer for receive data
	static PCHAR	pcRecData;
	static UINT		uiRecDataSize;
	static UINT		uiRecIndx;
	
	// pointer to data to be transmitted
	static PCHAR	pcTrData;
	static UINT		uiTrDataIndx;
	static UINT		uiTrDataSize;
	static UCHAR	ucTargetAdr;
	
	// pointer to SR mode receive data routine (ISR)
	static void		(*SRDataRoutine)(UCHAR recData);
	static char		slRecBuff[I2C_RECEIVEBUFFSIZE];
	static UINT		slRecIdx;
	
	// pointer to ST mode transmit data routine (ISR)
	static char		slTrBuff[I2C_TRANSMITBUFFSIZE];
	static UINT		slTrIdx;
};

extern tagI2C I2C;




#endif /* ERLIAI2C_H_ */