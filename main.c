#include <msp430fr6989.h>
#include "Grlib/grlib/grlib.h"      // Graphics Library Import
#include "LcdDriver/lcd_driver.h"
#include "graphics.h"
#include "math_utils.h"
#include "levels.h"
#include <stdio.h>

Graphics_Context g_sContext;      // Declare our graphics context for the library
Graphics_Rectangle blocks[30];

const unsigned char numBlocks = 30;
const Graphics_Rectangle nullBlock = { 128, 128, 128, 128 };

unsigned int paddle_x = 1;

int lives = 3;
unsigned int points = 0;
unsigned char activeBlocks = 30;

#define redLED BIT0
#define JOY_X BIT2
#define BUTTON BIT1
#define circleRadius 2

void initializeADC(void);
int readADC(unsigned int*data);

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

	// Configure Button Input
	P3DIR &= ~BUTTON;
	P3REN |=  BUTTON;
	P3OUT |=  BUTTON;
	P3IES |=  BUTTON;

	initializeADC();

	/////////////////////////////////////////////////////
	//           Initialize graphics library           //
	/////////////////////////////////////////////////////
	Crystalfontz128x128_Init();       // Initialize our display communication

	Initialize_Graphics(&g_sContext); // Prepare the display with initial commands

	// Load our level
	memcpy(blocks, LEVEL_1, sizeof(Graphics_Rectangle) * 30);

	Draw_Playspace(&g_sContext, blocks, numBlocks, lives, 1);

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
    static char isFree = 0;
    unsigned char i;

    // Clear interrupt flag
    TA0CTL &= ~TAIFG;

    // Set light
    P1OUT ^= redLED;

    // Paddle Logic
    Graphics_Rectangle paddle_rect = {
                                      paddle_x, PADDLE_Y, paddle_x + PADDLE_WIDTH, PADDLE_Y + PADDLE_HEIGHT
    };

    Draw_Paddle(&g_sContext, paddle_x, GRAPHICS_COLOR_BLACK);

    readADC(&paddle_x);
    paddle_x = (paddle_x / 4095.0) * 128 - PADDLE_WIDTH;

    paddle_rect.xMin = paddle_x;
    paddle_rect.xMax = paddle_x + PADDLE_WIDTH;

    if(isFree) {
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

        if((isCollidingWalls & COLLIDING_SOUTH) != 0) {
            // We've hit the bottom of the map! Set our next frame to be locked to the paddle,
            // take a life
            isFree = 0;

            lives--;
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_fillCircle(&g_sContext, (lives*6)+4, 123, circleRadius);

            // Check for Loss Condition
            if(lives < 0) {
                // We're dead. Stop the game, and establish the reset button's interrupt
                Draw_ModalBox(&g_sContext, "Game Over!", "Play Again?", 1);

                // Turn off the timer
                TA0CTL &= ~MC_3;

                // Enable Interrupt
                P3IE  |=  BUTTON;
                P3IFG &= ~BUTTON;

                // Break out of this
                return;
            }
        }

        // Check if circle is colliding with our blocks
        for(i = 0; i < numBlocks; i++) {
            if(IsNullBlock(&blocks[i])) continue;

            char isCollidingBlock = IsCollidingAABB(&blocks[i], &circleBounds);
            isCollidingWalls |= isCollidingBlock;

            if(isCollidingBlock) {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
                Graphics_fillRectangle(&g_sContext, &blocks[i]);
                blocks[i] = nullBlock;
                activeBlocks--;

                points++;

                // Check for Win Condition
                if(activeBlocks == 0) {
                    // We won! Stop the game, and establish the reset button's interrupt
                    Draw_ModalBox(&g_sContext, "You Win!", "Continue?", 1);

                    // Capture the ball
                    isFree = 0;

                    // Turn off the timer
                    TA0CTL &= ~MC_3;

                    // Enable Interrupt
                    P3IE  |=  BUTTON;
                    P3IFG &= ~BUTTON;

                    // Break out of this
                    return;
                }
            }
        }

        char isCollidingPaddle = IsCollidingAABB(&circleBounds, &paddle_rect);
        isCollidingWalls |= isCollidingPaddle;

        // Clear our old position
        Draw_Ball(&g_sContext, pos_x, pos_y, GRAPHICS_COLOR_BLACK);

        // Our circle is colliding
        if(isCollidingWalls != 0) {
            // Find the colliding direction, flipping velocity
            if((isCollidingWalls & (COLLIDING_EAST | COLLIDING_WEST)) != 0)
                velocity_x = -velocity_x;
            if((isCollidingWalls & (COLLIDING_NORTH | COLLIDING_SOUTH)) != 0)
                velocity_y = -velocity_y;

            // Apply the new vectors
            pos_x += velocity_x;
            pos_y += velocity_y;
        } else {
            // These old calcs are good, use them.
            pos_x = temp_pos_x;
            pos_y = temp_pos_y;
        }
    } else {
        // We're not free - we're to be locked to the top of the paddle

        // Clear our old position
        Draw_Ball(&g_sContext, pos_x, pos_y, GRAPHICS_COLOR_BLACK);

        // Place us above the paddle
        pos_x = paddle_x + PADDLE_WIDTH / 2;
        pos_y = PADDLE_Y - 4;

        // If the user has the release button down
        if((P3IN & BUTTON) == 0) {
            // Set the ball free
            isFree = 1;
        }
    }

    Draw_Paddle(&g_sContext, paddle_x, GRAPHICS_COLOR_RED);

    // Redrawing of Circle
    if(lives >= 0) {
        Draw_Ball(&g_sContext, pos_x, pos_y, GRAPHICS_COLOR_WHITE);
    }
}

