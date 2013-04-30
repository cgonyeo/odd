#ifndef TLC5947_DRIVER
#define TLC5947_DRIVER

#define NUM_TLCS 1
void setLed(int ledIndex, int value);
void setAllLeds(int value);
void clearLeds();
int getLedValue(int index);
void pulsePin(int pin);
void tlc5947init();
void tlc5947cleanup();
void updateLeds();
#endif
