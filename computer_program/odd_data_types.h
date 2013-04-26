#include "tlc5940.h"

#ifndef ODD_DATA_TYPES
#define ODD_DATA_TYPES

#define NUM_LEDS ((NUM_TLCS * 24) / 3)

//Used to represent a single LED
typedef struct {
	int R;
	int G;
	int B;
} odd_led_t;

//Holds the parameters for a single animation
typedef struct {
	void (*function)(double, double, double, odd_led_t*, odd_led_t *[NUM_LEDS]);
	void (*modifier)( odd_led_t *[NUM_LEDS], odd_led_t *[NUM_LEDS] );
	double speed;
	double radius;
	odd_led_t* color;
} animation_t;

#endif
