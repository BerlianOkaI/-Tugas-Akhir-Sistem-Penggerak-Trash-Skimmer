/*
 * ErliaI2C.cpp
 *
 * Created: 12/05/2024 12:04:10
 *  Author: ASUS
 */ 

#include "ErliaI2C.h"

UCHAR		tagI2C::devAddr						= 0x00;
UCHAR		tagI2C::status						= 0x00;
I2CMode		tagI2C::mode						= FREE;
void		(*tagI2C::ExtProc)(UCHAR status)	= 0x00;

PCHAR		tagI2C::pcRecData					= 0x00;
UINT		tagI2C::uiRecDataSize				= 0;
UINT		tagI2C::uiRecIndx					= 0;

PCHAR		tagI2C::pcTrData					= 0x00;
UINT		tagI2C::uiTrDataIndx				= 0;
UCHAR		tagI2C::ucTargetAdr					= 0;
UINT		tagI2C::uiTrDataSize				= 0;

void		(*tagI2C::SRDataRoutine)(UCHAR recData) = 0x00;
char		tagI2C::slRecBuff[I2C_RECEIVEBUFFSIZE];
UINT		tagI2C::slRecIdx					= 0;

char		tagI2C::slTrBuff[I2C_TRANSMITBUFFSIZE];
UINT		tagI2C::slTrIdx						= 0;

void tagI2C::begin(ULONG uiSCLFreq, UCHAR address)
{
	// Calculate the frequency of SCL
	TWSR = (TWSR & 0xFC);
	unsigned long dummyReg;
	dummyReg = ((FREQ_CPU / (2 * (unsigned long)uiSCLFreq)) - 8);
	if (dummyReg > 255)
	{
		// Prescale one time
		TWSR = (TWSR & 0xFC) | 1;
		dummyReg = dummyReg / 4;
		if (dummyReg > 255)
		{
			// Prescale two time
			TWSR = (TWSR & 0xFC) | 2;
			dummyReg = dummyReg / 4;
			if (dummyReg > 255)
			{
				// Prescale three time (last prescale)
				TWSR = (TWSR & 0xFC) | 3;
				dummyReg = dummyReg / 4;
			}
		}
	}
	// In here we have calculated the TWBR
	TWBR = (unsigned char)dummyReg;
	// Set address of the device and enable general call
	TWAR = (address << 1) | _b(TWGCE);
	tagI2C::devAddr = address;
	// Enable interrupt, Enable ACK bit, and Enable I2C Interface
	TWCR = _b(TWEA) | _b(TWIE) | _b(TWEN);
}

void tagI2C::commStart(void)
{
	// Send start condition to initiate I2C communication
	TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWSTA) | _b(TWEN);
}

void tagI2C::commStop(void)
{
	// Change mode into free
	tagI2C::mode = FREE;
	// Send stop condition to end I2C communication
	TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWSTO) | _b(TWEN);
}

void tagI2C::commSLAW(UCHAR address)
{
	// Set address into register so that it can be transmitted
	TWDR = (address << 1) | 0x00;
	// change mode into MT_MODE
	tagI2C::mode = MT_MODE;
	// Initiate transmitting address
	TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWEN);
}
void tagI2C::commTrData(UCHAR data)
{
	// Write data into TWDR first before TWINT goes low
	TWDR = data;
	// Initiate transmitting data
	TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWEN);
}

BOOL tagI2C::MTransmit(char* str, UINT str_size, UCHAR targetAdr)
{
	if (tagI2C::mode == FREE)
	{
		// Set the address of data to be transmitted
		// Reset Data Buffer Index and Initiate other parameter
		tagI2C::pcTrData = str;
		tagI2C::uiTrDataIndx = 0;
		tagI2C::ucTargetAdr = targetAdr;
		tagI2C::uiTrDataSize = str_size;
		// Change mode to READY_M_TRANSMIT
		tagI2C::mode = READY_M_TRANSMIT;
		// Initiating transmitting data by sending start condition
		tagI2C::commStart();
		
		return TRUE;
	} else {
		// Return FALSE to indicate that the I2C device is busy
		return FALSE;
	}
}

BOOL tagI2C::print(char* str, UCHAR targetAdr)
{
	UINT sizeBuff;
	sizeBuff = 0;
	while (str[sizeBuff] != 0)
	{
		sizeBuff ++;
	}
	return tagI2C::MTransmit(str, sizeBuff, targetAdr);
}

void tagI2C::commSLAR(UCHAR address)
{
	// Set address into register so that it can be transmitted
	TWDR = (address << 1) | 0x01;
	// change mode into MR_MODE
	tagI2C::mode = MR_MODE;
	// Initiate transmitting address
	TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWEN);
}

