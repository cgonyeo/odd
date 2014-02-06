#ifndef TLC5947_DRIVER
#define TLC5947_DRIVER

#define NUM_TLCS 8
void tlcSetLed(int ledIndex, int value);
void tlcSetAllLeds(int value);
void tlcClearLeds(void);
int tlcGetLedValue(int index);
void tlcPulsePin(int pin);
void tlc5947init(void);
void tlc5947cleanup(void);
void tlcUpdateLeds(void);
#endif
