//==============================================================================
// Copyright (c) 2005-2010, Isaac Marino Bavaresco
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Neither the name of the author nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==============================================================================
// isaacbavaresco@yahoo.com.br
//==============================================================================
// Set TAB width to 4 characters
//==============================================================================
#if         !defined __LCD_CFG_H__
#define     __LCD_CFG_H__
//==============================================================================

//##############################################################################
//#                                                                            #
//#                   THIS FILE IS HEAVILY HARDWARE-DEPENDANT                  #
//#                                                                            #
//#          IT MUST BE CHANGED TO FIT YOUR BOARD'S CIRCUIT AND CLOCK          #
//#                                                                            #
//##############################################################################

// This version is taylored for my board at 4MHz and Hi-Tech PICC.
// Change this file for your own needs.

//==============================================================================
//
//                           PIC16F648A
//
//              +-----------------U-----------------+
//              |                                   |
// LCD_DI <-  ==|  1 RA2/AN2/Vref        AN1/RA1 18 |==  -> LCD_D4
//              |                                   |
//  LCD_E <-  ==|  2 RA3/AN3/CMP1        AN0/RA0 15 |==  -> LCD_D5
//              |                                   |
//  PWRON <-  ==|  3 RA4/T0CKI/CMP2     OSC1/RA7 16 |==  -> LCD_D6
//              |                                   |
//  RIGHT  -> ==|  4 RA5/!MCLR          OSC2/RA6 15 |==  -> LCD_D7
//              |                                   |
//            ==|  5 GND                     VCC 14 |==
//              |                                   |
//        <-  ==|  6 RB0/INT           T1OSI/RB7 13 |== <-  UP
//              |                                   |
//    RXD  -> ==|  7 RB1/RX/DT   T1CKI/T1OSO/RB6 12 |== <-  DOWN
//              |                                   |
//    TXD <-  ==|  8 RB2/TX/CK               RB5 11 |==  ->
//              |                                   |
//   LEFT  -> ==|  9 RB3/CCP1                RB4 10 |==  ->
//              |                                   |
//              +-----------------------------------+
//
//
//==============================================================================

#include <pic.h>

//==============================================================================

// Define this macro if your board uses the LCD R/!W pin.

//#define   USE_READ_PIN

// Define this macro if you want to detect when the LCD is not connected or is
// damaged. Works only if using the R/!W pin.

//#define   DETECT_FAILURE

// Number of repetitions before signailng a defective display.

//#define   NUMBER_OF_READS_TIMEOUT     2000

// Define this macro if your board uses four bit interface.

#define USE_FOUR_BIT_INTERFACE

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

// Helper macros. Depend on your board.

#define DATAPORT_OUT    PORTA
#define DATAPORT_IN     PORTA
#define DATAPORT_DIR    TRISA

#define LCD_D4          RA1
#define LCD_D5          RA0
#define LCD_D6          RA7
#define LCD_D7          RA6

#define LCD_E           RA3
#define LCD_E_DIR       TRISA3

#if         defined USE_READ_PIN

#define LCD_RW          RB6
#define LCD_RW_DIR      TRISB6

#endif  //  defined USE_READ_PIN

#define LCD_DI          RA2
#define LCD_DI_DIR      TRISA2

//==============================================================================

// These macros depend on the model of your LCD module.

// The size your screen will have at startup.

#define INITIAL_MAXX    16
#define INITIAL_MAXY     2

// The maximum size your screen may have.

#define MAXIMUM_MAXX    16
#define MAXIMUM_MAXY     2

// The usual is 8, but 4 is more useful for such a narrow display.

#define TAB_WIDTH        4

//==============================================================================

// Interrupt-related macros
// If you don't need to disable interrupts, just define them all as empty macros

// The macro 'DeclareIntSaveVar' must declare 's' with a type suitable to hold
// the CPU's current interrupts (enabled/disabled) state
#define DeclareIntSaveVar( s )      unsigned char s

// The macro 'SaveInterruptsState' must save the current interrupts state to 's'
#define SaveInterruptsState( s )    (( s ) = INTCON & 0xc0 )

// The macro 'RestoreInterruptsState' must restore the previous interrupts state
// from 's'
#define RestoreInterruptsState( s ) ( INTCON = INTCON | ( s ))

// The macro 'DisableInterrupts' must disable all interrupts that may interfere
// with the LCD interface (for instance, an interrupt that uses the LCD interface
// pins to scan a keyboard)
#define DisableInterrupts()         ( INTCON = INTCON & 0x3f )

//==============================================================================

// Data port manipulation macros

// The macro 'SetDataPortAsInputs' must set the data port bits (4 or 8,
// depending on the used mode) as inputs

#if         defined USE_READ_PIN

#define SetDataPortAsInputs()       ( DATAPORT_DIR |= 0xc3 )

#else   //  defined USE_READ_PIN

#define SetDataPortAsInputs()

#endif  //  defined USE_READ_PIN

// The macro 'SetDataPortAsOutputs' must set the data port bits (4 or 8,
// depending on the used mode) as outputs
#define SetDataPortAsOutputs()      ( DATAPORT_DIR &= 0x3c )

