/*
 * ESC_Control.h
 *
 * Created: 29/11/2023 14:44:53
 *  Author: Berlian Oka Irvianto (Indonesia)
 */ 


#ifndef ESC_CONTROL_H_
#define ESC_CONTROL_H_

#define ESC_PORT			PORTB			// D8 ~ D13 in arduino uno
#define ESC_DDR				DDRB			// Data Direction of ESC_Port
#define ESC_IN1				PORTB1			// PWM Source for controlling ESC1
#define ESC_IN2				PORTB2			// PWM Source for controlling ESC2
#define ESC_PWMREG1			OCR1A			// Output Compare Register of ESC1
#define ESC_PWMREG2			OCR1B			// Output Compare Register of ESC2

// Timer1 Configuration
#define ESCPWM_PRESCALER	0b010			// 8 As Prescaler
#define ESCPWM_TOP			39999			// Top value of Fast PWM so that we get PWM period of 20 ms

#define ESC_MINPULSE		2000			// Minimum Pulse width of ESC - ESC_MINPULSE must have a length of 1 ms
#define ESC_MAXPULSE		4000			// Maximum Pulse width of ESC - ESC_MAXPULSE must have a length of 2 ms
#define ESC_ZPULSE			3000			// Pulse where speed of ESC is zero
#define INCR_FACT			0.9
#define JS_CONST

#define ESC_LEFT			0
#define ESC_RIGHT			1

/* PROTOTYPES */
void timer1SetUp(void);
void ESCSetup(void);

class ESC_Controller
{
private:
	int _motor_L, _motor_R;				// Current Motor states
	float _motor_pow_level[2];
public:
	void init(void);
	void Throttling(void);
	void Write(unsigned int ESC1_PulseWidth, unsigned int ESC2_PulseWidth);
	void WritePercentage(float ESC1_percentage, float ESC2_percentage);
	void Clear(void);
};



#endif /* ESC_CONTROL_H_ */