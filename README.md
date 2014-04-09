odd
===

One Dimensional Display

http://blog.gonyeo.com/category/odd.html

This project aims to create a single row of RGB LEDs that can be individually 
controlled by a Raspberry Pi. The Pi will be able to display single colors, 
basic hardcoded animations, dynamic animations (like audio visualization), and 
animations in reaction to various events (like someone entered a room, or the 
user reveived a message on Facebook).

The hardware will consist of packages of one TLC5947, eight RGB LEDs, one 
voltage regulator, and some capacitors and resistors on a PCB. These circuit 
boards will be daisy chainable, with an end goal of about 30 boards (240 LEDs).

The hardware will be controlled in roughly the following fashion:

HTML/CSS/Javascript ---> C ---> TLC5947 ---> LEDs

Here's an imcomplete parts list:
 - Raspberry Pi
 - TLC5947 (LED Driver)
 - RGB LEDs (8x the # of TLCs)
 - One of the PCBs from the eagle board folder for each TLC
 - Resistors & capacitors
 - LM3940 (Voltage Regulator)
