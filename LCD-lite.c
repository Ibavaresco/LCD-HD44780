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
#include "LCDcfg-lite.h"
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

#if         defined USE_READ_PIN && defined DETECT_FAILURE

// Flag to indicate that the display is defective.
unsigned char               displayfailed   = 0;

#endif  //  defined USE_READ_PIN && defined DETECT_FAILURE

//==============================================================================
#define LCDWriteCmd(c)  LCDWrite((c),0)
#define LCDWriteData(c) LCDWrite((c),1)

#define CMD_CLEAR_DISPLAY       0x01
#define CMD_SET_DDRAM_ADDRESS   0x80
//==============================================================================
// Writes a character or command to the display.
// Arguments:
//      c       The character or command to be written
//      di      Type of write: 0 = command, 1 = character

static void LCDWrite( unsigned char c, unsigned char di )
    {
    // Variable to save the interrupts state at function entry.
    DeclareIntSaveVar( Aux );

#if         defined USE_READ_PIN
    unsigned char       s;

#if         defined DETECT_FAILURE
    unsigned short      n   = NUMBER_OF_READS_TIMEOUT;

    // The display has failed previously...
    if( displayfailed )
        // ... so we will not even try to access it
        return;
#endif  //  defined DETECT_FAILURE

#endif  //  defined USE_READ_PIN

    SaveInterruptsState( Aux );

#if         defined USE_READ_PIN

    // Repeat while bit BUSY == 1
    do
        {
        RestoreInterruptsState( Aux );

        // At this point a pending interrupt may be serviced, so we don't
        // block the system for too long. On return, no assumption is
        // made about the state of the LCD interface pins, an interrupt
        // routine may change the value or direction of any pin, except
        // of the ENABLE pin.

        DisableInterrupts();

        SetDataPortAsInputs();
        // Set READ/!WRITE pin as read (1)
        SetRWAsRead();
        // Set DATA/!INSTRUCTION pin as instruction (0)
        SetDIAsInstruction();

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Read the status byte (only high nibble if in 4-bit mode).
        ReadDataPortValue( s );
        // Low half of the trasfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#if         defined USE_FOUR_BIT_INTERFACE

        // When using a 4-bit interface we need to do a dummy read

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Low half of the trasfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE

        // Set READ/!WRITE pin as write (0)
        SetRWAsWrite();
        }
#if         !defined DETECT_FAILURE

    // Repeat until the BUSY bit is zero
    while( s & 0x80 );

#else   //  !defined DETECT_FAILURE

    // Repeat until the BUSY bit is zero or until the maximum number of repetitions.
    while(( s & 0x80 ) && ( --n != 0u ));

    // We tried the maximum number of times...
    if( n == 0u )
        // ... so we flag the display as failed
        displayfailed   = 1;

#endif  //  !defined DETECT_FAILURE

#else   //  defined USE_READ_PIN

    DisableInterrupts();

#endif  //  defined USE_READ_PIN

    //--------------------------------------------------------------------------
    // Now we may send the data.
    //--------------------------------------------------------------------------

    // Set READ/!WRITE pin as write (0)
    SetRWAsWrite();
    // Set DATA/!INSTRUCTION pin as data (1).
    SetDIValue( di );
    // The data (only high nibble if in 4-bit mode) is output to the data port.
    SetDataPortValue( c );
    SetDataPortAsOutputs();
    // Effectivate the data transfer.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the minimum 'tLOW'.
    Delay500ns();

#if         defined USE_FOUR_BIT_INTERFACE
    // The low nibble is output to the data port.
    SetDataPortValueLow( c );
    // Effectivate the data transfer.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the minimum 'tLOW'.
    Delay500ns();
#endif  //  defined USE_FOUR_BIT_INTERFACE


    SetDataPortAsInputs();
    RestoreInterruptsState( Aux );

#if         !defined USE_READ_PIN

    // If we are not reading the busy flag, then we must wait at least for
    // the worst-case execution time before any other command ca be executed.

    // This write is a "clear screen" or a "home cursor" command...
    if( di == 0 && c <= 3 )
        // ... that takes up to 1.52ms to execute.
        Delay1520us();
    // For all other writes...
    else
        // ... the execution time is no more than 37us.
        Delay37us();

#endif  //  !defined USE_READ_PIN

    }
