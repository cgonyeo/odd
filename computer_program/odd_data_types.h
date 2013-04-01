#ifndef ODD_DATA_TYPES
#define ODD_DATA_TYPES

#define NUM_LEDS 5

//Used to represent a single LED
typedef struct {
	unsigned char R;
	unsigned char G;
	unsigned char B;
} odd_led_t;

//Holds the parameters for a single animation
typedef struct {
	void (*function)(double, double, double, odd_led_t*, odd_led_t *[NUM_LEDS]);
	void (*modifier)( odd_led_t *[NUM_LEDS], odd_led_t *[NUM_LEDS] );
	double speed;
	double radius;
	odd_led_t* color;
} animation_t;

/*typedef struct {
	int frameIndex;
	int maxFrameIndex;
	SAMPLE *recordedSamples;
} paTestData;*/

#endif