UCHAR tagI2C::commRecData(MR_DataStatus position)
{	
	// Reading data from selected slave
	UCHAR received_data;
	received_data = TWDR;
	if (position == NEXTLAST)
	{
		// next data will be the last that master want to read
		// Therefore, send NACK to the slave for next data reception
		TWCR = (TWCR & 0x0B) | _b(TWINT) | _b(TWEN);
	} 
	else if (position == NOLAST){
		// Send ACK to the slave to continue to the next byte
		TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWEN) | _b(TWEA);
	} 
	else if (position == LAST) {
		// Send stop condition, but reenable TWEA
		TWCR = (TWCR & 0x0B) | _b(TWINT) | _b(TWEN) | _b(TWEA) | _b(TWSTO);
	}
	return received_data;
}

BOOL tagI2C::MReceiveReq(char* msgBuff, UINT msgSize, UCHAR targetAdr)
{
	// Requesting to read data from spesific slave address
	// The data then is buffered into msgBuff pointer that user has created
	if (tagI2C::mode == FREE)
	{
		// Initiating parameter and buffer
		tagI2C::ucTargetAdr = targetAdr;
		tagI2C::uiRecDataSize = msgSize;
		tagI2C::pcRecData = msgBuff;
		tagI2C::uiRecIndx = 0;
		// Change mode into ready to start MR and initiate to start receiving
		// by seizing control over bus
		tagI2C::mode = READY_M_RECEIVE;
		tagI2C::commStart();
		return TRUE;
	} else {
		// Return FALSE to indicate that the I2C device is busy
		return FALSE;
	}
}
void tagI2C::slaveMode(UCHAR myAddress, BOOL genCallEn)
{
	tagI2C::devAddr = myAddress;
	TWAR = (tagI2C::devAddr << 1);
	if (genCallEn)
	{
		TWAR = TWAR | 0x01;
	}
	TWCR = (TWCR & 0x8B) | _b(TWEA) | _b(TWEN);
}
void tagI2C::commBeginSlaveReceiving(void)
{
	// Begining to listen data from master and enable ACK
	TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA);
}

void tagI2C::attachSRDataRoutine(void (*SRproc)(UCHAR ucData))
{
	tagI2C::SRDataRoutine = SRproc;
}

BOOL tagI2C::pullRecData(char* destination)
{
	// Pulling data from buffer. First In First Out (FIFO)
	if (tagI2C::slRecIdx)
	{
		// There is data in buffer
		*destination = tagI2C::slRecBuff[0];
		// shift all data
		for(UCHAR iter=0; iter<tagI2C::slRecIdx-1; iter++)
		{
			tagI2C::slRecBuff[iter] = tagI2C::slRecBuff[iter+1];
		}
		tagI2C::slRecBuff[tagI2C::slRecIdx-1] = 0;
		tagI2C::slRecIdx = tagI2C::slRecIdx - 1;
		// Return TRUE to indicate that data has been pulled
		return TRUE;
	} else {
		// Return FALSE to indicate that data is empty already
		return FALSE;
	}
}

void tagI2C::flushRecData(void)
{
	tagI2C::slRecIdx = 0;
	for (UCHAR iter=0; iter < I2C_RECEIVEBUFFSIZE; iter++)
	{
		tagI2C::slRecBuff[iter] = 0;
	}
}
void tagI2C::addSTData(char* str, UINT strSize)
{
	tagI2C::slTrIdx = strSize;
	if (tagI2C::slTrIdx > I2C_TRANSMITBUFFSIZE)
	{
		tagI2C::slTrIdx = I2C_TRANSMITBUFFSIZE;
	} 
	snprintf(
		tagI2C::slTrBuff,
		I2C_TRANSMITBUFFSIZE,
		"%s", str
	);
}

void tagI2C::attachExtCase(void (*proc)(UCHAR status))
{
	tagI2C::ExtProc = proc;
}

