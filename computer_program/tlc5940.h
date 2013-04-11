#ifndef TLC5940_DRIVER
#define TLC5940_DRIVER

#define NUM_TLCS 1
void setLed(int ledIndex, int value);
void setAllLeds(int value);
void clearLeds();
int getLedValue(int index);
void pulsePin(int pin);
void tlc5940init();
void tlc5940cleanup();
void updateLeds();
#endif
