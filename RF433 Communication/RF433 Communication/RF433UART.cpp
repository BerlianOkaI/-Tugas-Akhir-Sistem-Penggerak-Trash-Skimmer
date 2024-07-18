/*
 * A Library of ATmega328p to
 * handle RF433 Communication between ATmega328p
 * device via HC-12 using UART Protocol
 *
 * Berlian Oka Irvianto
 * Bandung (Indonesia), April 2024
 */ 

#include "RF433UART.h"

void tagSerial::init(unsigned long ulBaudRate)
{
	// Set the baud rate for the UART
	UBRR0H = (unsigned char) (MYUBRR(ulBaudRate) >> 8);
	UBRR0L = (unsigned char) (MYUBRR(ulBaudRate) & 0xFF);
	// Enable TX and RX and also their interrupts
	UCSR0B = _b(RXCIE0) | _b(TXCIE0) | _b(RXEN0) | _b(TXEN0);
	// 8-bit, no parity, and 1 stop bit mode
	UCSR0C = _b(UCSZ01) | _b(UCSZ00);
}

void tagSerial::write(WORD word)
{
	/* Transmitting one word (char) via UART */
	// Wait until data register is empty
	while(!(UCSR0A & _b(UDRE0)));
	// Send the word by putting it into transmit buffer
	UDR0 = word;
}

void tagSerial::write(WORD* word)
{
	/* Transmitting a array of word (string) via UART */
	UCHAR iter;
	iter = 0;
	while(word[iter] != 0)
	{
		tagSerial::write(word[iter]);
		iter++;
	}
}

WORD tagSerial::read(void)
{
	// Read the received data only if there is a received data in buffer
	if(UCSR0A & _b(RXC0))
	{
		return UDR0;
	} else {
		return 0;
	}
}

WORD tagSerial::waitRead(void)
{
	// Read the received data
	while(!(UCSR0A & _b(RXC0)));
	return UDR0;
}

RFData::RFData(UCHAR wSBAddress)
{
	this->_startbyte_adr	= wSBAddress;
	this->_tx_status		= EMPTY;
	this->_rx_status		= EMPTY;
	this->_tx_pointer		= 0;
	this->_rx_pointer		= 0;
	this->_tx_iddlingcounter= 0;
}

void RFData::parityGenerator(UCHAR* buffer)
{
	/*	Calculating 2 parity bytes that calculate parity
		of rows and columns of dataword.
		
		Codeword structures:
		
		[SB00]	[SB01]	[SB02]	[SB03]	[SB04]	[SB05]	[SB06]	[SB07]	|| [PB10]	[PB14]
		[DW00]	[DW01]	[DW02]	[DW03]	[DW04]	[DW05]	[DW06]	[DW07]	|| [PB11]	[PB15]
		[DW10]	[DW11]	[DW12]	[DW13]	[DW14]	[DW15]	[DW16]	[DW17]	|| [PB12]	[PB16]
		[DW20]	[DW21]	[DW22]	[DW23]	[DW24]	[DW25]	[DW26]	[DW27]	|| [PB13]	[PB17]
		================================================================[]
		[PB00]	[PB01]	[PB02]	[PB03]	[PB04]	[PB05]	[PB06]	[PB07]
	*/
	
	int iter;
	UCHAR dummyWord[DWSIZE+1];
	for (iter=0; iter<DWSIZE+1; iter++)
	{
		dummyWord[iter] = 0;
	}
	// Calculate the parity bytes of row of dataword using XOR operation
	for (iter=0; iter<8; iter++)
	{
		// In this code, we assume we use 3 byte of dataword and 5 byte of codeword
		dummyWord[0] = dummyWord[0] ^ _sb(this->_startbyte_adr, iter);
		dummyWord[1] = dummyWord[1] ^ _sb(buffer[0], iter);
		dummyWord[2] = dummyWord[2] ^ _sb(buffer[1], iter);
		dummyWord[3] = dummyWord[3] ^ _sb(buffer[2], iter);
	}
	buffer[DWSIZE+1] = (dummyWord[0] << 0) | (dummyWord[1] << 1);
	buffer[DWSIZE+1] = buffer[DWSIZE+1] | (dummyWord[2] << 2) | (dummyWord[3] << 3);
	buffer[DWSIZE+1] = buffer[DWSIZE+1] | ((~(buffer[DWSIZE+1]) << 4) & 0xF0);
	// Calculate the parity bytes of column of dataword using XOR operation
	buffer[DWSIZE] = this->_startbyte_adr ^ buffer[0] ^ buffer[1] ^ buffer[2];
}

BOOL RFData::TX_updateData(UCHAR x_data, UCHAR y_data, UCHAR button_data)
{
	// Updating and begin transmitting only if the data buffer is EMPTY before
	if (this->_tx_status == EMPTY)
	{
		this->_tx_buff[XDATA]			= x_data;
		this->_tx_buff[YDATA]			= y_data;
		this->_tx_buff[BUTTONDATA]		= button_data;
		this->parityGenerator(this->_tx_buff);
		// Change tx status so that data ready to be transmitted
		this->_tx_status = READY;
		// Trigger transmitter by sending idling byte
		tagSerial::write(IDLING_BYTE);
		return 1;
	} else {
		return 0;
	}
}

