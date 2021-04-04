# MSPBreakout
By Thomas Stoeckert

The first One-Day Project of 2021

A simple clone of the classic ATARI breakout game, made for the MSP430FR6989 Microcontroller and the Educational Booster Pack MKII.

## How to Play:
Use the joystick on the booster pack to move left and right, bouncing the ball up into the field of bricks. Break all of them to move on to the next level.

If the ball falls off the south edge of the playing field, press the button (J4.32, Button 2 on the booster pack) to send it back out. You have three spare balls, but they are replenished every five levels.

## Future Goals:
- [ ] More than just a single level
   - Requires a level editor, because there's no way I'm doing that by hand
- [ ] Power-Ups
   - "Punching": Ball doesn't bounce off blocks, but blows through them
   - "Multi-Ball": Multiple balls. Would involve a big of the fixed update loop. Might bring the MSP to its knees.
   - "Longer Paddle": Makes the paddle longer. More control, easier play
- [ ] Sound effects?
   - Might want to experiment with that on the buzzer on the booster pack.

## Known Problems
* Ball Cuts Corners
  * My implementation of collision doesn't check for what's between the last frame and this one, so it ends up teleporting through the sides of blocks sometimes.
* Ball bounces unpredictably
  * Again, due to the shoddy collision detection, the ball will sometimes clip through one surface of a brick and then bounce off of another, inside it. This makes the bouncing a little unpredictable, which is unfun.