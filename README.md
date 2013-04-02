odd
===

One Dimensional Display

This project aims to create a single row of RGB LEDs that can be directly controlled by a computer. The computer will be able to display single colors, basic hardcoded animations, dynamic animations (like audio visualization), and animations in reaction to various events (like someone entered a room, or the user reveived a message on Facebook).

The hardware will consist of one TLC5940NT, five RGB LEDs, one voltage regulator, and some capacitors and resistors on a PCB. These circuit boardswill be daisy chainable, with an end goal of about 50 boards (150 LEDs).

The hardware will be controlled in roughly the following fashion:

HTML/CSS/Javascript ---> Python ---> C ---> Arduino ---> TLC5940NT ---> LEDs

The Javascript uses Websockets (you need an up-to-date browser to do this) to communicate with a Python server, which uses normal sockets to parrot messages to and from the C program. The C program calculates the various animations and tracks each LED's color. It passes updated "frames" over serial to an Arduino, and the Arduino controls the TLC5940NTs which power the LEDs.

Here's an imcomplete parts list:
 - Arduino
 - TLC5940NT (LED Driver)
 - RGB LEDs (5x the # of TLCs)
 - One of the PCBs from the eagle board folder for each TLC
 - Resistors & capacitors
 - LM3940 (Voltage Regulator)
