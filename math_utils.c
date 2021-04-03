/*
 * math_utils.c
 *
 *  Created on: Apr 3, 2021
 *      Author: Thoma
 */

#include "math_utils.h"

char IsCollidingAABB(Graphics_Rectangle r1, Graphics_Rectangle r2) {
    // Returns 0 if not colliding
    // If it is colliding, it'll return a char with bits set for what direction it is colliding in

    char result = 0;
    /*
    if(r1.minX < r2.maxX) result &= BIT0;
    if(r1.minY < r2.maxY) result &= BIT1;
    */
    return result;
}

/*
 * Returns a char of bitflags representing the collision status of the
 *  rectangle with respect to the screen's walls. If no collision, the
 *  result is zero. Otherwise, bitwise-and the result to find the collision
 *  direction
 */
char IsCollidingWalls(Graphics_Rectangle *ball) {
    char result = 0;

    if(ball->xMin < 0)   result |= BIT0;
    if(ball->yMax > 127) result |= BIT1;
    if(ball->xMax > 127) result |= BIT2;
    if(ball->yMin < 0)   result |= BIT3;

    return result;
}
