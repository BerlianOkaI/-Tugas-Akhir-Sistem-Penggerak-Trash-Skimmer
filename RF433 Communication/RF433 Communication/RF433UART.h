/*
 * RF433UART.h
 *
 * Created: 21/04/2024 05:16:22
 *  Author: ASUS
 */ 


#ifndef RF433UART_H_
#define RF433UART_H_

#include "TypeDef.h"

#define FOSC				16000000 
#define MYUBRR(BAUD)		(FOSC/16/BAUD-1)

#define CWSIZE			5
#define DWSIZE			3
#define RWSIZE			( CWSIZE - DWSIZE )
#define XDATA			0
#define YDATA			1
#define BUTTONDATA		2

#define IDLING_BYTE		0x58

class tagSerial
{
public:
	static void init(unsigned long ulBaudRate);
	static void write(WORD word);
	static void write(WORD* word);
	static WORD read(void);
	static WORD waitRead(void);
};
extern tagSerial Serial;

class RFData
{
public:
	RFData(UCHAR wSBAddress);
	// For transmitter
	BOOL TX_updateData(UCHAR x_data, UCHAR y_data, UCHAR button_data);
	void ISR_transmitting(void);
	// For receiver
	void RXDefaultData(void);
	void ISR_receiving(void);
	UCHAR RX_return_data(UCHAR* joystick_data_destination, UCHAR* button_data_destination);
	UCHAR RX_flush_data(void);
	
private:
	DataStatus	_tx_status;
	DataStatus	_rx_status;
	UCHAR		_tx_buff[CWSIZE];
	UCHAR		_rx_buff[CWSIZE];
	UCHAR		_tx_checkbyte[RWSIZE];  
	UCHAR		_rx_checkbyte[RWSIZE];
	UINT		_tx_pointer;
	UINT		_rx_pointer;
	
	WORD		_tx_iddlingcounter;
	WORD		_rx_parity_error;
	WORD		_parity_dummy;
	UCHAR		_dummy;
	
	UCHAR		_startbyte_adr;
	
	void parityGenerator(UCHAR* buffer);
};

#endif /* RF433UART_H_ */