#ifndef ODD
#define ODD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
//hardware
#include "tlc5947.h"
//audio
#include <jack/jack.h>
#include <fftw3.h>
//threads
#include <pthread.h>
//networking
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <jansson.h>

#define NUM_LEDS NUM_TLCS * 8
#define PI 3.141592653

//Data Types
typedef struct {
	int R;
	int G;
	int B;
} odd_led_t;

typedef void (*animation_f)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]);
typedef void (*modifier_f)( odd_led_t *[NUM_LEDS], odd_led_t *[NUM_LEDS] );

typedef struct animation_type {
	animation_f function;
	modifier_f modifier;
	double* params;
	odd_led_t* color;
    struct animation_type *next;
} animation_t;

//Driver Stuff
#define INCPORT 10001
#define MAXRCVLEN 500
#define LISTENQ (1024)

int getLED(int i, char color);
int getTempLED(int i, char color);
void setLED(int i, char color, int value);
void setTempLED(int i, char color, int value);

//Networking
void *networkListen(char *buffer);
void *handleConnection(void *num);

//Animation Functions
typedef struct Animation {
    const char *name;
    void (*function)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]);
    int numParams;
} Animation;

#define ANIMATION(name, num) void name(double *params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]);
#include "animations.def"
#undef ANIMATION

//Animation Modifier Functions
void invertTempLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void addLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void subtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void inverseSubtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);
void multiplyLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS]);

//Math Functions. No, I didn't need to write some of these. Shut up.
double odd_remainder(double dividend, int divisor);
double odd_pow(double x, double y);
double odd_sin(double x);
double formatTime(long int seconds, long int useconds);

//Audio related stuff
#define BUFSIZE (4096)
typedef float SAMPLE;

#define FFT_INPUT_SIZE (4096)
#define FFT_OUTPUT_SIZE (4096)
#define SAMPLE_RATE (44100)

void getSoundBuffer(SAMPLE* buf);
void runFFT(SAMPLE* buf);
int audioInitialization();

//Socket stuff
#define INCPORT 10001
#define MAXRCVLEN 500
#define LISTENQ (1024)

//Device stuff
#define DEV "/dev/ttyUSB0"

#endif