//==============================================================================
// Reads one data byte from the LCD.

#if         defined USE_READ_PIN
static unsigned char LCDReadData( void )
    {
    // Variable to save the interrupts state at function entry.
    DeclareIntSaveVar( Aux );
    // Variable to temporarily hold the value read.
    unsigned char       s;

#if         defined DETECT_FAILURE
    // Variable to count the number of busy flag reads before an error is signaled.
    unsigned short      n   = NUMBER_OF_READS_TIMEOUT;

    // If the display failed previously...
    if( displayfailed )
        // ... we are not using it until some routine clears the error flag.
        return 0x00;
#endif  //  defined DETECT_FAILURE

    SaveInterruptsState( Aux );

    // Repeat while busy flag == 1
    do
        {
        RestoreInterruptsState( Aux );

        // At this point a pending interrupt may be serviced, so we don't
        // block the system for too long. On return, no assumption is
        // made about the state of the LCD interface pins, an interrupt
        // routine may change the value or direction of any pin, except
        // of the ENABLE pin.

        DisableInterrupts();

        SetDataPortAsInputs();
        // Set the READ/!WRITE pin as read (1)
        SetRWAsRead();
        // Set DATA/!INSTRUCTION pin as instruction (0)
        SetDIAsInstruction();

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Read the status byte (only high nibble if in 4-bit mode).
        ReadDataPortValue( s );
        // Low half of the trasfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#if         defined USE_FOUR_BIT_INTERFACE

        // When using a 4-bit interface we need to do a dummy read

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Low half of the trasfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE

        // Set READ/!WRITE pin as write (0)
        SetRWAsWrite();
        }
    // Test the LCD's BUSY flag.
#if         !defined DETECT_FAILURE
    while( s & 0x80 );
#else   //  !defined DETECT_FAILURE
    while(( s & 0x80 ) && ( --n != 0u ));

    if( n == 0u )
        displayfailed   = 1;
#endif  //  !defined DETECT_FAILURE


    //--------------------------------------------------------------------------
    // Now we can read the data.
    //--------------------------------------------------------------------------

    // Set READ/!WRITE pin as read (1)
    SetRWAsRead();
    // Set DATA/!INSTRUCTION pin as data (1).
    SetDIAsData();
    // Effectivate the data transfer.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Read the data byte (only high nibble if in 4-bit mode).
    ReadDataPortValue( s );
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the minimum 'tLOW'.
    Delay500ns();

#if         defined USE_FOUR_BIT_INTERFACE

    // Effectivate the data transfer.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Read the data byte (low nibble only).
    ReadDataPortValueLow( s );
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the minimum 'tLOW'.
    Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE

    // Set READ/!WRITE pin as write (0)
    SetRWAsWrite();

    RestoreInterruptsState( Aux );

    // Return the read data.
    return s;
    }

#endif  //  defined USE_READ_PIN

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//
// Hardware-independent part
//
//
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================


//==============================================================================

// Current coordinates of the cursor.
static unsigned char    cursorx         = 0,    cursory = 0;

// Number of columns and lines of the screen.
static unsigned char    maxx            = INITIAL_MAXX, maxy    = INITIAL_MAXY;

//==============================================================================
unsigned char getmaxx( void )
    {
    return maxx;
    }
//==============================================================================
unsigned char getmaxy( void )
    {
    return maxy;
    }
//==============================================================================
unsigned char getcursorx( void )
    {
    return cursorx + 1;
    }
//==============================================================================
unsigned char getcursory( void )
    {
    return cursory + 1;
    }
//==============================================================================

void clrscr( void )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

    // Locate the cursor at the top-left corner of the screen
    cursorx = 0;
    cursory = 0;
    // Send the clear screen command to the LCD controller
    LCDWriteCmd( CMD_CLEAR_DISPLAY );

//  RestoreInterruptsState( Aux );
    }

//==============================================================================

void gotoxy( unsigned char x, unsigned char y )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

    // If the coordinates are valid...
    if( x <= maxx && y <= maxy )
        {
        // If the column is different than zero...
        if( x != 0u )
            // ... change the current column to it.
            cursorx = x - 1;
        // If the line is different than zero...
        if( y != 0u )
            // ... change the current line to it.
            cursory = y - 1;

        // We need to positon the hardware cursor to the right place.
        LCDWriteCmd( CMD_SET_DDRAM_ADDRESS | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x3f ) + ( cursory & 0x02 ? maxx : 0 ));
        }

