/*
 * Sistem Penggerak - Receiver.cpp
 *
 * Created: 01/12/2023 09:49:07
 * Author : Berlian Oka Irvianto
 * 
 * For accessing the source code of this program, please visit my github in the following link below
 * https://github.com/BerlianOkaI/-Tugas-Akhir-Sistem-Penggerak-Trash-Skimmer.git
 */ 

#include "ProjectHeader.h"

RFData			myRFData(0x40);
ESC_Controller	myPropellers;
MainControl		myControl(&myRFData, &myPropellers);
unsigned long	timeCP_US;
unsigned long	timeCPMid_US;

float fJoystickValue[2];
float propLeftValue, propRightValue;

// FUZZY LOGIC FOR CONTROLLING ESC
// Fuzzy Logic: Antecedent part declaration
#define		X_LEFT		0
#define		X_MID		1
#define		X_RIGHT		2
#define		Y_BOT		0
#define		Y_MID		1
#define		Y_TOP		2
#define		JOY_X		0
#define		JOY_Y		1

FuzzySet	XAxisVar[3];
FuzzySet	YAxisVar[3];
FuzzyFrame	joystickInput[2];

// Fuzzy Logic: Consequent part declaration
#define		PROP_STOP	0
#define		PROP_RUN	1
#define		PROP_LEFT	0
#define		PROP_RIGHT	1

FuzzySet	propOutL[2];
FuzzySet	propOutR[2];
FuzzyFrame	proppelerOutput[2];

// Fuzzy Logic: Rules Declaration
FuzzyRule		PropRules[9];
unsigned int	JS_Antecedent[9][2];
unsigned int	prop_Consequent[9][2];

// Fuzzy Logic: Fuzzy System
FuzzySystem		propFuzzyControl(PropRules, 9);

void FuzzySetup(void);

// SETUPS
void setup(void)
{	
	timeCP_US = 0;
	timeCPMid_US = 500000;					// 500 ms
	fJoystickValue[0] = 0.0F;
	fJoystickValue[1] = 0.0F;
	propLeftValue = 0.0F;
	propRightValue = 0.0F;
	// Setup Fuzzy Control
	FuzzySetup();
	// Set the LED Pin as Output
	LED_BUILTIN_DDR = LED_BUILTIN_DDR | LED_BUILTIN;
	// Initiating ESC (Propellers), conveyor, and RF433 Communication through MainControl Object
	Serial.init(2400);
	myControl.all_init();
	// Enabling global interrupt
	sei();
}
// MAIN PROGRAM
int main(void)
{
	setup();
	// First, let's try to run the conveyor
	conveyorWrite(0, 190);
	LED_BUILTIN_PORT = LED_BUILTIN_PORT | LED_BUILTIN;
	// Waiting for ESC to be setup
	delay1ms(6000);
	
    while (1) 
    {
		myControl.update_device_state();
		delay1ms(100);
    }
}

// INTERRUPTS
ISR(TIMER0_OVF_vect)
{
	/* 
	Overflown Flag Interrupt of Timer 2.
	Please, refer to Conveyor.cpp to configure the timer 2
	*/
	myControl.ISR_Timer_Routine(32640);
	timeCP_US += 32640;
	if (timeCP_US >= timeCPMid_US && timeCP_US < TIMEOUT_LED)
	{
		// Turn Off LED 
		LED_BUILTIN_PORT = LED_BUILTIN_PORT & (~LED_BUILTIN);
	} else if (timeCP_US >= TIMEOUT_LED){
		timeCP_US = 0;
		LED_BUILTIN_PORT = LED_BUILTIN_PORT | LED_BUILTIN;
	}
}
ISR(USART_RX_vect)
{
	/*
	Receive complete interrupt. RFData.ISR_receiving()
	must be executed in this interrupt.
	*/
	myRFData.ISR_receiving();
}

ISR(USART_TX_vect)
{
	// Do nothing. Just clearing TXC Flag
}


MainControl::MainControl(RFData* RFData_address, ESC_Controller* ESC_Controller_address)
{
	this->_RFData_pointer = RFData_address;
	this->_ESC_Controller_pointer = ESC_Controller_address;
	this->_analog_joystick[0] = 128;
	this->_analog_joystick[1] = 128;
	this->_button_status = 0x00;
}

void MainControl::all_init(void)
{
	this->_timeout_us = 0;
	conveyorSetup();
	this->_ESC_Controller_pointer->init();
}

