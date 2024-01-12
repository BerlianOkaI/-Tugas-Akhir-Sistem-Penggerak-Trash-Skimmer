/*
 * RF433UART.cpp
 *
 * Created: 01/12/2023 10:16:55
 *  Author: ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "RF433UART.h"
#include "Misc.h"

/* FUNCTIONS AND PROCEDURES */
void RF433UART_Init(void)
{
	// Set the baud rate
	UBRR0H = (unsigned char) (MyUBRR >> 8);
	UBRR0L = (unsigned char) (MyUBRR);
	
	// Enable the TX and RX
	UART_TRXENABLE_REG = ENABLE_TRX;
	
	// Set the Frame Format (5 bit mode - 2 stop bit)
	UART_FRAMES_REG_H = UART_FRAMES_REG_H | FRAME_FORMAT_H;
	UART_FRAMES_REG_L = ASYNC_MODE | FRAME_FORMAT_L;
	
	// Clear the RXC and TXC flags
	if (UART_FLAGS_REG & (1 << RXC0))
	{
		UART_FLAGS_REG = UART_FLAGS_REG | (1 << RXC0);
	}
	if (UART_FLAGS_REG & (1 << TXC0))
	{
		UART_FLAGS_REG = UART_FLAGS_REG | (1 << TXC0);
	}
	
	// Set the interrupt mode
	UART_INTERRUPTS_REG = UART_INTERRUPTS_REG | INTERRUPT_MODE;
}

void UART_TxChar(char chara)
{
	/*
	* Transmitting a 5 bit char to RF433 Transmitter
	* so that it can send data to RF433 Receiver
	*/
	// Wait until register is empty
	while(!(UART_FLAGS_REG & UDR_EMPTY));
	// Put data to the buffer to send data
	UDR0 = chara;
}

void UART_TxStr(char *str)
{
	int iter_;
	iter_ = 0;
	while(str[iter_] != 0)
	{
		UART_TxChar(str[iter_]);
		iter_ ++;
	}
}
void UART_TxStr(char *str, int size)
{
	int iter_;
	for (iter_=0; iter_ < size; iter_++)
	{
		UART_TxChar(str[iter_]);
	}
}

unsigned char RFData::_ParityCheck(unsigned char x1, unsigned char x2)
{
	// Generate a bytes of parity bits for last check
	return (((~x1) & x2) | (x1 & (~x2)));
}

void RFData::AT_Setting(void)
{
	// Set Default Baud Rate
	UBRR0H = (unsigned char) (DEF_UBRR >> 8);
	UBRR0L = (unsigned char) (DEF_UBRR);
	// Enable transmitter and receiver
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	// Set Frame as 8 bit, no parity, and 1 stop bit
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);
	// Disable interrupt mode
	UART_INTERRUPTS_REG = UART_INTERRUPTS_REG & (~INTERRUPT_MODE);
	// Setup for SET pin (give it low level logic)
	AT_DDR = AT_DDR | AT_CTRPIN;
	AT_PORT = AT_PORT & (~AT_CTRPIN);			// Low level, going into AT Command Mode
	delay1ms(50);
	AT_PORT = AT_PORT | (AT_CTRPIN);			// High level, escaping AT Command Mode
	delay1ms(50);
	AT_PORT = AT_PORT & (~AT_CTRPIN);			// Because we enter AT Command mode again with interval time that is less than 80 ms,
												// then we will enter AT Command with 9600 bps, No Parity Check, 1 stop bit
	
	delay1ms(200);
	// AT_COMMAND SETUP
	UART_TxStr(AT_SETMODE);
	delay1ms(100);
	UART_TxStr(AT_SETFRAME);
	delay1ms(100);
	UART_TxStr(AT_SETBAUD);
	delay1ms(100);
	
	// In this time, we have changed the module settings. Return it into transmission mode
	AT_PORT = AT_PORT | (AT_CTRPIN);
	delay1ms(200);
}

