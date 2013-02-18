odd
===

One Dimensional Display

This project aims to create a single row of RGB LEDs that can be directly controlled by a computer. The computer will be able to display single colors, basic hardcoded animations, dynamic animations (like audio visualization), and animations in reaction to various events (like someone entered a room, or the user reveived a message on Facebook).

This will be done by having an arduino control multiple TLC5940s that are powering the LEDs. The arduino will connect to a computer over USB, and receive updated states for the LEDs very rapidly, thus producing animations. The computer will be a linux computer (eventually a raspberry pi) running a service written in C.

The LEDs to come in discrete panels of 5 LEDs with one TLC5940, which will be daisy chainable and can thus be easily added or removed if more or less LEDs are desired.

Eventually both hardware buttons and a Web UI will be added as ways to control the LEDs.
