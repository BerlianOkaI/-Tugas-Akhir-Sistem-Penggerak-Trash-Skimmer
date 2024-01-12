/*
 * RF433UART.h
 *
 * Created: 01/12/2023 10:16:29
 *  Author: Berlian Oka Irvianto, Indonesia
 
 Summary:
 +	Creating UART algorithm that will be used for
	RF communication between transmitter and receiver
	using RF433 Module for my [Trash Skimmer] project
 +	The configuration of UART that will be used is UART
	with baud rate of 1250. The frame that is used is
	8 bit mode, 1 stop bit, and odd parity
 +	Data bytes format that will be send is 5 byte size as follow:
	"@[ADC X][ADC Y][Button bits][Last Check]"
	@				: Start byte of message
	[ADC X]			: The value of ADC of analog joystick for x axis (maximum size: 255).
	[ADC Y]			: The value of ADC of analog joystick for y axis (maximum size: 255).
	[Button bits]	: bits of pushed button from transmitter.
	[Last Check]	: Stop byte of message consisting parity check for 3 data bytes that will be sent.
 */ 


#ifndef RF433UART_H_
#define RF433UART_H_

/* FOR UART REGISTER CONFIGURATION */
#define UART_FLAGS_REG				UCSR0A					// Flags and interrupts flags
#define UART_INTERRUPTS_REG			UCSR0B					// Interrupts enable 
#define UART_FRAMES_REG_H			UCSR0B					// Frame formating 1
#define UART_TRXENABLE_REG			UCSR0B					// Enabling Tx/Rx
#define UART_FRAMES_REG_L			UCSR0C					// Frame formating 0

#define FOSC						16000000				// Frekuensi dari Kristal yang digunakan
#define BAUD						2400					// Desired Baud Rate
#define MyUBRR						(FOSC/16/BAUD-1)		// Corresponding UBRR Register
#define DEF_UBRR					(FOSC/16/9600-1)		// Default UBRR for default baud rate (9600 bps)

#define FRAME_FORMAT_L				(0b00011 << UCSZ00)		// Frame format configuration (UCSZ1 and UCSZ0) (8 bit mode, 1 stop bit, No parity)
#define FRAME_FORMAT_H				(0b0 << UCSZ02)			// Frame format configuration (UCSZ2)			(8 bit mode, 1 stop bit, No parity)
#define ENABLE_TRX					(0b11 << TXEN0)			// Enabling transmitter and receiver
#define ASYNC_MODE					(0b0 << UMSEL00)		// Asynchronous Mode of UART
#define INTERRUPT_MODE				(0b11 << TXCIE0)		// Transmit complete and receive complete interrupt enable

#define UDR_EMPTY					(1 << UDRE0)			// bit of empty buffer

#define TX_MSG_SIZE					4
#define RX_MSG_SIZE					4
#define X_DATA						0
#define Y_DATA						1
#define BUTTON_DATA					2
#define MSG_PARITY					3
#define START_BYTE					64
#define IDLING_BYTE					255
/* TYPEDEFs */
typedef enum data_status
{
	READY, BUSY, EMPTY
} data_status;


/* IN CASE USING MODULE WITH AT-COMMAND */
#define AT_SETBAUD					"AT+B2400"				// Set the baud rate of RF module as 2400 bps
#define AT_SETFRAME					"AT+U8O1"				// Set the frame format of RF module as 8 bit mode, odd parity, and 1 stop bit.
#define AT_SETMODE					"AT+FU3"				// Set the transmission mode as FU3
#define AT_PORT						PORTD
#define AT_DDR						DDRD
#define AT_CTRPIN					(1 << 2)				// PD2


/* PROTOTYPES */
void RF433UART_Init(void);				// Inisiasi UART ATMega328p sesuai kebutuhan komunikasi RF433
void UART_TxChar(char chara);			// Kirim char melalui Tx Pin
void UART_TxStr(char *str);				// Kirim string (array of char) melalui Tx Pin
void UART_TxStr(char *str, int size);	// Kirim string (array of char) melalui Tx Pin

class RFData
{
private:
	data_status _tx_status;
	data_status _rx_status;
	unsigned char _tx_buffer[TX_MSG_SIZE];
	unsigned char _rx_buffer[RX_MSG_SIZE];
	unsigned char _rx_parity_error;
	unsigned char _rx_parity_stopbytes;
	unsigned char _dummy;
	unsigned char _parity_dummy;
	unsigned int _tx_pointer;
	unsigned int _rx_pointer;
	unsigned char _ParityCheck(unsigned char x1, unsigned char x2);
public:
	void AT_Setting(void);
	void init(void);
	// For transmitter
	void TX_update_data(unsigned char x_data, unsigned char y_data, unsigned char button_data);
	void ISR_transmitting(void);
	// For receiver
	void RX_default_data(void);
	void ISR_receiving(void);
	unsigned char RX_return_data(unsigned char* joystick_data_destination, unsigned char* button_data_destination);
	unsigned char RX_flush_data(void);
};


#endif /* RF433UART_H_ */