#ifndef ODD_ANIMATIONS
#define ODD_ANIMATIONS

int numParams(void (*function)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]));
void cylonEye(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void cylonEye_Linear(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void strobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void setAll(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void smoothStrobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void sinAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void volumeAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void dammitAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);

#endif