void tagI2C::I2C_ISR(void) 
{	// UNFINISHED
	tagI2C::status = (TWSR & 0xF8);
	switch(tagI2C::status)
	{
	case I2C_START_COMP:
		switch(tagI2C::mode)
		{	// UNFINISHED
		case READY_M_TRANSMIT:
			tagI2C::mode = MT_MODE;
			tagI2C::commSLAW(tagI2C::ucTargetAdr);
			break;
		case READY_M_RECEIVE:
			tagI2C::mode = MR_MODE;
			tagI2C::commSLAR(tagI2C::ucTargetAdr);
			break;
		default :
			// Just send SLA+R with address of 0x00 before end transmission
			tagI2C::mode = FREE;
			tagI2C::commSLAR(0x00);
			break;
		}
		break;
	case I2C_RSTART_COMP:
		switch(tagI2C::mode)
		{	// UNFINISHED
		case READY_M_TRANSMIT:
			tagI2C::mode = MT_MODE;
			tagI2C::commSLAW(tagI2C::ucTargetAdr);
			break;
		case READY_M_RECEIVE:
			tagI2C::mode = MR_MODE;
			tagI2C::commSLAR(tagI2C::ucTargetAdr);
			break;
		default :
			// Just send SLA+R with address of 0x00 before end transmission
			tagI2C::mode = FREE;
			tagI2C::commSLAR(0x00);
			break;
		}
		break;
	case I2C_MTSLAW_ACK:
		// In here, we assume the device only in MT mode
		if (tagI2C::uiTrDataIndx < tagI2C::uiTrDataSize)
		{
			tagI2C::commTrData(tagI2C::pcTrData[tagI2C::uiTrDataIndx]);
			tagI2C::uiTrDataIndx ++;
		}
		break;
	case I2C_MTSLAW_NACK:
		// In here, we the slave does not acknowledge for us to transmit data
		// stop transmit process
		tagI2C::pcTrData = 0x00;
		tagI2C::uiTrDataIndx = 0;
		tagI2C::uiTrDataSize = 0;
		tagI2C::ucTargetAdr = 0x00;
		tagI2C::mode = FREE;
		tagI2C::commStop();
		break;
	case I2C_MTDATA_ACK:
		// In here, previous byte has successfully transmitted and been acknowledged
		// by the slave
		// Proceed into next byte
		if (tagI2C::uiTrDataIndx < tagI2C::uiTrDataSize)
		{
			tagI2C::commTrData(tagI2C::pcTrData[tagI2C::uiTrDataIndx]);
			tagI2C::uiTrDataIndx ++;
		} else {
			// All data byte has been sended
			// Proceed to end transmission
			tagI2C::pcTrData = 0x00;
			tagI2C::uiTrDataIndx = 0;
			tagI2C::uiTrDataSize = 0;
			tagI2C::ucTargetAdr = 0x00;
			tagI2C::mode = FREE;
			tagI2C::commStop();
		}
		break;
	case I2C_MTDATA_NACK:
		// In here, previous data has been received but the slave send NACK
		// It also possible that the slave lost connection to I2C bus
		// Proceed to end transmission process
		tagI2C::pcTrData = 0x00;
		tagI2C::uiTrDataIndx = 0;
		tagI2C::uiTrDataSize = 0;
		tagI2C::ucTargetAdr = 0x00;
		tagI2C::mode = FREE;
		tagI2C::commStop();
		break;
	case I2C_MTARBL:
		// In here, device lost control over the bus
		// Just try to transmit again when the bus is free (for MT Mode)
		if (tagI2C::mode == MT_MODE)
		{
			tagI2C::uiTrDataIndx = 0;
			tagI2C::mode = READY_M_TRANSMIT;
			TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWSTA) | _b(TWEN);	
		} 
		// Or Just try to Read again when the bus is free (for MT Mode)
		else if (tagI2C::mode == MR_MODE)
		{
			tagI2C::uiRecIndx = 0;
			tagI2C::mode = READY_M_RECEIVE;
			TWCR = (TWCR & 0x4B) | _b(TWINT) | _b(TWSTA) | _b(TWEN);
		}
		// Make sure the TWINT is cleared
		if (TWCR & _b(TWINT))
		{
			// Clear the TWINT bit by writing 1 to it
			TWCR = TWCR | _b(TWINT);
		}
		break;
	case I2C_MRSLAR_ACK:	
		// EVALUATION IS NEED FOR tagI2C::mode == FREE
		// Requesting reading data from Slave has been acknowledged
		// Proceed to receive data and don't forget to acknowledge it aftermath
		if (tagI2C::mode == MR_MODE)
		{
			TWCR = (TWCR & 0x0B) | _b(TWEA) | _b(TWINT) | _b(TWEN);
		} else if (tagI2C::mode == FREE) {
			tagI2C::commStop();
		}
		if (TWCR & _b(TWINT))
		{
			// Clear the TWINT bit by writing 1 to it
			TWCR = TWCR | _b(TWINT);
		}
		
		break;
	case I2C_MRSLAR_NACK:
		// Requesting reading data from Slave has been not acknowledged
		// Reset all parameter of receiving process
		tagI2C::pcRecData = 0x00;
		tagI2C::uiRecDataSize = 0;
		tagI2C::uiRecIndx = 0;
		tagI2C::ucTargetAdr = 0x00;
		tagI2C::mode = FREE;
		// Stop requesting reading
		tagI2C::commStop();
		break;
	case I2C_MRDATA_ACK :
		if (tagI2C::uiRecIndx < tagI2C::uiRecDataSize - 2)
		{
			// Read the data byte
			tagI2C::pcRecData[tagI2C::uiRecIndx] = tagI2C::commRecData(NOLAST);
			tagI2C::uiRecIndx ++;
		} 
		else if (tagI2C::uiRecIndx == tagI2C::uiRecDataSize - 2) {
			// Read the data byte and send NACK for next received data
			tagI2C::pcRecData[tagI2C::uiRecIndx] = tagI2C::commRecData(NEXTLAST);
			tagI2C::uiRecIndx ++;
		} else {
			// This is last data. However, this condition never met because for last
			// data, I2C_MRDATA_NACK will be received instead
			tagI2C::pcRecData[tagI2C::uiRecIndx] = tagI2C::commRecData(LAST);
		}
		break;
	case I2C_MRDATA_NACK:
		// In this case, data has been received but master returned NACK
		// It indicates that the receiving process is finished.
		tagI2C::mode = FREE;
		tagI2C::pcRecData = 0x00;
		tagI2C::uiRecDataSize = 0;
		tagI2C::uiRecIndx = 0;
		tagI2C::ucTargetAdr = 0x00;
		tagI2C::commRecData(LAST);
		break;
	case I2C_SRSLAW_ACK:
		// Proceed to receive data and go to SR_MODE
		tagI2C::mode = SR_MODE;
		tagI2C::commBeginSlaveReceiving();
		break;
	case I2C_SRARBL_SLAW_ACK:
		// Losing in seizing control of the bus and addressed as slave instead
		// Proceed to SR mode
		tagI2C::mode = SR_MODE;
		tagI2C::commBeginSlaveReceiving();
		break;
	case I2C_SRGENCALL_ACK:
		// Proceed to SR_MODE_GENCALL and only updating slRecBuff
		tagI2C::mode = SR_MODE_GENCALL;
		tagI2C::commBeginSlaveReceiving();
		break;
	case I2C_SRARBL_GENCALL_ACK:
		// Losing in seizing control over bus but receive general call from winning master
		// Proceed to SR_MODE_GENCALL
		tagI2C::mode = SR_MODE_GENCALL;
		tagI2C::commBeginSlaveReceiving();
		break;
	case I2C_SRPSLAW_DATA_ACK:
		// In this state, device has been addressed before, data has been received, and
		// device has sent ACK to master
		// *Never send NACK while in SR mode
		
		// Read the data
		if (tagI2C::slRecIdx < I2C_RECEIVEBUFFSIZE)
		{
			tagI2C::slRecBuff[tagI2C::slRecIdx] = TWDR;
			tagI2C::slRecIdx ++;
		} else {
			tagI2C::slRecIdx = I2C_RECEIVEBUFFSIZE;
		}
		
		if (tagI2C::SRDataRoutine)
		{
			tagI2C::SRDataRoutine(TWDR);
		}
		// Proceed to receive next byte and send ACK for next byte
		TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_SRPSLAW_DATA_NACK:
		// Data has been returned but we (slave) send NACK instead
		if (tagI2C::slRecIdx < I2C_RECEIVEBUFFSIZE)
		{
			tagI2C::slRecBuff[tagI2C::slRecIdx] = TWDR;
			tagI2C::slRecIdx ++;
		} else {
			tagI2C::slRecIdx = I2C_RECEIVEBUFFSIZE;
		}
		if (tagI2C::SRDataRoutine)
		{
			tagI2C::SRDataRoutine(TWDR);
		}
		// Proceed to the not addressed mode but own address still can be recognize
		tagI2C::mode = FREE;
		TWCR = (TWCR & 0x0F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_SRPGENCALL_DATA_ACK:
		// Data from general call has been received and we acknowledge it
		if (tagI2C::slRecIdx < I2C_RECEIVEBUFFSIZE)
		{
			tagI2C::slRecBuff[tagI2C::slRecIdx] = TWDR;
			tagI2C::slRecIdx ++;
		} else {
			tagI2C::slRecIdx = I2C_RECEIVEBUFFSIZE;
		}
		// Proceed to receive next byte and send ACK for next byte
		TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_SRPGENCALL_DATA_NACK:
		// Data from general call has been received and we don't acknowledge it
		if (tagI2C::slRecIdx < I2C_RECEIVEBUFFSIZE)
		{
			tagI2C::slRecBuff[tagI2C::slRecIdx] = TWDR;
			tagI2C::slRecIdx ++;
		} else {
			tagI2C::slRecIdx = I2C_RECEIVEBUFFSIZE;
		}
		// Proceed to the not addressed mode but own address still can be recognize
		tagI2C::mode = FREE;
		TWCR = (TWCR & 0x0F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_SR_STOP:
		// Repeated start or stop condition has been received while still addressed as slave
		// Proceed to the not addressed mode but own address still can be recognize
		tagI2C::mode = FREE;
		TWCR = (TWCR & 0x0F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_STSLAR_ACK:
		// Our device has been addressed as slave transmitter and we acknowledged it
		// Proceed to ST Mode
		tagI2C::mode = ST_MODE;
		// Load data to be transmitted
		if (tagI2C::slTrIdx > 1)
		{
			TWDR = tagI2C::slTrBuff[0];
			for (UCHAR iter=0; iter < tagI2C::slTrIdx-1; iter++)
			{
				tagI2C::slTrBuff[iter] = tagI2C::slTrBuff[iter+1]; 
			}
			tagI2C::slTrBuff[tagI2C::slTrIdx-1] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			
			// Start transmitting
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		} else if (tagI2C::slTrIdx == 1) {
			TWDR = tagI2C::slTrBuff[0];
			tagI2C::slTrBuff[0] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			// Start transmitting and NACK should be received
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		} else {
			// In here all data has been transmitted
			TWDR = 0;
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		}
		break;
	case I2C_STARBL_SLAR_ACK:
		// Our device has been addressed as slave transmitter and we acknowledged it
		// although our device has lost on seizing control over the bus
		// Proceed to ST Mode
		tagI2C::mode = ST_MODE;
		// Load data to be transmitted
		if (tagI2C::slTrIdx > 1)
		{
			TWDR = tagI2C::slTrBuff[0];
			for (UCHAR iter=0; iter < tagI2C::slTrIdx-1; iter++)
			{
				tagI2C::slTrBuff[iter] = tagI2C::slTrBuff[iter+1];
			}
			tagI2C::slTrBuff[tagI2C::slTrIdx-1] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			
			// Start transmitting
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		} else if (tagI2C::slTrIdx == 1) {
			TWDR = tagI2C::slTrBuff[0];
			tagI2C::slTrBuff[0] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			// Start transmitting and NACK should be received
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		} else {
			// In here all data has been transmitted
			TWDR = 0;
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		}
		break;
	case I2C_STDATA_ACK:
		// In here, our data has been transmitted and master has been acknowledge it
		// Load data to be transmitted
		if (tagI2C::slTrIdx > 1)
		{
			TWDR = tagI2C::slTrBuff[0];
			for (UCHAR iter=0; iter < tagI2C::slTrIdx-1; iter++)
			{
				tagI2C::slTrBuff[iter] = tagI2C::slTrBuff[iter+1];
			}
			tagI2C::slTrBuff[tagI2C::slTrIdx-1] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			
			// Start transmitting
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		} else if (tagI2C::slTrIdx == 1) {
			TWDR = tagI2C::slTrBuff[0];
			tagI2C::slTrBuff[0] = 0;
			tagI2C::slTrIdx = tagI2C::slTrIdx - 1;
			// Start transmitting and NACK should be received
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		} else {
			// In here all data has been transmitted
			TWDR = 0;
			TWCR = (TWCR & 0x2F) | _b(TWINT) | _b(TWEN);
		}
		break;
	case I2C_STDATA_NACK:
		// Our byte has been transmitted but master return NACK
		// Proceed to end transmission
		tagI2C::mode = FREE;
		TWCR = (TWCR & 0x0F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	case I2C_STLASTDATA_ACK:
		// We have sent last byte but master sent us ACK instead
		// Proceed to end transmission
		TWDR = 0;
		TWCR = TWCR = (TWCR & 0x0F) | _b(TWINT) | _b(TWEA) | _b(TWEN);
		break;
	default:
		if (tagI2C::ExtProc)
		{
			tagI2C::ExtProc(tagI2C::status);	
		}
		// Make sure that TWINT is cleared
		if (TWCR & _b(TWINT))
		{
			// Clear the TWINT bit by writing 1 to it
			TWCR = TWCR | _b(TWINT);
		}
		break;
	}
}

ISR(TWI_vect)
{
	// Interrupt Service Routine for I2C interrupt flag
	tagI2C::I2C_ISR();
}