//==============================================================================
// Copyright (c) 2005-2025, Isaac Marino Bavaresco
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
#include <string.h>
#include "LCDcfg.h"
#include "LCD.h"
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
#if         defined USE_READ_PIN && USE_READ_PIN != 0 && defined DETECT_FAILURE && DETECT_FAILURE != 0
//==============================================================================
// Flag to indicate that the display is defective.
unsigned char               displayfailed   = 0;
//==============================================================================
signed char LCDHasFailed( void )
	{
	return displayfailed ? 1 : 0;
	}
//==============================================================================
void LCDClearFailedFlag( void )
	{
	displayfailed	= 0;
	}
//==============================================================================
#endif  //  defined USE_READ_PIN && USE_READ_PIN != 0 && defined DETECT_FAILURE && DETECT_FAILURE != 0
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

#if         defined USE_READ_PIN && USE_READ_PIN != 0
    unsigned char       s;

#if         defined DETECT_FAILURE && DETECT_FAILURE != 0
    unsigned short      n   = NUMBER_OF_READS_TIMEOUT;

    // The display has failed previously...
    if( displayfailed )
        // ... so we will not even try to access it
        return;
#endif  //  defined DETECT_FAILURE && DETECT_FAILURE != 0

#endif  //  defined USE_READ_PIN && USE_READ_PIN != 0

    SaveInterruptsState( Aux );

#if         defined USE_READ_PIN && USE_READ_PIN != 0

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

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

        // When using a 4-bit interface we need to do a dummy read

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Low half of the trasfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

        // Set READ/!WRITE pin as write (0)
        SetRWAsWrite();
        }
#if         !defined DETECT_FAILURE || DETECT_FAILURE == 0

    // Repeat until the BUSY bit is zero
    while( s & 0x80 );

#else   //  !defined DETECT_FAILURE || DETECT_FAILURE == 0

    // Repeat until the BUSY bit is zero or until the maximum number of repetitions.
    while(( s & 0x80 ) && ( --n != 0u ));

    // We tried the maximum number of times...
    if( n == 0u )
        // ... so we flag the display as failed
        displayfailed   = 1;

#endif  //  !defined DETECT_FAILURE || DETECT_FAILURE == 0

#else   //  defined USE_READ_PIN && USE_READ_PIN != 0

    DisableInterrupts();

#endif  //  defined USE_READ_PIN && USE_READ_PIN != 0

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

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0
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
#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0


    SetDataPortAsInputs();
    RestoreInterruptsState( Aux );

#if         !defined USE_READ_PIN || USE_READ_PIN == 0

    // If we are not reading the busy flag, then we must wait at least for
    // the worst-case execution time before any other command can be executed.

    // This write is a "clear screen" or a "home cursor" command...
    if( di == 0 && c <= 3 )
        // ... that takes up to 1.52ms to execute.
        Delay1520us();
    // For all other writes...
    else
        // ... the execution time is no more than 37us.
        Delay37us();

#endif  //  !defined USE_READ_PIN || USE_READ_PIN == 0

    }
//==============================================================================
#if 0
static unsigned char LCDReadStatus( void )
    {
    // Variable to save the interrupts state at function entry.
    DeclareIntSaveVar( Aux );
    // Variable to temporarily hold the value read.
    unsigned char       s;

    SaveInterruptsState( Aux );

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

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

    // Effectivate the data transfer.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Read the status byte (low nibble only).
    ReadDataPortValueLow( s );
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the minimum 'tLOW'.
    Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0


    // Set READ/!WRITE pin as write (0)
    SetRWAsWrite();

    RestoreInterruptsState( Aux );

    // Return the read data.
    return s;
    }
#endif
//==============================================================================
// Reads one data byte from the LCD.