// In 8-bit mode, the macro 'ReadDataPortValue' must read all 8 bits of the
// data port to 's', in 4-bit mode it must read the 4 bits of the data
// port to the higher 4 bits of 's'
#define ReadDataPortValue( s )          do                                  \
                                            {                               \
                                            s    = 0;                       \
                                            if( LCD_D7 ) s  |= 0x80;        \
                                            if( LCD_D6 ) s  |= 0x40;        \
                                            if( LCD_D5 ) s  |= 0x20;        \
                                            if( LCD_D4 ) s  |= 0x10;        \
                                            }                               \
                                        while( 0 )

// In 8-bit mode, the macro 'SetDataPortValue' must output all the 8 bits of 's'
// to the data port, in 4-bit mode it must output the higher 4 bits of 's' to the
// data port
#define SetDataPortValue( s )           do                                  \
                                            {                               \
                                            char t;                         \
                                            t    = DATAPORT_OUT & 0x3c;     \
                                            if( s & 0x80 ) t    |= 0x40;    \
                                            if( s & 0x40 ) t    |= 0x80;    \
                                            if( s & 0x20 ) t    |= 0x01;    \
                                            if( s & 0x10 ) t    |= 0x02;    \
                                            DATAPORT_OUT    = t;            \
                                            }                               \
                                        while( 0 )
//( DATAPORT_OUT = DATAPORT_OUT & 0xf0 | ((( s ) >> 4 ) & 0x0f ))

// The following macros are only needed if you are using 4-bit interface

#if         defined USE_FOUR_BIT_INTERFACE

    // The macro 'ReadDataPortValueLow' must read the 4 bits of the data
    // port to the lower 4 bits of 's' without disturbing its 4 higher bits.
    #define ReadDataPortValueLow( s )   do                                  \
                                            {                               \
                                            if( LCD_D7 ) s  |= 0x08;        \
                                            if( LCD_D6 ) s  |= 0x04;        \
                                            if( LCD_D5 ) s  |= 0x02;        \
                                            if( LCD_D4 ) s  |= 0x01;        \
                                            }                               \
                                        while( 0 )

    // The macro 'SetDataPortValueLow' must output the lower 4 bits of 's' to
    // the data port.
    #define SetDataPortValueLow( s )    do                                  \
                                            {                               \
                                            char t;                         \
                                            t    = DATAPORT_OUT & 0x3c;     \
                                            if( s & 0x08 ) t    |= 0x40;    \
                                            if( s & 0x04 ) t    |= 0x80;    \
                                            if( s & 0x02 ) t    |= 0x01;    \
                                            if( s & 0x01 ) t    |= 0x02;    \
                                            DATAPORT_OUT    = t;            \
                                            }                               \
                                        while( 0 )

#endif  //  defined USE_FOUR_BIT_INTERFACE

//==============================================================================

// Control pins manipulation macros

#if         defined USE_READ_PIN

    #define SetRWAsRead()           ( LCD_RW = 1 )
    #define SetRWAsWrite()          ( LCD_RW = 0 )
    #define SetRWAsOutput()         ( LCD_RW_DIR = 0 )

#else   //  defined USE_READ_PIN

    #define SetRWAsRead()
    #define SetRWAsWrite()
    #define SetRWAsOutput()

#endif  //  defined USE_READ_PIN

#define SetDIValue( s )             ( LCD_DI = ( s ))
#define SetDIAsData()               ( LCD_DI = 1 )
#define SetDIAsInstruction()        ( LCD_DI = 0 )
#define SetDIAsOutput()             ( LCD_DI_DIR = 0 )

#define SetEAsEnabled()             ( LCD_E = 1 )
#define SetEAsDisabled()            ( LCD_E = 0 )
#define SetEAsOutput()              ( LCD_E_DIR = 0 )

//==============================================================================

void Delay_10us( unsigned short t );

// Delay macros.

#define Delay500ns()                asm( "nop" )


// This macro is defined this way (without calling a function) to save call-stack
// levels, because it is used in the deepest function of the package.

// NOTE: The last "nop" is important to prevent the optimizer to optimize the sequence.

#define Delay37us()                 do                          \
                                        {                       \
                                        asm( "movlw       9" ); \
                                        asm( "addlw      -1" ); \
                                        asm( "btfss     3,2" ); \
                                        asm( "goto      $-2" ); \
                                        asm( "nop          " ); \
                                        }                       \
                                    while( 0 )

#define Delay100us()                Delay_10us( 10 )

// This macro is defined this way (without calling a function) to save call-stack
// levels, because it is used in the deepest function of the package.

// NOTE: The last "nop" is important to prevent the optimizer to optimize the sequence.

#define Delay1520us()               do                          \
                                        {                       \
                                        asm( "movlw     254" ); \
                                        asm( "nop          " ); \
                                        asm( "nop          " ); \
                                        asm( "addlw      -1" ); \
                                        asm( "btfss     3,2" ); \
                                        asm( "goto      $-4" ); \
                                        asm( "nop          " ); \
                                        }                       \
                                    while( 0 )

#define Delay4100us()               Delay_10us( 410 )
#define Delay15ms()                 Delay_10us( 1500 )

//==============================================================================

// Macro or prototype for the function 'Beep'. Define it as an empty macro if
// your board doesn't have a buzzer or the like.

#define Beep()

//==============================================================================

// This macro defines the name that your 'putc' function must have to be linked
// with the library functions ('printf', etc).
// For MPLAB-C18 it is '_user_putc', for Hi-Tech PICC it is 'putch'.

#define LCD_PUTC                    putch

//==============================================================================
#endif  //  !defined __LCD_CFG_H__
//==============================================================================