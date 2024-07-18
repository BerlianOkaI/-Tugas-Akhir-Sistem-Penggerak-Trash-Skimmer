/*
 * TypeDef.h
 *
 * Created: 21/04/2024 05:07:50
 *  Author: ASUS
 */ 


#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

#define _b(bit)			(1 << bit)
#define _sb(byte, bit)	((byte & _b(bit)) >> bit)

typedef char			WORD;
typedef int				DWORD;
typedef unsigned char	UCHAR;
typedef unsigned char	BOOL;
typedef unsigned int	UINT;

typedef enum {
	READY, BUSY, EMPTY
} DataStatus;



#endif /* TYPEDEF_H_ */