#if         defined USE_READ_PIN && USE_READ_PIN != 0
static unsigned char LCDReadData( void )
    {
    // Variable to save the interrupts state at function entry.
    DeclareIntSaveVar( Aux );
    // Variable to temporarily hold the value read.
    unsigned char       s;

#if         defined DETECT_FAILURE && DETECT_FAILURE != 0
    // Variable to count the number of busy flag reads before an error is signaled.
    unsigned short      n   = NUMBER_OF_READS_TIMEOUT;

    // If the display failed previously...
    if( displayfailed )
        // ... we are not using it until some routine clears the error flag.
        return 0x00;
#endif  //  defined DETECT_FAILURE && DETECT_FAILURE != 0

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
        // Low half of the transfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

        // When using a 4-bit interface we need to do a dummy read

        // Effectivate the data transfer.
        SetEAsEnabled();
        // Wait for the minimum 'tHIGH'.
        Delay500ns();
        // Low half of the transfer cycle.
        SetEAsDisabled();
        // Wait for the minimum 'tLOW'.
        Delay500ns();

#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

        // Set READ/!WRITE pin as write (0)
        SetRWAsWrite();
        }
    // Test the LCD's BUSY flag.
#if         !defined DETECT_FAILURE || DETECT_FAILURE == 0
    while( s & 0x80 );
#else   //  !defined DETECT_FAILURE || DETECT_FAILURE == 0
    while(( s & 0x80 ) && ( --n != 0u ));

    if( n == 0u )
        displayfailed   = 1;
#endif  //  !defined DETECT_FAILURE || DETECT_FAILURE == 0


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

#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

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

#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

    // Set READ/!WRITE pin as write (0)
    SetRWAsWrite();

    RestoreInterruptsState( Aux );

    // Return the read data.
    return s;
    }

#endif  //  defined USE_READ_PIN && USE_READ_PIN != 0

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

#if         defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0
// Flag to enable the delayed scroll mode.
static unsigned char    delayedscroll   = 1;
#endif  //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

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
signed char LCDResizeScreen( unsigned char width, unsigned char height )
	{
	if( width < 1 || width > MAXIMUM_MAXX || height < 1 || height > MAXIMUM_MAXY )
		return -1;

	maxx	= width;
	maxy	= height;

	clrscr();

	return 1;
	}
//==============================================================================
#if         defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

void setscrollmode( unsigned char Mode )
    {
    delayedscroll = Mode != 0;
    }

#endif  //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

//==============================================================================

#if         ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )

static unsigned char screenbuffer[MAXIMUM_MAXY*MAXIMUM_MAXX];
static void scroll( void )
    {
    unsigned char x, y, c, *p, *q;

    for( y = 0, p = screenbuffer, q = screenbuffer + maxx; y < maxy - 1; y++ )
        {
        // Locate the cursor at the begining of the current text line
        LCDWriteCmd( 0x80 | (( y << 6 ) & 0x40 ) + ( y & 0x02 ? maxx : 0 ));
        for( x = maxx; x; x-- )
            {
            *p++ = c = *q++;
            LCDWriteData( c );
            }
        }

    // Locate the cursor at the begining of the last text line
    LCDWriteCmd( 0x80 | ((( maxy - 1 ) << 6 ) & 0x40 ) + (( maxy - 1 ) & 0x02 ? maxx : 0 ));
    // Iterate for all columns of the last text line...
    for( x = 0; x < maxx; x++ )
        {
        // ... fill the last text line with spaces
        *p++ = ' ';
        LCDWriteData( ' ' );
        }

    // Locate the cursor to its original position
    LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x3f ) + ( cursory & 0x02 ? maxx : 0 ));
    }

#else   //  ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )

static unsigned char screenbuffer[MAXIMUM_MAXX];
static void scroll( void )
    {
    unsigned char x, y;

    // Iterate for all text lines, starting from the second...
    for( y = 1; y < maxy; y++ )
        {
        // Locate the cursor at the begining of the current text line
        LCDWriteCmd( 0x80 | (( y << 6 ) & 0x40 ) + ( y & 0x02 ? maxx : 0 ));
        // Iterate for all columns of the current text line...
        for( x = 0; x < maxx; x++ )
            // ... read all the line characters to the buffer
            screenbuffer[x] = LCDReadData();
        // Locate the cursor at the begining of the text line right above the current one
        LCDWriteCmd( 0x80 | ((( y - 1 ) << 6 ) & 0x40 ) + (( y - 1 ) & 0x02 ? maxx : 0 ));
        // Iterate for all columns of the text line...
        for( x = 0; x < maxx; x++ )
            // ... print the whole line back
            LCDWriteData( screenbuffer[x] );
        }

    // Locate the cursor at the begining of the last text line
    LCDWriteCmd( 0x80 | ((( maxy - 1 ) << 6 ) & 0x40 ) + (( maxy - 1 ) & 0x02 ? maxx : 0 ));
    // Iterate for all columns of the last text line...
    for( x = 0; x < maxx; x++ )
        // ... fill the last text line with spaces
        LCDWriteData( ' ' );

    // Locate the cursor to its original position
    LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x3f ) + ( cursory & 0x02 ? maxx : 0 ));
    }

#endif  //   ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )

//==============================================================================
void clrscr( void )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

#if         ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )
	memset( screenbuffer, ' ', sizeof screenbuffer );
#endif	//	( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )

    // Locate the cursor at the top-left corner of the screen
    cursorx = 0;
    cursory = 0;
    // Send the clear screen command to the LCD controller
    LCDWriteCmd( CMD_CLEAR_DISPLAY );

//  RestoreInterruptsState( Aux );
    }
//==============================================================================
void gotoxy( signed char x, signed char y )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

	if( x < -maxx || x > maxx || y < -maxy || y > maxy )
		return;

	if( x == 0 )
		x	= cursorx + 1;
	else if( x < 0 )
		x	= maxx + x + 1;

	if( y == 0 )
		y	= cursory + 1;
	else if( y < 0 )
		y	= maxy + x + 1;

	// We need to positon the hardware cursor to the right place.
	LCDWriteCmd( CMD_SET_DDRAM_ADDRESS | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x3f ) + ( cursory & 0x02 ? maxx : 0 ));

//  RestoreInterruptsState( Aux );
    }
//==============================================================================
// This routine's real name is defined in the file 'LCDcfg.h'.
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