//  RestoreInterruptsState( Aux );
    }

//==============================================================================
// This routine's real name is defined in the file 'LCDcfg-lite.h'.
// The name must be chosen to allow the library routines (printf, etc.) to link
// to it.

// NEW: The redefinable characters 0x01 to 0x06 may be printed directly, but
// characters 0x00 and 0x07 conflict with special control codes. They may be
// printed by using codes 0x0e and 0x0f, respectively.
// This is the best possible solution, because we don't lose neither displayable
// nor control characters (characters 8 to 15 are just a shadow of characters 0
// to 7).

// Character  0         = '\0' (end of string)
// Characters 1 to 6    = special redefinable characters 1 to 6.
// Characters 7 to 13   = standard control characters.
// Character  14        = printed as redefinable character zero.
// Character  15        = printed as redefinable character 7.


void LCD_PUTC( char c )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

    switch( c )
        {

        //----------------------------------------------------------------------
        // '\a' = BELL

        case '\a':
            Beep();
//          RestoreInterruptsState( Aux );
            return /*0*/;

        //----------------------------------------------------------------------
        // '\b' = BACKSPACE

        case '\b':
            // The cursor is not on the first column, ...
            if( cursorx > 0u )
                // ... decrement the column.
                cursorx--;
            // The cursor is on the first column but it is not on the first line, ...
            else if( cursory > 0u )
                {
                // ... position the cursor at the end of the previous line.
                cursory--;
                cursorx = maxx - 1;
                }
            // The LCD must be updated with the correct cursor coordinates.
            LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x0f ) + ( cursory & 0x02 ? maxx : 0 ));
            // ... finished.
//          RestoreInterruptsState( Aux );
            return /*0*/;

        //----------------------------------------------------------------------
        // '\f' = FORMFEED

        case '\f':
            // Clear the screen.

            // Locate the cursor at the top-left corner of the screen
            cursorx = 0;
            cursory = 0;
            // Send the clear screen command to the LCD controller
            LCDWriteCmd( CMD_CLEAR_DISPLAY );

//          RestoreInterruptsState( Aux );
            return /*0*/;
        //----------------------------------------------------------------------
        // '\n' = NEWLINE

        case '\n':
            // Return the cursor to the beginning of the line.
            cursorx = 0;
            // Fall-through to 'VTAB', to increment the line

        //----------------------------------------------------------------------
        // '\v' = VTAB

        case '\v':
            // Increment the line number.
            cursory++;
            // Outside the 'switch' we will check if a wrap-around is needed.
            break;

        //----------------------------------------------------------------------
        // '\r' = CR (carriage return)

        case '\r':
            // Return the cursor to the beginning of the line.
            cursorx = 0;
            // The LCD must be updated with the correct cursor coordinates.
            LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x0f ) + ( cursory & 0x02 ? maxx : 0 ));
            // Fim
//          RestoreInterruptsState( Aux );
            return /*0*/;

        //----------------------------------------------------------------------
        // Aqui tratamos do caractere tabula��o ('\t' = TAB)

        case '\t':
            {
            // We need a temporary variable.
            unsigned char   Temp;
            // Calculate the column of the next tabulation.
            Temp = ( cursorx + TAB_WIDTH ) & ~( TAB_WIDTH - 1 );
            // The cursor will be beyond the column after the last, ...
            if( Temp > maxx )
                // ... restrict the cursor to the column after the last.
                Temp = maxx;
            // Calculate how many spaces must be inserted.
            Temp    -= cursorx;
            // Advance the column to its final position.
            cursorx += Temp;
            // Print as many spaces as needed to reach the final position
            for( ; Temp; Temp-- )
                LCDWriteData( ' ' );

            // The cursor ended beyond the last column, ...
            if( cursorx >= maxx )
                {
                // ... position the cursor to the beginning of the next line.
                cursorx = 0;
                cursory++;
                // Outside the 'switch' we will check if a wrap-around is needed.
                break;
                }

            // The cursor is yet on the same line, we may return right now.
//          RestoreInterruptsState( Aux );
            return /*0*/;
            }

        //----------------------------------------------------------------------
        // The character 14 (0x0e) will be translated to the redefinable
        // character 0 (zero).

        case 14:
            // Here we subtract 6, in the next case we subtract 8, resulting in zero.
            c   -= 6;
            // Fall-through.

        //----------------------------------------------------------------------
        // The character 15 (0x0f) will be translated to the redefinable
        // character 7.

        case 15:
            c   -= 8;
            // Fall-through.

        //----------------------------------------------------------------------
        // Here we cope with the printable characters.

        default:
            // Print the character.
            LCDWriteData( c );
            // Increment the column.
            cursorx++;

            // The cursor ended beyond the last column, ...
            if( cursorx >= maxx )
                {
                // ... position the cursor to the beginning of the next line.
                cursorx = 0;
                cursory++;
                // Outside the 'switch' we will check if a wrap-around is needed.
                break;
                }
            // The cursor is yet on the same line, we may return right now.
