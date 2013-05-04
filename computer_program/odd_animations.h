#ifndef ODD_ANIMATIONS
#define ODD_ANIMATIONS

void cylonEye(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void cylonEye_Linear(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void strobe(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void setAll(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void smoothStrobe(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void sinAnimation(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void volumeAnimation(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
void dammitAnimation(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);

#endif