#pragma vector = PORT3_VECTOR
__interrupt void RESET_ISR() {
    // Clear flag, disable interrupt
    P3IFG &= ~BUTTON;
    P3IE  &= ~BUTTON;

    // Reset points if loss
    if(lives < 0) points = 0;

    // Set lives to max
    lives = 3;

    // Rest our active block count
    activeBlocks = 30;

    // Clear our screen, reset everything to zero.
    memcpy(blocks, LEVEL_1, sizeof(Graphics_Rectangle) * 30);

    // Draw the original screen
    Draw_Playspace(&g_sContext, blocks, numBlocks, lives, 1);

    // Begin play loop
    TA0CTL |= MC_1;
}

void initializeADC(void) {
    // Divert pins for analong functionality
    P9SEL1 |= JOY_X;
    P9SEL0 |= JOY_X;

    // Turn on the ADC module
    ADC12CTL0 |= ADC12ON;

    // Turn off the ENC (Enable Conversion) bit while modifying the config
    ADC12CTL0 &= ~ADC12ENC;

    // Set ADC12SHT0 (Determined number of cycles)
    ADC12CTL0 |= BIT8 | BIT9;

    // *********** ADC12CTL1 ******** //
    // Set ADC12SHS (select ADC12SC bit as the trigger)
    // Set ADC12SHP bit
    // Set ADC12DIV (select the divider you determined)
    // Set ADC12SSEL (select MODOSC)
    // Looks like only bit 9 needs to be set high.
    ADC12CTL1 = BIT9;

    // *********** ADC12CTL2 ******** //
    // Set ADC12RES (select 12-bit resolution)
    // [5:4] 10b
    // Set ADC12DF  (select unsigned binary format)
    // [3] 0b
    ADC12CTL2 = BIT5;

    // *********** ADC12CTL3 ******** //
    // Leave all fields default

    // *********** ADC12MCTL0 ******** //
    // Set ADC12VRSEL (select VR+=AVCC, VR-=AVSS)
    // [11:8] 0000b
    // Set ADC12INCH  (select channel A10)
    // [4:0] 01010b;
    ADC12MCTL0 = BIT3 | BIT1;

    // Turn on ENC to signal the end of configuration
    ADC12CTL0 |= ADC12ENC;

    return;
}

int readADC(unsigned int* data) {
    // Set ADC12SC bit
    ADC12CTL0 |= BIT0;
    // Wait for ADC12Busy bit to clear
    while((ADC12CTL1 & BIT0) != 0) {}

    // Read result from register ADC12MEM0
    (*data) = ADC12MEM0;

    return 1;
}
