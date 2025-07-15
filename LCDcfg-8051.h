/*============================================================================*/
/*
Copyright (c) 2005-2025, Isaac Marino Bavaresco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Neither the name of the author nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*============================================================================*/
/* isaacbavaresco@yahoo.com.br */
/*============================================================================*/
/* Set TAB width to 4 characters */
/*============================================================================*/
#if         !defined __LCD_CFG_H__
#define     __LCD_CFG_H__
/*============================================================================*/
/* This file is customized for 8051/8052 series MCUs. */
/*============================================================================*/
#include <reg52.h>
/*============================================================================*/
/* Define this macro to 1 (one) if your board uses the LCD R/!W pin. */

#define USE_READ_PIN				   0

/* Define this macro to 1 (one) if you want to detect when the LCD is not */
/* connected or is damaged. Works only if using the R/!W pin. */

#define DETECT_FAILURE				   0

/* Number of repetitions before signaling a defective display. */

#define NUMBER_OF_READS_TIMEOUT     2000

/* Define this macro to 1 (one) if your board uses four bit interface. */

#define USE_FOUR_BIT_INTERFACE		   1

#define	OPTIMIZE_ACCESS				   0

/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/* Helper macros. Depend on your board. */

#define DATAPORT			P1

/*============================================================================*/
/* These macros depend on the model of your LCD module. */

/* The size your screen will have at startup. */

#define INITIAL_MAXX    	16
#define INITIAL_MAXY     	 2

/* The maximum size your screen may have. Larger sizes will use more RAM if your */
/* board doesn't use the R/!W pin, because a screen-buffer is kept in RAM to */
/* allow scroll operations. */

#define MAXIMUM_MAXX    	16
#define MAXIMUM_MAXY     	 2

/* The usual is 8, but 4 is more useful for such a narrow display. */

#define TAB_WIDTH        	 4

/* This define activates the delayed scroll. */
/* In normal scroll mode, the screen scrolls when a character is shown on the */
/* last column of the last line of the screen. In delayed scroll mode, the */
/* screen scrolls only after one more character is shown. */

#define USE_DELAYED_SCROLL	 1

/*============================================================================*/
/* Interrupt-related macros */
/* If you don't need to disable interrupts, just define them all as empty macros */

/* The macro 'DeclareIntSaveVar' must declare 's' with a type suitable to hold */
/* the CPU's current interrupts (enabled/disabled) state */

#define DeclareIntSaveVar( s )      bit s

/* The macro 'SaveInterruptsState' must save the current interrupts state to 's' */

#define SaveInterruptsState( s )    (( s ) = EA )

/* The macro 'RestoreInterruptsState' must restore the previous interrupts state */
/* from 's' */

#define RestoreInterruptsState( s ) ( EA = ( s ))

/* The macro 'DisableInterrupts' must disable all interrupts that may interfere */
/* with the LCD interface (for instance, an interrupt that uses the LCD interface */
/* pins to scan a keyboard) */

#define DisableInterrupts()         ( EA = 0 )

/*============================================================================*/
/* Data port manipulation macros */

/* The macro 'SetDataPortAsInputs' must set the data port bits (4 or 8, */
/* depending on the used mode) as inputs */

#define SetDataPortAsInputs()       ( DATAPORT |= 0x0f )

/* The macro 'SetDataPortAsOutputs' must set the data port bits (4 or 8, */
/* depending on the used mode) as outputs */

#define SetDataPortAsOutputs()      

/* In 8-bit mode, the macro 'ReadDataPortValue' must read all 8 bits of the */
/* data port to 's', in 4-bit mode it must read the 4 bits of the data */
/* port to the higher 4 bits of 's' */

#define ReadDataPortValue( s )      (( s ) = DATAPORT << 4 )

/* In 8-bit mode, the macro 'SetDataPortValue' must output all the 8 bits of 's' */
/* to the data port, in 4-bit mode it must output the higher 4 bits of 's' to the */
/* data port */

#define SetDataPortValue( s )       ( DATAPORT = DATAPORT & 0xf0 | (( s ) >> 4 ))

/* The following macros are only needed if you are using 4-bit interface */

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

    /* The macro 'ReadDataPortValueLow' must read the 4 bits of the data */
    /* port to the lower 4 bits of 's' without disturbing its 4 higher bits. */

    #define ReadDataPortValueLow( s )   (( s ) |= DATAPORT & 0x0f )

    /* The macro 'SetDataPortValueLow' must output the lower 4 bits of 's' to */
    /* the data port. */

    #define SetDataPortValueLow( s )    ( DATAPORT = DATAPORT & 0xf0 | (( s ) & 0x0f ))

#endif  /*  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0 */
/*============================================================================*/
/* Control pins manipulation macros */

#if         defined USE_READ_PIN && USE_READ_PIN != 0

sbit	LCD_RW	= 0xb0;				/* LCD_RW is P3.0 */

    #define SetRWAsRead()			( LCD_RW = 1 )
    #define SetRWAsWrite()			( LCD_RW = 0 )
    #define SetRWAsOutput()

#else   /*  defined USE_READ_PIN && USE_READ_PIN != 0 */

    #define SetRWAsRead()
    #define SetRWAsWrite()
    #define SetRWAsOutput()

#endif  /*  defined USE_READ_PIN && USE_READ_PIN != 0 */

sbit	LCD_DI	= 0x94;				/* LCD_DI is P1.4 */
sbit	LCD_EN	= 0x95;				/* LCD_EN is P1.5 */

#define SetDIValue( s )             ( LCD_DI = ( s ))
#define SetDIAsData()               ( LCD_DI = 1 )
#define SetDIAsInstruction()        ( LCD_DI = 0 )
#define SetDIAsOutput()

#define SetEAsEnabled()             ( LCD_EN = 1 )
#define SetEAsDisabled()            ( LCD_EN = 0 )
#define SetEAsOutput()
/*============================================================================*/
/* Delay macros. */

#define	DELAY_CY(t)					do{unsigned char c;for(c=(t);--c;){}}while(0)

#define	Delay500ns()                
#define Delay37us()               	DELAY_CY( 18 )
#define Delay100us()                DELAY_CY( 50 )
#define Delay1520us()             	do{DELAY_CY(0);DELAY_CY(0);DELAY_CY(82);}while(0)
#define Delay4100us()               do{unsigned char c;for(c=3;--c;)DELAY_CY(227);}while(0)
#define Delay15ms()                 do{unsigned char c;for(c=10;--c;)DELAY_CY(249);}while(0)
/*============================================================================*/
/* Macro or prototype for the function 'Beep'. Define it as an empty macro if */
/* your board doesn't have a buzzer or the like. */

#define Beep()

/*============================================================================*/
/* This macro defines the name that your 'putc' function must have to be linked */
/* with the library functions ('printf', etc). */
/* For MPLAB-C18 it is '_user_putc', for Hi-Tech PICC it is 'putch', for 8052 it is 'putchar'. */

#define LCD_PUTC                    putchar

/*============================================================================*/
#endif  /*  !defined __LCD_CFG_H__ */
/*============================================================================*/
