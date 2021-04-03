#include <msp430fr6989.h>
#include "Grlib/grlib/grlib.h"      // Graphics Library Import
#include "LcdDriver/lcd_driver.h"
#include "graphics.h"
#include "math_utils.h"
#include <stdio.h>

Graphics_Context g_sContext;      // Declare our graphics context for the library

#define redLED BIT0
#define circleRadius 2

int main(void)
{
    // Standard MSP430 Behavior
	WDTCTL   =  WDTPW | WDTHOLD;	  // Stop Watchdog Timer
	PM5CTL0 &= ~LOCKLPM5;             // Enable GPI
	
	// Configure SMCLK to 8MHz for SPI
	CSCTL0   =  CSKEY;                // Unlock CS registers
	CSCTL3  &= ~(BIT4|BIT5|BIT6);     // Divider = 0
	CSCTL0_H =  0;                    // Lock CS registers

	// Configure debug LED
	P1DIR |=  redLED;
	P1OUT &= ~redLED;

	/////////////////////////////////////////////////////
	//           Initialize graphics library           //
	/////////////////////////////////////////////////////
	Crystalfontz128x128_Init();       // Initialize our display communication

	Initialize_Graphics(&g_sContext); // Prepare the display with initial commands

	/////////////////////////////////////////////////////
	//               Initialize update timer           //
	/////////////////////////////////////////////////////
	// Use SMCLK @8KHz, Divide by 8, in up mode. Clear it, enable the interrupt flag
	TA0CTL  =  TASSEL_2 | ID_3 | MC_1 | TACLR | TAIE;
	TA0CCR0 =  33333 - 1;             // Appx. 30.003Hz, or, 33.33ms
	TA0CTL &= ~TAIFG;                 // Clear interrupt flag

	// Go to sleep
	_low_power_mode_1();

	return 0;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void FIXED_UPDATE(void) {
    static int velocity_x = 1, velocity_y = 2;
    static int pos_x = 64, pos_y = 64;

    // Clear our old position
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillCircle(&g_sContext, pos_y, pos_x, 6);

    // Calculate our new position

    int temp_pos_x = pos_x, temp_pos_y = pos_y;

    temp_pos_x += velocity_x;
    temp_pos_y += velocity_y;

    // Create our bounds for our circle
    Graphics_Rectangle circleBounds = {
                                       temp_pos_x - circleRadius,
                                       temp_pos_y - circleRadius,
                                       temp_pos_x + circleRadius,
                                       temp_pos_y + circleRadius
    };

    // Check if circle collides with walls
    char isCollidingWalls = IsCollidingWalls(&circleBounds);

    // Our circle is colliding
    if(isCollidingWalls != 0) {
        if((isCollidingWalls & (COLLIDING_NORTH | COLLIDING_SOUTH)) != 0)
            velocity_x = -velocity_x;
        if((isCollidingWalls & (COLLIDING_EAST | COLLIDING_WEST)) != 0)
            velocity_y = -velocity_y;

        pos_x += velocity_x;
        pos_y += velocity_y;
    } else {
        pos_x = temp_pos_x;
        pos_y = temp_pos_y;
    }

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_fillCircle(&g_sContext, pos_y, pos_x, circleRadius);

    TA0CTL &= ~TAIFG;

    P1OUT ^= redLED;
}
