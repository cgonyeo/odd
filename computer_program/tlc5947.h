#ifndef TLC5947_DRIVER
#define TLC5947_DRIVER

#define NUM_TLCS 6
void setLed(int ledIndex, int value);
void setAllLeds(int value);
void clearLeds(void);
int getLedValue(int index);
void pulsePin(int pin);
void tlc5947init(void);
void tlc5947cleanup(void);
void updateLeds(void);
#endif