void RFData::ISR_transmitting(void)
{
	switch(this->_tx_status)
	{
	case READY:
		// change the status
		this->_tx_status = BUSY;
		// Initiating other transmit parameter
		this->_tx_pointer = 0;
		this->_tx_iddlingcounter = 0;
		// Send start byte to the target device
		tagSerial::write(this->_startbyte_adr);
		break;
	case BUSY:
		// Sending the message
		tagSerial::write(this->_tx_buff[this->_tx_pointer]);
		this->_tx_pointer++;
		if (this->_tx_pointer >= CWSIZE)
		{
			this->_tx_pointer = 0;
			this->_tx_status = EMPTY;
		}
		break;
	case EMPTY:
		// Sending idling byte
		if (this->_tx_iddlingcounter < 4)
		{
			tagSerial::write(IDLING_BYTE);
			this->_tx_iddlingcounter++;
		}
		break;
	}
}

void RFData::RXDefaultData(void)
{
	if (this->_rx_status != READY)
	{
		this->_rx_pointer			= 0;
		this->_rx_buff[XDATA]		= 128;
		this->_rx_buff[YDATA]		= 128;
		this->_rx_buff[BUTTONDATA]	= 0x00;
		this->parityGenerator(this->_rx_buff);
		
		// Data ready to be used
		this->_rx_status = READY;
	}
}

void RFData::ISR_receiving(void)
{
	// Receive the incoming msg first into this->_dummy
	// So that the receive buffer on hardware will be empty
	if (UCSR0A & (1 << FE0))
	{
		// Frame error detected.
		switch(this->_rx_status)
		{
			case EMPTY:
				// Check the parity error
				this->_parity_dummy = (UCSR0A & (1 << UPE0));
				// Read receive buffer register and turn the incoming byte as IDLING_BYTE
				this->_dummy = UDR0;
				this->_dummy = IDLING_BYTE;
				// With this, we hope we don't come to false START_BYTE
				break;
			case BUSY:
				// Check the parity error
				this->_parity_dummy = (UCSR0A & (1 << UPE0));
				// error detected, ignored data and proceed to change msg as default
				this->_dummy = UDR0;
				this->RXDefaultData();
				break;
			case READY:
				// Ignore the incoming data
				this->_parity_dummy = (UCSR0A & (1 << UPE0));
				this->_dummy = UDR0;
				break;
		}
	}
	else {
		this->_parity_dummy = (UCSR0A & (1 << UPE0));
		this->_dummy = UDR0;
	}
	
	// This below step depend on incoming msg char and _rx_status
	switch (this->_rx_status)
	{
	case EMPTY:
		if ((this->_dummy == this->_startbyte_adr) && (this->_parity_dummy == 0))
		{
			// Start Byte has been detected without parity error
			// Proceed to take the msg
			this->_rx_pointer = 0;
			this->_rx_parity_error = 0x00;
			this->_rx_status = BUSY;
		}
		break;
	case BUSY:
		// Update the RX Buffer
		this->_rx_buff[this->_rx_pointer] = this->_dummy;
		// Update wether there is a parity error or not
		if (this->_parity_dummy)
		{
			// There is parity error in this byte!
			this->_rx_parity_error = this->_rx_parity_error | (1 << _rx_pointer);
		}
		this->_rx_pointer ++;
		if (this->_rx_pointer >= CWSIZE)
		{
			// Data ready to be taken
			this->_rx_status = READY;
		}
		break;
	case READY:
		break;
	}
}

UCHAR RFData::RX_return_data(UCHAR* joystick_data_destination, UCHAR* button_data_destination)
{
	if (this->_rx_status == READY)
	{
		// Returning the data into MCU so that it can be used to control actuator (motor)
		// First, check wether there is a parity bit error or parity msg error
		this->_rx_checkbyte[0] = this->_rx_buff[DWSIZE];
		this->_rx_checkbyte[1] = this->_rx_buff[DWSIZE+1];
		this->parityGenerator(this->_rx_buff);
		this->_rx_checkbyte[0] = this->_rx_checkbyte[0] ^ this->_rx_buff[DWSIZE];
		this->_rx_checkbyte[1] = this->_rx_checkbyte[1] ^ this->_rx_buff[DWSIZE+1];
		
		if ((!this->_rx_checkbyte[0]) && (!this->_rx_checkbyte[1]) && (this->_rx_parity_error == 0))
		{
			// There is no parity error in both of each byte and total bytes
			// Proceed to returning data into destination
			joystick_data_destination[XDATA] = (UCHAR)this->_rx_buff[XDATA];
			joystick_data_destination[YDATA] = (UCHAR)this->_rx_buff[YDATA];
			*button_data_destination = (UCHAR)this->_rx_buff[BUTTONDATA];
			
			this->_rx_status = EMPTY;	// Proceed to receive next data train
			return 1;					// Return 1 to indicate that we have successfully return data
		} else {
			// There is an, indeed, error. Just ignore the data and proceed to receive next data train
			this->_rx_status = EMPTY;
			return 0x80;
		}
	}
	else
	{
		// If the data is not ready, just wait the next chance after the buffer is ready
		return 0;
	}
}

UCHAR RFData::RX_flush_data(void)
{
	if (this->_rx_status == READY)
	{
		this->_rx_status = EMPTY;
		return 1;
	}
	else
	{
		return 0;
	}
}
