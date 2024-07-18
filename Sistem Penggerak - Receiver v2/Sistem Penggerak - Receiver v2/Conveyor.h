
/*
 * Conveyor.h
 *
 * Created: 29/11/2023 13:25:02
 *  Author: ASUS
 */ 


#ifndef CONVEYOR_H_
#define CONVEYOR_H_

#define CLK_PERIOD		0.0625				// Period of CLK in us
#define CONV_PORT		PORTD				// D0 ~ D7 in arduino uno
#define CONV_DDR		DDRD				// Data Direction of Conv_Port. Must have a same port as CONV_PORT

#define CONV_IN1		PORTD4				
#define CONV_IN2		PORTD7				
#define CONV_EN			PORTD6				
#define CONV2_IN1		PORTD2				
#define CONV2_IN2		PORTD3
#define CONV2_EN		PORTD5


#include <avr/io.h>
#include <avr/interrupt.h>
#include "TypeDef.h"

/* PROTOTYPE */
void timer0SetUp(void);		// * Ubah jadi timer0
void conveyorSetup(void);			// * Ubah supaya pake timer0

void conveyorWrite(unsigned char dir, unsigned int pulse_length);		// * Ubah supaya pake timer0
void conveyorWrite2(unsigned char dir, unsigned int pulse_length);

#endif /* CONVEYOR_H_ */