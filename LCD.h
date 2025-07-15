/*============================================================================*/
#if			!defined __LCD_H__
#define __LCD_H__
/*============================================================================*/
void			LCDInit				( void );
signed char		LCDHasFailed		( void );
void			LCDClearFailedFlag	( void );
signed char		LCDResizeScreen		( unsigned char width, unsigned char height );
void			LCDDefineChar		( unsigned char CharIndex, const unsigned char *Pattern );
/*----------------------------------------------------------------------------*/
void			gotoxy				( signed char x, signed char y );
void			clrscr				( void );
unsigned char	getmaxx				( void );
unsigned char	getmaxy				( void );
unsigned char	getcursorx			( void );
unsigned char	getcursory			( void );
/*----------------------------------------------------------------------------*/
#define CURSOR_MODE_OFF				0
#define CURSOR_MODE_BLINK			1
#define CURSOR_MODE_ON				2
/*----------------------------------------------------------------------------*/
void			LCDControlCursor	( unsigned char Mode );
/*----------------------------------------------------------------------------*/
#define	SCROLL_MODE_NORMAL	0
#define	SCROLL_MODE_DELAYED	1
/*----------------------------------------------------------------------------*/
void			setscrollmode		( unsigned char Mode );
/*============================================================================*/
#endif	/*	!defined __LCD_H__ */
/*============================================================================*/
