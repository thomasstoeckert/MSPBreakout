#include <msp430fr6989.h>
#include "Grlib/grlib/grlib.h"      // Graphics Library Import
#include "LcdDriver/lcd_driver.h"
#include "graphics.h"
#include <stdio.h>

int main(void)
{
    // Standard MSP430 Behavior
	WDTCTL   =  WDTPW | WDTHOLD;	  // Stop Watchdog Timer
	PM5CTL0 &= ~LOCKLPM5;             // Enable GPI
	
	// Configure SMCLK to 8MHz for SPI
	CSCTL0   =  CSKEY;                // Unlock CS registers
	CSCTL3  &= ~(BIT4|BIT5|BIT6);     // Divider = 0
	CSCTL0_H =  0;                    // Lock CS registers

	/////////////////////////////////////////////////////
	//           Initialize graphics library           //
	/////////////////////////////////////////////////////
	Graphics_Context g_sContext;      // Declare our graphics context for the library

	Crystalfontz128x128_Init();       // Initialize our display communication

	Initialize_Graphics(&g_sContext); // Prepare the display with initial commands

	/////////////////////////////////////////////////////
	//                Draw a temp screen               //
	/////////////////////////////////////////////////////

	Graphics_drawStringCentered(&g_sContext, "Breakout!", AUTO_STRING_LENGTH, 60, 64, OPAQUE_TEXT);

	// Go to sleep
	_low_power_mode_1();

	return 0;
}
