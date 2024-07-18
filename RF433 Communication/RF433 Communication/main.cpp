/*
 * RF433 Communication.cpp
 *
 * Created: 21/04/2024 04:59:12
 * Author : ASUS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TypeDef.h"
#include "RF433UART.h"
#include "ErliaI2C.h"

#define MCU_LED_PORT	PORTB
#define MCU_LED			(1 << PORTB0)

typedef union
{
	UINT	uiData[3];
	char	cData[6];
} I2CRFTestFormat;

RFData		myRF(0x40);
UINT2BYTES	totalMsg;
UCHAR		buttonState;
UINT		receivedMsg;
UINT		lostMsg;
UINT		corruptedMsg;
UCHAR		msg_state;

// Buffer for I2C transmission
#define				BUFFER_SIZE		6
#define				ESP_ADR			0x41
I2CRFTestFormat		Data2I2C;

void setup(void)
{
	totalMsg.uiNum	= 0;
	buttonState		= 0;
	receivedMsg		= 0;
	lostMsg			= 0;
	corruptedMsg	= 0;
	// Initiating peripherals (serial and two-wire communication)
	Serial.init(2400);
	Serial.write("Hello World!");
	I2C.begin(100000, 0x40);
	sei();
}
int main(void)
{
	setup();
    /* Replace with your application code */
    while (1) 
    {
		/* Check whether there is received message from RF transmitter */
		msg_state = myRF.RX_return_data(totalMsg.bytes, &buttonState);
		if (msg_state > 0)
		{
			/* Evaluate the data */
			receivedMsg ++;
			lostMsg = totalMsg.uiNum - receivedMsg;
			// Check if the data is corrupted
			if (msg_state == 0x80)
			{
				corruptedMsg ++;
			}
			/* Send data to ESP32 via I2C / Two-Wire Protocol */
			Data2I2C.uiData[0] = totalMsg.uiNum;
			Data2I2C.uiData[1] = lostMsg;
			Data2I2C.uiData[2] = corruptedMsg;
			while (!I2C.MTransmit(Data2I2C.cData, BUFFER_SIZE, ESP_ADR));
		}
	}
}

ISR(USART_RX_vect)
{
	/*
	Receive complete interrupt. RFData.ISR_receiving()
	must be executed in this interrupt.
	*/
	myRF.ISR_receiving();
}

ISR(USART_TX_vect)
{
	myRF.ISR_transmitting();
}