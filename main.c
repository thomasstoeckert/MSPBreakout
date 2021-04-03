#include <msp430fr6989.h>
#include "Grlib/grlib/grlib.h"      // Graphics Library Import
#include "LcdDriver/lcd_driver.h"
#include "graphics.h"
#include "math_utils.h"
#include "levels.h"
#include <stdio.h>

Graphics_Context g_sContext;      // Declare our graphics context for the library
Graphics_Rectangle blocks[30] = {
                                {1, 1, 20, 8}, {22, 1, 42, 8}, {44, 1, 64, 8}, {66, 1, 86, 8}, {88, 1, 108, 8}, {110, 1, 127, 8},
                                {1, 10, 20, 18}, {22, 10, 42, 18}, {44, 10, 64, 18}, {66, 10, 86, 18}, {88, 10, 108, 18}, {110, 10, 127, 18},
                                {1, 20, 20, 28}, {22, 20, 42, 28}, {44, 20, 64, 28}, {66, 20, 86, 28}, {88, 20, 108, 28}, {110, 20, 127, 28},
                                {1, 30, 20, 38}, {22, 30, 42, 38}, {44, 30, 64, 38}, {66, 30, 86, 38}, {88, 30, 108, 38}, {110, 30, 127, 38},
                                {1, 40, 20, 48}, {22, 40, 42, 48}, {44, 40, 64, 48}, {66, 40, 86, 48}, {88, 40, 108, 48}, {110, 40, 127, 48}
};

const unsigned char numBlocks = 30;
const Graphics_Rectangle nullBlock = {
                                      128, 128, 128, 128
};

const int paddle_width = 20, paddle_height = 10, paddle_y = 120;
int paddle_x = 1;

#define redLED BIT0
#define circleRadius 2

int main(void)
{
    unsigned char i;

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

    // Draw our set of blocks
	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
	for(i = 0; i < numBlocks; i++) {
	    if(!IsNullBlock(&blocks[i]))
	        Graphics_fillRectangle(&g_sContext, &blocks[i]);
	}

	/*// Draw the initial paddle
	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
	Graphics_Rectangle initial_paddle = {paddle_x, paddle_y, paddle_x + paddle_width, paddle_y + paddle_height};
	Graphics_fillRectangle(&g_sContext, &initial_paddle);
    */

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
    unsigned char i;

    // Clear our old position
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillCircle(&g_sContext, pos_x, pos_y, circleRadius);

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

    // Check if circle is colliding with our blocks
    for(i = 0; i < numBlocks; i++) {
        if(IsNullBlock(&blocks[i])) continue;

        char isCollidingBlock = IsCollidingAABB(&blocks[i], &circleBounds);
        isCollidingWalls |= isCollidingBlock;

        if(isCollidingBlock) {
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_fillRectangle(&g_sContext, &blocks[i]);
            blocks[i] = nullBlock;
        }
    }

    // Check if paddle is colliding with blocks
    ///char isCollidingPaddle = IsCollidingAABB()

    // Our circle is colliding
    if(isCollidingWalls != 0) {
        // Find the colliding direction, flipping velocity
        if((isCollidingWalls & (COLLIDING_NORTH | COLLIDING_SOUTH)) != 0)
            velocity_x = -velocity_x;
        if((isCollidingWalls & (COLLIDING_EAST | COLLIDING_WEST)) != 0)
            velocity_y = -velocity_y;

        // Apply the new vectors
        pos_x += velocity_x;
        pos_y += velocity_y;
    } else {
        // These old calcs are good, use them.
        pos_x = temp_pos_x;
        pos_y = temp_pos_y;
    }

    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_fillCircle(&g_sContext, pos_x, pos_y, circleRadius);

    TA0CTL &= ~TAIFG;

    P1OUT ^= redLED;
}
