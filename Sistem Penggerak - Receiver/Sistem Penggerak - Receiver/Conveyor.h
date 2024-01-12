
/*
 * Conveyor.h
 *
 * Created: 29/11/2023 13:25:02
 *  Author: ASUS
 */ 


#ifndef CONVEYOR_H_
#define CONVEYOR_H_

#define CLK_PERIOD 0.0625			// Period of CLK in us
#define CONV_PORT PORTD				// D0 ~ D7 in arduino uno
#define CONV_DDR DDRD				// Data Direction of Conv_Port. Must have a same port as CONV_PORT
#define CONV_IN1 PORTD4				// Not a PWM source (OC Pin)
#define CONV_IN2 PORTD7				// Not a PWM source (OC Pin)
#define CONV_EN PORTD3				// Make sure to use OC pin of timer 2 as source of PWM. In here, OC2B

/* PROTOTYPE */
void timer2SetUp(void);
void conveyorSetup(void);

void conveyorWrite(unsigned char dir, unsigned int pulse_length);


#endif /* CONVEYOR_H_ */