void RFData::init(void)
{
	RF433UART_Init();
	this->_tx_pointer = 0;
	this->_rx_pointer = 0;
	this->_tx_status = EMPTY;
	this->_rx_status = EMPTY;
	
}
void RFData::TX_update_data(unsigned char x_data, unsigned char y_data, unsigned char button_data)
{
	// Updating data of joystick and pushed button that will be sent to receiver
	// Only update it when status is EMPTY
	if (this->_tx_status == EMPTY)
	{	
		this->_tx_pointer = 0;
		this->_tx_buffer[X_DATA]			= x_data;												// Analog joystick, horizontal axis
		this->_tx_buffer[Y_DATA]			= y_data;												// Analog joystick, vertical axis
		this->_tx_buffer[BUTTON_DATA]		= button_data;											// Buttons
		this->_tx_buffer[MSG_PARITY]		= this->_ParityCheck(x_data, y_data);					// Stop bits consisting last parity checker
		this->_tx_buffer[MSG_PARITY]		= this->_ParityCheck(this->_tx_buffer[3], button_data);	// Stop bits consisting last parity checker
		
		this->_tx_status = READY;																	// Data is ready to be transmitted
	}
}



void RFData::ISR_transmitting(void)
{
	switch (this->_tx_status)
	{
	case READY:
		// Change status to BUSY
		this->_tx_status = BUSY;
		// Send '@' as start byte
		UART_TxChar(START_BYTE);
		break;
	case BUSY:
		UART_TxChar(this->_tx_buffer[_tx_pointer]);
		_tx_pointer ++;
		if (_tx_pointer >= TX_MSG_SIZE) {this->_tx_status = EMPTY;}
		break;
	case EMPTY:
		// Standby mode, just send '\n'
		UART_TxChar(IDLING_BYTE);
		break;
	}
}

void RFData::RX_default_data(void)
{
	// Return receive data as default data
	this->_rx_pointer = 0;
	this->_rx_buffer[X_DATA]		= 128;
	this->_rx_buffer[Y_DATA]		= 128;
	this->_rx_buffer[BUTTON_DATA]	= 0x00;
	this->_rx_buffer[MSG_PARITY]	= this->_ParityCheck(this->_rx_buffer[X_DATA], this->_rx_buffer[Y_DATA]);
	this->_rx_buffer[MSG_PARITY]	= this->_ParityCheck(this->_rx_buffer[MSG_PARITY], this->_rx_buffer[BUTTON_DATA]);
	
	// Data ready to be used
	this->_rx_status = READY;
}


void RFData::ISR_receiving(void)
{
	// Receive the incoming msg first into this->_dummy
	// So that the receive buffer on hardware will be empty
	if (UART_FLAGS_REG & (1 << FE0))
	{
		// Frame error detected.
		switch(this->_rx_status)
		{
		case EMPTY:
			// Check the parity error
			this->_parity_dummy = (UART_FLAGS_REG & (1 << UPE0));
			// Read receive buffer register and turn the incoming byte as IDLING_BYTE
			this->_dummy = UDR0;
			this->_dummy = IDLING_BYTE;
			// With this, we hope we don't come to false START_BYTE		
			break;
		case BUSY:
			// Check the parity error
			this->_parity_dummy = (UART_FLAGS_REG & (1 << UPE0));
			// error detected, ignored data and proceed to change msg as default
			this->_dummy = UDR0;
			this->RX_default_data();
			break;
		case READY:
			// Ignore the incoming data
			this->_parity_dummy = (UART_FLAGS_REG & (1 << UPE0));
			this->_dummy = UDR0;
			break;
		}
	}
	else {
		this->_parity_dummy = (UART_FLAGS_REG & (1 << UPE0));
		this->_dummy = UDR0;
	}
	
	// This below step depend on incoming msg char and _rx_status
	switch (this->_rx_status)
	{
	case EMPTY:
		if ((this->_dummy == START_BYTE) && (this->_parity_dummy == 0))
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
		this->_rx_buffer[this->_rx_pointer] = this->_dummy;
		// Update wether there is a parity error or not
		if (this->_parity_dummy)
		{
			// There is parity error in this byte! 
			this->_rx_parity_error = this->_rx_parity_error | (1 << _rx_pointer);
		}
		this->_rx_pointer ++;
		if (this->_rx_pointer >= RX_MSG_SIZE)
		{
			// Data ready to be taken
			this->_rx_status = READY;
		}
		break;
	case READY:
		break;	
	}
}
