/*
 * graphics.h
 *
 *  Created on: Apr 3, 2021
 *      Author: Thoma
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "Grlib/grlib/grlib.h"      // Graphics Library Import
#include "LcdDriver/lcd_driver.h"
#include <msp430fr6989.h>

/*
 * Initialize the default settings for our graphics context and screen
 */
void Initialize_Graphics(Graphics_Context * context);

#endif /* GRAPHICS_H_ */