char LCD_PUTC( char c )
    {
//  interruptstate_t    Aux;
//
//  SaveInterruptsState( Aux );
//  DisableInterrupts();

#if         defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0
    // There is a pending scroll and the current character is not '\a', '\b' or '\f', ...
    if( cursorx >= maxx && c != (char)'\a' && c != (char)'\b' && c != (char)'\f' )
        {
        // ... return the cursor to the beginning of the next line...
        cursorx = 0;
        // ... and increment the line.
        cursory++;
        // The cursor is beyond the last line, ...
        if( cursory >= maxy )
            {
            // ... position the cursor on the last line...
            cursory = maxy - 1;
            // ... and do a 'scroll'.
            scroll();
            }
        }
#endif  //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

    switch( c )
        {

        //----------------------------------------------------------------------
        // '\a' = BELL

        case '\a':
            Beep();
//          RestoreInterruptsState( Aux );
            return c;

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
            return c;

        //----------------------------------------------------------------------
        // '\f' = FORMFEED

        case '\f':
            // Clear the screen.
            clrscr();
//          RestoreInterruptsState( Aux );
            return c;
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
            // Outside the 'switch' we will check if a 'scroll' is needed.
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
            return c;

        //----------------------------------------------------------------------
        // Here we deal with character tabulation ('\t' = TAB)

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
				{
#if         ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )
				// Store the character in the buffer. 
				screenbuffer[cursory*maxx+cursorx]	= c;
#endif	//	( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )
                LCDWriteData( ' ' );
				}

#if         defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

            // The cursor ended beyond the last column but it is not on the last line, ...
            if( cursorx >= maxx && ( cursory + 1 < maxy || !delayedscroll ))

#else   //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

            // The cursor ended beyond the last column, ...
            if( cursorx >= maxx )

#endif  //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0
                {
                // ... position the cursor to the beginning of the next line.
                cursorx = 0;
                cursory++;
                // Outside the 'switch' we will check if a 'scroll' is needed.
                break;
                }

            // The cursor is yet on the same line, we may return right now.
//          RestoreInterruptsState( Aux );
            return c;
            }

        // ---------------------------------------------------------------------- 
        // The character 14 (0x0e) will be translated to the redefinable
        // character 0 (zero).

        case 14:
            // Here we subtract 6, in the next case we subtract 8, resulting in zero.
            c   -= 6;
            // Fall-through.

        // ---------------------------------------------------------------------- 
        // The character 15 (0x0f) will be translated to the redefinable
        // character 7.

        case 15:
            c   -= 8;
            // Fall-through.

        // ---------------------------------------------------------------------- 
        // Here we cope with the printable characters.

        default:
#if         ( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )
			// Store the character in the buffer. 
			screenbuffer[cursory*maxx+cursorx]	= c;
#endif	//	( !defined USE_READ_PIN || USE_READ_PIN == 0 ) || ( defined OPTIMIZE_ACCESS && OPTIMIZE_ACCESS != 0 )

            // Print the character.
            LCDWriteData( c );
            // Increment the column.
            cursorx++;

#if         defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

            // The cursor ended beyond the last column but it is not on the last line, ...
            if( cursorx >= maxx && ( cursory + 1 < maxy || !delayedscroll ))

#else   //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0

            // The cursor ended beyond the last column, ...
            if( cursorx >= maxx )

#endif  //  defined USE_DELAYED_SCROLL && USE_DELAYED_SCROLL != 0
                {
                // ... position the cursor to the beginning of the next line.
                cursorx = 0;
                cursory++;
                // Outside the 'switch' we will check if a 'scroll' is needed.
                break;
                }
            // The cursor is yet on the same line, we may return right now.
//          RestoreInterruptsState( Aux );
            return c;
        }
    //--------------------------------------------------------------------------
    // The cursor is beyond the last line, ...
    if( cursory >= maxy )
        {
        // ... position it on the last line...
        cursory = maxy - 1;
        // ... and do a scroll.
        scroll();
        // We may return now, the 'scroll' already positioned the cursor on the right place.
//      RestoreInterruptsState( Aux );
        return c;
        }
    // We must position the LCD cursor on the right place.
    LCDWriteCmd( 0x80 | (( cursory << 6 ) & 0x40 ) | ( cursorx & 0x0f ) + ( cursory & 0x02 ? maxx : 0 ));
    //--------------------------------------------------------------------------
//  RestoreInterruptsState( Aux );
    return c;
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

    // The initial value of the LCD's DATA/!INSTRUCTION pin is 0 (instruction).
    SetDIAsInstruction();
    // The LCD's DATA/!INSTRUCTION pin is an output.
    SetDIAsOutput();

    // Chip initialization, following data-sheet procedure.

    // Delay mandated by the data-sheet

    // "Wait for more than 15 ms after VCC rises to 4.5 V."
    // or
    // "Wait for more than 40 ms after VCC rises to 2.7 V."

    Delay15ms();

    // Value to set the LCD interface to 8 bits wide, irrespective of the real board
    // interface width.
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

    // "Wait for more than 100us."
    Delay100us();

    // Issue the command (third time).
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();

    // Set the correct interface bus width
#if         defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

    // Wait for the execution time.
    Delay37us();
    // "Function set (Set interface to be 4 bits wide.) Interface is currently 8 bits wide."
    SetDataPortValue( 0x28 );
    // Issue the command.
    SetEAsEnabled();
    // Wait for the minimum 'tHIGH'.
    Delay500ns();
    // Low half of the trasfer cycle.
    SetEAsDisabled();
    // Wait for the execution time.
    Delay37us();
#else   //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

#if         !defined USE_READ_PIN || USE_READ_PIN == 0
    // Wait for the execution time.
    Delay37us();
#endif  //  !defined USE_READ_PIN || USE_READ_PIN == 0
    // "Function set (Interface is 8 bits wide.)"
    LCDWriteCmd( 0x38 );

#endif  //  defined USE_FOUR_BIT_INTERFACE && USE_FOUR_BIT_INTERFACE != 0

    // Turn display on
    LCDWriteCmd( 0x0c );
    // Clear display
    clrscr();
    }
//==============================================================================