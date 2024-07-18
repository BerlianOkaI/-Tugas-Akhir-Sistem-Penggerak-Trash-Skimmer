/*
 * JoystickButton.h
 *
 * Created: 01/12/2023 15:16:31
 *  Author: ASUS
 */ 



#ifndef JOYSTICKBUTTON_H_
#define JOYSTICKBUTTON_H_

#define T1_WGMODE_A				(0b10 << WGM10)				// Bits configuration for WG Mode (LSB) for Fast PWM with TOP of ICR1 (TCCR1A)
#define T1_WGMODE_B				(0b11 << WGM12)				// Bits configuration for WG Mode (MSB) for Fast PWM with TOP of ICR1 (TCCR1B)
#define T1_COMP_MODE			(0b00 << COM1B0)			// Compare output mode of Timer1 (normal mode for both output) (TCCR1A)
#define T1_PRESCALER			(0b010 << CS10)				// 8 as prescaler (TCCR1B)
#define T1_TOP					3999						// Top value for Timer1 counter operated in fast PWM mode. (Put this value to ICR1)				 


#define JOY_X					0							// Pin of adc for x-axis potentiometer in joystick
#define JOY_Y					1							// Pin of adc for y-axis potentiometer in joystick
#define ADC_PORT				PORTC
#define ADC_DDR					DDRC
#define ADC_USED_PINS			(1 << JOY_X)|(1 << JOY_Y)	// The pins that will be used

#define REF_MODE				(0b01 << REFS0)				// Volt reference of adc (using AVcc as reference)
#define ADC_ADJUST				(1 << ADLAR)				// ADC Left adjust for 8 bit mode adc
#define MUX_BITS				0x0F						// Mux bits in ADMUX Register
#define ADC_CONFIG				0xAF						// Enable ADC, Enable Auto Trigger, Enable ADIF interrupt, ADC Presclaer of 128
#define ADAT_SOURCE				(0b110 << ADTS0)			// Auto trigger source of Timer1 Overflown Flag

#define BUTTON_PORT				PORTD
#define BUTTON_DDR				DDRD
#define BUTTON_DDRCONFIG		(0xFC)
#define BUTTON_PIN				PIND						

/* PROTOTYPES */
void timer1_init(void);
void ADC_Init(void);
void ADC_change_mux(unsigned char channel);
unsigned char ADC_read_8bit(void);

typedef enum state
{
	X_POT,
	Y_POT	
} state;

class JoystickButton
{
private:
	unsigned char _x_data, _y_data;
	unsigned char _button_state;
	state _pot_state;
public:
	JoystickButton(void);
	void init(void);
	void data_reset(void);
	void read_joystick(void);
	void read_button(void);	
	
	unsigned char return_x_data(void);
	unsigned char return_y_data(void);
	unsigned char return_button_data(void);
};





#endif /* JOYSTICKBUTTON_H_ */