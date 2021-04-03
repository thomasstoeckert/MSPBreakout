/*
 * graphics.c
 *
 *  Created on: Apr 3, 2021
 *      Author: Thoma
 */

#include "graphics.h"

void Initialize_Graphics(Graphics_Context * context) {
    // Set our screen orientation
    Crystalfontz128x128_SetOrientation(0);

    // Initialize our graphics context
    Graphics_initContext(context, &g_sCrystalfontz128x128);

    // Set our default background and foreground colors
    Graphics_setBackgroundColor(context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(context, GRAPHICS_COLOR_WHITE);

    // Establish our default font
    GrContextFontSet(context, &g_sFontFixed6x8);

    // Clear the screen
    Graphics_clearDisplay(context);

    // Set the display's backlight to high
    P2DIR |= BIT6;
    P2OUT |= BIT6;
}
