odd
===

One Dimensional Display

This project aims to create a single row of RGB LEDs that can be individually controlled by a Raspberry Pi. The Pi will be able to display single colors, basic hardcoded animations, dynamic animations (like audio visualization), and animations in reaction to various events (like someone entered a room, or the user reveived a message on Facebook).

The hardware will consist of packages of one TLC5947, five RGB LEDs, one voltage regulator, and some capacitors and resistors on a PCB. These circuit boards will be daisy chainable, with an end goal of about 30 boards (240 LEDs).

The hardware will be controlled in roughly the following fashion:

HTML/CSS/Javascript ---> Python ---> C ---> TLC5947 ---> LEDs

The Javascript uses Websockets (you need an up-to-date browser to do this) to communicate with a Python server, which uses normal sockets to parrot messages to and from the C program. The C program calculates the various animations and tracks each LED's color. It then uses the GPIO pins available on the Raspberry Pi to control the TLC5947s, which power the LEDs.

Here's an imcomplete parts list:
 - Raspberry Pi
 - TLC5947 (LED Driver)
 - RGB LEDs (8x the # of TLCs)
 - One of the PCBs from the eagle board folder for each TLC
 - Resistors & capacitors
 - LM3940 (Voltage Regulator)
