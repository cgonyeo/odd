#ifndef ODD_ANIM_MODIFIERS
#define ODD_ANIM_MODIFIERS

void invertTempLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void addLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void subtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void inverseSubtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void multiplyLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);

#endif