void MainControl::ISR_Timer_Routine(unsigned long deltime)
{
	// Update the timeout timer
	this->_timeout_us += deltime;
	if (this->_timeout_us > TIMEOUT_LIMIT)
	{
		this->_timeout_us = 0;
		// Force the RFData RX Buffer Data Train to be default!
		this->_RFData_pointer->RXDefaultData();
	}
}

void FuzzySetup(void)
{
	// Input Frames
	joystickInput[JOY_X].Frame_SetUp(XAxisVar, 3, 0.0F, 255.0F, INPUT);
	joystickInput[JOY_Y].Frame_SetUp(YAxisVar, 3, 0.0F, 255.0F, INPUT);

	joystickInput[JOY_X].Set_SetUp(X_LEFT, TRP_L, 0.0, 112.0);
	joystickInput[JOY_X].Set_SetUp(X_MID, TRP_C, 0.0, 112.0, 144.0, 255.0);
	joystickInput[JOY_X].Set_SetUp(X_RIGHT, TRP_R, 144.0, 255.0);

	joystickInput[JOY_Y].Set_SetUp(Y_BOT, TRP_L, 0.0, 112.0);
	joystickInput[JOY_Y].Set_SetUp(Y_MID, TRP_C, 0.0, 112.0, 144.0, 255.0);
	joystickInput[JOY_Y].Set_SetUp(Y_TOP, TRP_R, 144.0, 255.0);

	// Output Frames
	proppelerOutput[PROP_LEFT].Frame_SetUp(propOutL, 2, 0.0F, 10.0F, OUTPUT);
	proppelerOutput[PROP_LEFT].domainSetUp(0.0F, 10.0F, 10.0F);
	proppelerOutput[PROP_RIGHT].Frame_SetUp(propOutR, 2, 0.0F, 10.0F, OUTPUT);
	proppelerOutput[PROP_RIGHT].domainSetUp(0.0F, 10.0F, 10.0F);

	proppelerOutput[PROP_LEFT].Set_SetUp(PROP_STOP, SINGLE, 0.0F);
	proppelerOutput[PROP_LEFT].Set_SetUp(PROP_RUN, SINGLE, 10.0F);

	proppelerOutput[PROP_RIGHT].Set_SetUp(PROP_STOP, SINGLE, 0.0F);
	proppelerOutput[PROP_RIGHT].Set_SetUp(PROP_RUN, SINGLE, 10.0F);

	// Setting Fuzy Rules (This section will make you headache)
	PropRules[0].Rule_SetUp(joystickInput, JS_Antecedent[0], 2, proppelerOutput, prop_Consequent[0], 2);
	JS_Antecedent[0][JOY_X] = X_LEFT;
	JS_Antecedent[0][JOY_Y] = Y_BOT;
	prop_Consequent[0][PROP_LEFT] = PROP_STOP;
	prop_Consequent[0][PROP_RIGHT] = PROP_STOP;

	PropRules[1].Rule_SetUp(joystickInput, JS_Antecedent[1], 2, proppelerOutput, prop_Consequent[1], 2);
	JS_Antecedent[1][JOY_X] = X_MID;
	JS_Antecedent[1][JOY_Y] = Y_BOT;
	prop_Consequent[1][PROP_LEFT] = PROP_STOP;
	prop_Consequent[1][PROP_RIGHT] = PROP_STOP;

	PropRules[2].Rule_SetUp(joystickInput, JS_Antecedent[2], 2, proppelerOutput, prop_Consequent[2], 2);
	JS_Antecedent[2][JOY_X] = X_RIGHT;
	JS_Antecedent[2][JOY_Y] = Y_BOT;
	prop_Consequent[2][PROP_LEFT] = PROP_STOP;
	prop_Consequent[2][PROP_RIGHT] = PROP_STOP;

	// *
	PropRules[3].Rule_SetUp(joystickInput, JS_Antecedent[3], 2, proppelerOutput, prop_Consequent[3], 2);
	JS_Antecedent[3][JOY_X] = X_LEFT;
	JS_Antecedent[3][JOY_Y] = Y_MID;
	prop_Consequent[3][PROP_LEFT] = PROP_STOP;
	prop_Consequent[3][PROP_RIGHT] = PROP_RUN;

	PropRules[4].Rule_SetUp(joystickInput, JS_Antecedent[4], 2, proppelerOutput, prop_Consequent[4], 2);
	JS_Antecedent[4][JOY_X] = X_MID;
	JS_Antecedent[4][JOY_Y] = Y_MID;
	prop_Consequent[4][PROP_LEFT] = PROP_STOP;
	prop_Consequent[4][PROP_RIGHT] = PROP_STOP;

	PropRules[5].Rule_SetUp(joystickInput, JS_Antecedent[5], 2, proppelerOutput, prop_Consequent[5], 2);
	JS_Antecedent[5][JOY_X] = X_RIGHT;
	JS_Antecedent[5][JOY_Y] = Y_MID;
	prop_Consequent[5][PROP_LEFT] = PROP_RUN;
	prop_Consequent[5][PROP_RIGHT] = PROP_STOP;

	// *
	PropRules[6].Rule_SetUp(joystickInput, JS_Antecedent[6], 2, proppelerOutput, prop_Consequent[6], 2);
	JS_Antecedent[6][JOY_X] = X_LEFT;
	JS_Antecedent[6][JOY_Y] = Y_TOP;
	prop_Consequent[6][PROP_LEFT] = PROP_STOP;
	prop_Consequent[6][PROP_RIGHT] = PROP_RUN;

	PropRules[7].Rule_SetUp(joystickInput, JS_Antecedent[7], 2, proppelerOutput, prop_Consequent[7], 2);
	JS_Antecedent[7][JOY_X] = X_MID;
	JS_Antecedent[7][JOY_Y] = Y_TOP;
	prop_Consequent[7][PROP_LEFT] = PROP_RUN;
	prop_Consequent[7][PROP_RIGHT] = PROP_RUN;

	PropRules[8].Rule_SetUp(joystickInput, JS_Antecedent[8], 2, proppelerOutput, prop_Consequent[8], 2);
	JS_Antecedent[8][JOY_X] = X_RIGHT;
	JS_Antecedent[8][JOY_Y] = Y_TOP;
	prop_Consequent[8][PROP_LEFT] = PROP_RUN;
	prop_Consequent[8][PROP_RIGHT] = PROP_STOP;
}

