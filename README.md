 These routines implement the interface to text LCD modules controled by the HD44780 controller and compatibles. The display mimics a TTY screen, with automatic scroll and recognizes the standard control codes '\a', '\b', '\f', '\n', '\r', '\t' and '\v'.

I'm using this code for some years now, in various slightly different versions. Now I decided to make the 'definitive' version, to be the most flexible yet as simple to use as possible.

This incarnation is not tested yet. I'm not sure the code without the R/!W pin is OK, because I always use the R/!W pin.

I included a lot of #if... to allow for maximum flexibility and I must test every combination to make sure I didn't screwed anything.

They can be used together with the 'stdio' functions ('printf', etc.) to program for embedded systems the same way someone would do for desktop systems.

    LCDcfg.h
    LCD.c

The files below are a lighter version of the ones above. In this version, the scroll was replaced by wrap-around, to simplify the code and save some RAM and hardware-stack levels.

This version was tested and works very well. It was adapted to a different board than the version above, please check the ASCII-art at line 43 of file LCDcfg-lite.h.

    LCDcfg-lite.h
    LCD-lite.c
