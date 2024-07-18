/*
 * Sistem Penggerak - Transmitter.cpp
 *
 * Created: 01/12/2023 10:14:09
 * Author : Berlian Oka Irvianto
 *
 * For accessing the source code of this program, please visit my github in the following link below
 * https://github.com/BerlianOkaI/-Tugas-Akhir-Sistem-Penggerak-Trash-Skimmer.git
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TypeDef.h"
#include "RF433UART.h"
#include "Misc.h"
#include "JoystickButton.h"

#define BOARD_LED	(1 << PORTB0)
#define LED_PORT	PORTB

unsigned long	time_ms;
unsigned long	LED_time_ms;
UINT2BYTES		dataSent;
unsigned char	counter;

// Inisiasi Data
RFData myRFData(0x40);
JoystickButton myJSButton;

/** SETUP **/
void setup(void)
{
	time_ms = 0;
	LED_time_ms = 0;
	dataSent.uiData = 0;
	counter = 1;
	DDRB = DDRB | BOARD_LED;
	LED_PORT = LED_PORT & (~BOARD_LED);
	// Initiating communication
	Serial.init(2400);
	Serial.write("Hello World!");
	
	// Initiating ADC and Timer1 for Joystick Data Routine
	myJSButton.init();
	
	sei();
	Serial.write(10);	
}

/** INTERRUPTS  **/
ISR(USART_TX_vect)
{
	myRFData.ISR_transmitting();
}
ISR(USART_RX_vect)
{
	// Empty
}
ISR(TIMER1_OVF_vect)
{
	time_ms += 2;
	LED_time_ms += 2;
	myJSButton.read_button();
}
ISR(ADC_vect)
{
	myJSButton.read_joystick();
}

/** MAIN ALGORITHM **/
int main(void)
{
	setup();
    /* Replace with your application code */
    while (1) 
    {
		if (time_ms > 200)
		{
			
			time_ms = 0;
			
			myRFData.TX_updateData(
				myJSButton.return_x_data(),
				myJSButton.return_y_data(),
				myJSButton.return_button_data()
			);
			/*/
			dataSent.uiData ++;
			myRFData.TX_updateData(
				dataSent.bytes[0],
				dataSent.bytes[1],
				myJSButton.return_button_data()
			);
			//*/
		}
		if (LED_time_ms >= 1000)
		{
			LED_time_ms = 0;
			LED_PORT = (LED_PORT & (~BOARD_LED)) | ((~LED_PORT) & BOARD_LED);
		}
    }
	return 0;
}

