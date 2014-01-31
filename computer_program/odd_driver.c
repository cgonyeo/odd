#include "odd.h"

long double totalTime, elapsedTime;
int done = 0;
int numAnimations = 0;
int timeLoops = 0;

pthread_mutex_t ledsLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ledsTempLock = PTHREAD_MUTEX_INITIALIZER;

odd_led_t* leds[NUM_LEDS]; //All the LEDs in use
odd_led_t* tempLeds[NUM_LEDS]; //Current alterations to the LEDs, used with animations
animation_t* animations[50]; //All currently used animations.

//Returns the value of the color of led i in a thread-safe manner
int getLED(int i, char color)
{
    int returnValue;
    pthread_mutex_lock(&ledsLock);
    switch(color)
    {
        case 'R': case 'r': returnValue = leds[i]->R; break;
        case 'G': case 'g': returnValue = leds[i]->G; break;
        case 'B': case 'b': returnValue = leds[i]->B; break;
    }
    pthread_mutex_unlock(&ledsLock);
    return returnValue;
}

//Returns the value of the color of templed i in a thread-safe manner
int getTempLED(int i, char color)
{
    int returnValue;
    pthread_mutex_lock(&ledsTempLock);
    switch(color)
    {
        case 'R': case 'r': returnValue = tempLeds[i]->R; break;
        case 'G': case 'g': returnValue = tempLeds[i]->G; break;
        case 'B': case 'b': returnValue = tempLeds[i]->B; break;
    }
    pthread_mutex_unlock(&ledsTempLock);
    return returnValue;
}

//sets the color of led i to value in a thread-safe manner
void setLED(int i, char color, int value)
{
    pthread_mutex_lock(&ledsLock);
    switch(color)
    {
        case 'R': case 'r': leds[i]->R = value; break;
        case 'G': case 'g': leds[i]->G = value; break;
        case 'B': case 'b': leds[i]->B = value; break;
    }
    pthread_mutex_unlock(&ledsLock);
}

//sets the color of templed i to value in a thread-safe manner
void setTempLED(int i, char color, int value)
{
    pthread_mutex_lock(&ledsTempLock);
    switch(color)
    {
        case 'R': case 'r': tempLeds[i]->R = value; break;
        case 'G': case 'g': tempLeds[i]->G = value; break;
        case 'B': case 'b': tempLeds[i]->B = value; break;
    }
    pthread_mutex_unlock(&ledsTempLock);
}

//Writes the led array to the console
void write_console(void) {
	printf("\n");
	for(int i=0; i<NUM_LEDS; i++) {
		printf("%d, ", getLED(i, 'g'));
		fflush(NULL);
	}
}

//Sends the new LED values to the hardware
void write_odd(void) {
	for(int j = 0; j < NUM_TLCS; j++)
		for(int i = 0; i < 8; i++)
		{
			tlcSetLed(j * 24 + i*3,   getLED(j * 8 + i, 'r'));
			tlcSetLed(j * 24 + i*3+1, getLED(j * 8 + i, 'g'));
			tlcSetLed(j * 24 + i*3+2, getLED(j * 8 + i, 'b'));
		}
	tlcUpdateLeds();
}

//Resets all LEDs to 0
void resetLeds(void)
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
        setLED(i, 'r', 0);
        setLED(i, 'g', 0);
        setLED(i, 'b', 0);
	}
}

//Adds an animation
void addAnimation( void (*function)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]), double* params, odd_led_t* color, void (*modifier)( odd_led_t* leds[NUM_LEDS], odd_led_t *[NUM_LEDS] ))
{
	animation_t* derp;
	if ((derp = malloc(sizeof(animation_t))) == NULL) {
		fprintf(stderr, "Malloc failed");
		exit(1);
	}
	derp->function = function;
	derp->params = params;
	derp->color = color;
	derp->modifier = modifier;
	animations[numAnimations++] = derp;
}

//Updates an animation
void updateAnimation( int index, double* params, odd_led_t* color)
{
	animations[index]->params = params;
	odd_led_t *temp = animations[index]->color;
	animations[index]->color = color;
	free(temp);
}

//Removes an animation
void removeAnimation(int index)
{
	if(index > numAnimations || index < 0)
		return;
	for(int i = index; i < numAnimations - 1; i++)
		animations[i] = animations[i+1];
	animations[numAnimations - 1] = NULL;
	numAnimations--;
}

//Program's update loop
void *updateLoop(void *arg) {
	(void)arg;
	int failed = 0;
	
	elapsedTime = 0;
	totalTime = 0;
	struct timeval start, current, previous;
	gettimeofday(&start, NULL);
	gettimeofday(&previous, NULL);

	while(!done)
	{
		resetLeds();
		previous = current;
		gettimeofday(&current, NULL);
		elapsedTime =  formatTime(current.tv_sec, current.tv_usec) - formatTime(previous.tv_sec, previous.tv_usec);
		totalTime = formatTime(current.tv_sec, current.tv_usec);// - formatTime(start.tv_sec, start.tv_usec);

		for(int i = 0; i < numAnimations; i++)
		{
			animations[i]->function(animations[i]->params, totalTime, animations[i]->color, tempLeds);
			animations[i]->modifier(leds, tempLeds);
		}
				
		if(failed==0)
			write_odd();

		usleep(500);
	}
	resetLeds();
	if(failed==0)
		write_odd();
	return NULL;
}

int main(void)
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = malloc(sizeof(odd_led_t));
		tempLeds[i] = malloc(sizeof(odd_led_t));
	}

	tlc5947init();
    audioInitialization();

	//Start the thread that updates our LEDs
	pthread_t ul;
	pthread_create(&ul,NULL,updateLoop,"randomargs");

	printf("ODD started.\n");
	
	char input[255];
	input[0] = '\0';

	odd_led_t* color = malloc(sizeof(odd_led_t));
	color->R = 0;
	color->G = 40;
	color->B = 0;
	
	double params[2];
	params[0] = 0.5;
	params[1] = 15;

	addAnimation(cylonEye, params, color, addLeds);

	odd_led_t* color2 = malloc(sizeof(odd_led_t));
	color2->R = 0;
	color2->G = 0;
	color2->B = 40;
	
	double params2[2];
	params2[0] = 0.45;
	params2[1] = 15;

	addAnimation(cylonEye, params2, color2, addLeds);

	odd_led_t* color3 = malloc(sizeof(odd_led_t));
	color3->R = 40;
	color3->G = 0;
	color3->B = 0;
	
	double params3[2];
	params3[0] = 0.4;
	params3[1] = 15;

	addAnimation(cylonEye, params3, color3, addLeds);

	networkListen(input);
    sleep(10000);
	printf("Exiting...\n");
	done = 1;
	pthread_join(ul, NULL);
	tlc5947cleanup();
	for(int i = 0; i < numAnimations; i++)
	{
		//free(animations[numAnimations]->color);
		free(animations[numAnimations]);
	}
	for(int i = 0; i < NUM_LEDS; i++)
	{
		free(leds[i]);
		free(tempLeds[i]);
	}
}