void MainControl::update_device_state(void)
{
	if (this->_RFData_pointer->RX_return_data(this->_analog_joystick, &this->_button_status))
	{
		// Refresh timeout
		this->_timeout_us = 0;
		
		// Received new data. Calculate Fuzzy Output for ESC
		fJoystickValue[0] = (float)this->_analog_joystick[0];
		fJoystickValue[1] = (float)this->_analog_joystick[1]; 
		propLeftValue = propFuzzyControl.Defuzzyfication(fJoystickValue, PROP_LEFT);
		propRightValue = propFuzzyControl.Defuzzyfication(fJoystickValue, PROP_RIGHT);
		
		// Change conveyor action
		if (this->_button_status & VARCONV_BUTTON)
		{
			/* This button inhibit other control (propeller etc) */
			timeCPMid_US = 2000000;			// 2 s
			// Control the conveyor speed based on rotation direction of conveyor
			switch (this->_button_status & (CONVIN_BUTTON | CONVOUT_BUTTON))
			{
				case CONVIN_BUTTON:
					conveyorWrite(1, this->_analog_joystick[1]);
					break;
				case CONVOUT_BUTTON:
					conveyorWrite(2, this->_analog_joystick[1]);
					break;
				default:
					conveyorWrite(0, 180);
					break;
			}
			
		} else {
			/* In this case, conveyor and propeller can run simultaneously */
			// Conveyor speed has a constant value
			// Propeller speed can varied, based on fuzzy output from analog joystick input
			switch (this->_button_status & (CONVIN_BUTTON | CONVOUT_BUTTON))
			{
				case CONVIN_BUTTON:
					conveyorWrite(1, 200);
					break;
				case CONVOUT_BUTTON:
					conveyorWrite(2, 200);
					break;
				default:
					conveyorWrite(0, 180);
					break;
			}
			
			// Change ESC action
			if (this->_button_status & ACC_BUTTON)
			{
				timeCPMid_US = 2000000;			// 2 s
				this->_ESC_Controller_pointer->WritePercentage(propLeftValue + 10.0F, propRightValue + 10.0F);
			}
			else
			{
				timeCPMid_US = 500000;			// 500 ms
				this->_ESC_Controller_pointer->WritePercentage(propLeftValue, propRightValue);
			}
		}
		
	} else {
		// do nothing
	}
}