//          RestoreInterruptsState( Aux );
            return /*0*/;
        }
    //--------------------------------------------------------------------------
    // The cursor is beyond the last line, ...
    if( cursory >= maxy )
        // ... position it on the last line...
        cursory = 0;

    // We must position the LCD cursor on the right place.
    LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x0f ) + ( cursory & 0x02 ? maxx : 0 ));
    //--------------------------------------------------------------------------
//  RestoreInterruptsState( Aux );
    return /*0*/;
    }
//==============================================================================
void LCDControlCursor( unsigned char Mode )
    {
    LCDWriteCmd(( Mode & 0x03 ) | 0x0c );
    }
//==============================================================================
void LCDDefineChar( unsigned char Char, const unsigned char *Pattern )
    {
    unsigned char c;

    LCDWriteCmd((( Char << 3 ) & 0x38 ) | 0x40 );
    for( c = 8; c; c-- )
        LCDWriteData( *Pattern++ );
    LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x3f ) + ( cursory & 0x02 ? maxx : 0 ));
    }
//==============================================================================
void LCDInit( void )
    {
    // Initialize the LCD data interface.

    // The initial value of the LCD's ENABLE pin is 0 (disabled).
    SetEAsDisabled();
    // The LCD's ENABLE pin is an output.
    SetEAsOutput();

    // The initial value of the LCD's READ/!WRITE pin is 0 (write).
    SetRWAsWrite();
    // The LCD's READ/!WRITE pin is an output.
    SetRWAsOutput();

    // The initial value of the LCD's DATA/!INSTRUCTION pin is 0 (data).
    SetDIAsInstruction();
    // The LCD's DATA/!INSTRUCTION pin is an output.
    SetDIAsOutput();

    // Chip initialization, following data-sheet procedure.

    // Delay mandated by the data-sheet

    // "Wait for more than 15 ms after VCC rises to 4.5 V."
    // or
    // "Wait for more than 40 ms after VCC rises to 2.7 V."

    Delay15ms();

    // Value to set the LCD interface to 8 bits, irrespective of the real board
    // interface length.
    SetDataPortValue( 0x30 );

    SetDataPortAsOutputs();

    // The command to change the interface to 8 bits must be issued 3 times.

    // Issue the command (first time).
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();

    // Delay mandated by the data-sheet

    // "Wait for more than 4.1 ms."
    Delay4100us();

    // Issue the command (second time).
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();

    // Delay mandated by the data-sheet

    // "Wait for more than 100 �s."
    Delay100us();

    // Issue the command (third time).
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();

#if         !defined USE_READ_PIN
    // Wait for the execution time.
    Delay37us();
#endif  //  !defined USE_READ_PIN

    // Set the correct interface length
#if         defined USE_FOUR_BIT_INTERFACE

    // "Function set (Set interface to be 4 bits long.) Interface is 8 bits in length."
    LCDWriteCmd( 0x28 );

#else   //  defined USE_FOUR_BIT_INTERFACE

    // "Function set (Interface is 8 bits long.)"
    LCDWriteCmd( 0x38 );

#endif  //  defined USE_FOUR_BIT_INTERFACE

    // Turn display on
    LCDWriteCmd( 0x0c );
    // Clear display
    LCDWriteCmd( CMD_CLEAR_DISPLAY );
    }
//==============================================================================