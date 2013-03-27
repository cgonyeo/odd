#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "portaudio.h"

#define PI 3.141592653
#define INCPORT 3490
#define OUTPORT 3491
#define MAXRCVLEN 500

#define NUM_LEDS 32
#define DEV "/dev/ttyUSB0"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define DITHER_FLAG (0)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

double totalTime, elapsedTime;
int done = 0;
int numAnimations = 0;
int timeLoops = 0;

//Used to represent a single LED
typedef struct {
	unsigned char R;
	unsigned char G;
	unsigned char B;
} odd_led_t;

//Holds the parameters for a single animation
typedef struct {
	void (*function)(double, double, double);
	void (*modifier)( void );
	double speed;
	double strength;
	double radius;
} animation_t;

typedef struct {
	int frameIndex;
	int maxFrameIndex;
	SAMPLE *recordedSamples;
} paTestData;

odd_led_t* leds[NUM_LEDS]; //All the LEDs in use
odd_led_t* tempLeds[NUM_LEDS]; //Current alterations to the LEDs, used with animations
animation_t* animations[50]; //All currently used animations.

//Writes leds to the file stream fp.
void write_odd(FILE* fp) {
	unsigned char end = 255;
	for(int i=0; i<NUM_LEDS; i++) {
		fwrite(&((leds[i]->R)), 1, 1, fp);
		fflush(fp);
	}
	fwrite(&end, 1, 1, fp);
}

//Writes the led array to the console
void write_console() {
	printf("\n");
	for(int i=0; i<NUM_LEDS; i++) {
		printf("%d, ", leds[i]->R);
		fflush(NULL);
	}
}

//Returns the remainder of dividend / divisor
double remainder(double dividend, int divisor) {
	int quotient = (int) dividend / divisor;
	return dividend - divisor * quotient;
}

//Returns the value of X raised to Y
//Y should be an integer. Ignore the fact that it isn't.
double pow(double x, double y)
{
	if(y > 0)
		return pow(x, y - 1) * x;
	else
		return 1.0;
}

//Returns a taylor series approximation for (sin((x - 0.5) * pi) + 1) / 2
//Put in a number between 0 and 1
//and it returns a number between 0 and 1
double sin(double x)
{
	x -= .5;
	x = PI * x - ( pow(PI, 3) * pow(x, 3) ) / 6 + ( pow(PI, 5) * pow(x, 5) ) / 120;
	x++;
	x /= 2;
	return x;
}

//Returns the time as a double (formatted to be smaller)
double formatTime(long int seconds, long int useconds)
{
	double time = seconds % 10000 + (useconds - useconds % 1000) / 1000000.0;
	
	if(time > 100000)
		time = remainder(time, 100000);
	
	return time;
}

//Inverts all channels on the temp LEDS.
//For example:
//254 -> 0
//0 -> 254
//180 -> 74
void invertTempLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = 254 - tempLeds[i]->R;
		tempLeds[i]->G = 254 - tempLeds[i]->G;
		tempLeds[i]->B = 254 - tempLeds[i]->B;
	}
}

//Adds the current modifications to the LEDs to the led array
void addLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i]->R + tempLeds[i]->R > 254)
			leds[i]->R = 254;
		else
			leds[i]->R += tempLeds[i]->R;
		
		if(leds[i]->G + tempLeds[i]->G > 254)
			leds[i]->G = 254;
		else
			leds[i]->G += tempLeds[i]->G;
		
		if(leds[i]->B + tempLeds[i]->B > 254)
			leds[i]->B = 254;
		else
			leds[i]->B += tempLeds[i]->B;
	}
}

//Subtracts the current modifications to the LEDs from the led array
void subtractLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i]->R - tempLeds[i]->R < 0)
			leds[i]->R = 0;
		else
			leds[i]->R -= tempLeds[i]->R;
		
		if(leds[i]->G - tempLeds[i]->G < 0)
			leds[i]->G = 0;
		else
			leds[i]->G -= tempLeds[i]->G;
		
		if(leds[i]->B - tempLeds[i]->B < 0)
			leds[i]->B = 0;
		else
			leds[i]->B -= tempLeds[i]->B;
	}
}

//Subtracts the inverse of tempLeds from leds
void inverseSubtractLeds()
{
	invertTempLeds();
	subtractLeds();
	invertTempLeds();
}

//Replaces the LEDs in the LED array for every value greater than 0.
void replaceLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(tempLeds[i]->R > 0 || tempLeds[i]->G > 0 || tempLeds[i]->B > 0)
		{
			leds[i]->R = tempLeds[i]->R;
			leds[i]->G = tempLeds[i]->G;
			leds[i]->B = tempLeds[i]->B;
		}
	}
}

//Resets all LEDs to 0
void resetLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		leds[i]->R = 0;
		leds[i]->G = 0;
		leds[i]->B = 0;
	}
}

//audio stuffs
static int recordCallback( const void *inputBuffer, void *outputBuffer,
							unsigned long framesPerBuffer,
							const PaStreamCallbackFlags statusFlags,
							void *userData )
{
	paTestData *data = (paTestData*)userData;
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[data->frameIndex * 2];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void) outputBuffer;
	//(void) timeInfo;
	(void) statusFlags;
	(void) userData;
	if(framesLeft < framesPerBuffer)
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}
	if(inputBuffer == NULL)
	{
		for(int i = 0; i < framesToCalc; i++)
		{
			*wptr++ = SAMPLE_SILENCE; //Left
			*wptr++ = SAMPLE_SILENCE; //Right
		}
	}
	else
	{
		for(int i = 0; i < framesToCalc; i++)
		{
			*wptr++ = *rptr++;
			*wptr++ = *rptr++;
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double speed, double strength, double radius) {
	if(strength < 0 || strength > 1)
		return;
	//scale the time by our speed to alter the rate of tf the animation
	double time = totalTime * speed;
	//double to keep track of the location of the center
	double center = 0.0;
	//we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
	int numLeds = NUM_LEDS - 1;
	//clear the tempLeds array so we can use it
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i]->R = 0;
	
	//Calculate the center
	if((int)time % 2 == 1)
		center = remainder(time, 1);
	else
		center = 1 - remainder(time, 1);
	center = sin(center);
	center *= numLeds;
	
	
	//Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
	double ledDistances[NUM_LEDS];
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(center - i > 0)
			ledDistances[i] = center - i;
		else
			ledDistances[i] = i - center;
		ledDistances[i] -= numLeds;
		ledDistances[i] *= -1;
		ledDistances[i] -= numLeds - radius;
		ledDistances[i] *= 1 / radius;
		ledDistances[i] *= 254 * strength;
		if(ledDistances[i] > 254)
			ledDistances[i] = 254;
	}
	//If an LED has a positive brightness, set it.
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
			tempLeds[i]->R = (unsigned char)ledDistances[i];
	}
}

//Animation: same as cylonEye but follows a linear movement (constant speed)
void cylonEye_Linear(double speed, double strength, double radius) {
	if(strength < 0 || strength > 1)
		return;
	//scale the time by our speed to alter the rate of tf the animation
	double time = totalTime * speed;
	//double to keep track of the location of the center
	double center = 0.0;
	//we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
	int numLeds = NUM_LEDS - 1;
	//clear the tempLeds array so we can use it
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i]->R = 0;
	
	//Calculate the center
	if((int)time % 2 == 1)
		center = remainder(time, 1);
	else
		center = 1 - remainder(time, 1);
	center *= numLeds;
	
	
	//Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
	double ledDistances[NUM_LEDS];
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(center - i > 0)
			ledDistances[i] = center - i;
		else
			ledDistances[i] = i - center;
		ledDistances[i] -= numLeds;
		ledDistances[i] *= -1;
		ledDistances[i] -= numLeds - radius;
		ledDistances[i] *= 1 / radius;
		ledDistances[i] *= 254 * strength;
		if(ledDistances[i] > 254)
			ledDistances[i] = 254;
	}
	//If an LED has a positive brightness, set it.
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
			tempLeds[i]->R = (unsigned char)ledDistances[i];
	}
}

//Animation: Turns the LEDs off and on and off and on and off and on and off...
void strobe(double speed, double strength, double radius)
{
	(void)radius;
	if(strength < 0 || strength > 1)
		return;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = 254 * strength;
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = (unsigned char)power;
	}
}

//Animation: Sets all LEDs to strength
void setAll(double speed, double strength, double radius)
{
	(void)radius;
	(void)speed;
	if(strength < 0 || strength > 1)
		return;
	double power = 254 * strength;
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i]->R = (unsigned char)power;
}

//Animation: Like strobe but fades LEDs in and out and pauses when they're off.
void smoothStrobe(double speed, double strength, double radius)
{
	(void)radius;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = remainder(time, 1);
	else
		power = 1 - remainder(time, 1);
	power = pow(power, 2);
	power *= 1.5;
	power -= 0.5;
	if(power < 0)
		power = 0;
	if(strength >= 0 && strength <= 1)
		power *= 254 * strength;
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i]->R = (unsigned char)power;
}

//Animation that makes the brightness of the LEDs follow a sin curve
void sinAnimation(double speed, double strength, double radius)
{
	(void)radius;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = remainder(time, 1);
	else
		power = 1 - remainder(time, 1);
	power = sin(power);
	if(strength >= 0 && strength <= 1)
		power *= 254 * strength;
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i]->R = (unsigned char)power;
}

//Adds an animation
void addAnimation( void (*function)(double, double, double), double speed, double strength, double radius, void (*modifier)( void ))
{
	animation_t* derp = malloc(sizeof(animation_t));
	derp->function = function;
	derp->speed = speed;
	derp->strength = strength;
	derp->radius = radius;
	derp->modifier = modifier;
	animations[numAnimations++] = derp;
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
	//printf("Thread started.\n");
	(void)arg;
	int failed = 0;
	//Open the stream, record if it fails
	int fd = open(DEV, O_WRONLY);
	if( fd == -1 ) {
		perror("open");
		failed = 1;
		//exit(EXIT_FAILURE);
	}
	FILE *fp;
	if(!failed)
		fp  = fdopen(fd, "w");
	//puts("SAVED");
	
	elapsedTime = 0;
	totalTime = 0;
	struct timeval start, current, previous;
	gettimeofday(&start, NULL);
	gettimeofday(&previous, NULL);
	for(int i = 0; i < NUM_LEDS; i++)
		leds[i]->R = 0;

	while(!done)
	{
		resetLeds();
		previous = current;
		gettimeofday(&current, NULL);
		elapsedTime =  formatTime(current.tv_sec, current.tv_usec) - formatTime(previous.tv_sec, previous.tv_usec);
		totalTime = formatTime(current.tv_sec, current.tv_usec) - formatTime(start.tv_sec, start.tv_usec);

		for(int i = 0; i < numAnimations; i++)
		{
			animations[i]->function(animations[i]->speed, animations[i]->strength, animations[i]->radius);
			animations[i]->modifier();
		}
				
		if(failed==0)
			write_odd(fp);
		//else
		//	write_console();
	}
	resetLeds();
	if(failed==0)
		write_odd(fp);
	//and we're done, close the stream.
	if(failed==0)
		fclose(fp);
	return NULL;
}

void networkListen(char *input)
{
	printf("Starting listen...\n");
	//char buffer[MAXRCVLEN + 1];
	int mysocket, newsockfd;
	socklen_t len;
	struct sockaddr_in server, cli_addr;
	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	printf("Socket made\n");
	
	if(mysocket == -1)
	{
		perror("socket");
		return;
	}
	//bzero( &server, sizeof(server));

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(INCPORT);

	if(bind(mysocket, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
	{
		perror("Socket");
		return;
	}
	printf("Bound to socket\n");

	//len = -1;
	//while(len < 0)
	//	len = recv(mysocket, input, MAXRCVLEN, 0);

	if( listen(mysocket, 5) < 0 )
	{
		perror("socket");
		return;
	}

	len = sizeof(cli_addr);
	printf("Listening on socket\n");
	newsockfd = accept(mysocket, (struct sockaddr *) &cli_addr, &len);
	printf("Accepted connection on port\n");
	if(newsockfd < 0)
	{
		perror("socket");
		return;
	}



	//if(len == -1)
	//{
	//	perror("socket");
	//	return;
	//}

	//input[len] = '\0';

	//printf("Received %s (%d bytes).\n", input, len);

	close(mysocket);
}

void flushInput()
{
	char c = 'f';
	while((c = getchar()) != '\n' && c != EOF);

}

void getUserInput(char *buffer)
{
	char ch;
	int char_count;
	
	ch = getchar();
	char_count = 0;
	while((ch != '\n') && char_count < 80) {
		buffer[char_count++] = ch;
		ch = getchar();
	}
	buffer[char_count] = 0x00;
}

void audioStuff()
{
	PaStreamParameters inputParameters, outputParameters;
	PaStream* stream;
	PaError err=paNoError;
	paTestData data;
	int i;
	int numSamples;
	int numBytes;
	double average;

	data.frameIndex = 0;
	numSamples = 441; //0.01 seconds of audio
	numBytes = numSamples * sizeof(SAMPLE);
	data.recordedSamples = (SAMPLE *) malloc( numBytes );
	if(data.recordedSamples == NULL)
		printf("Error allocating data.recordedSamples\n");
	for(int i = 0; i < numSamples; i++)
		data.recordedSamples[i] = 0;

	err = Pa_Initialize();
	if(err != paNoError)
		printf("Pa_Initialize() failed!");
	
	inputParameters.channelCount = 2;
	inputParameters.suggestedFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLateny = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,
		recordCallback,
		&data );
	if(err != paNoError)
		printf("Error opening stream");

	while(!done)
	{
		Pa_Sleep(100);
	}
	err = Pa_CloseStream( stream );
	if(err != paNoError)
		printf("Error closing stream");
}

int main(void)
{
	//Start the thread that updates our LEDs
	pthread_t ul;
	pthread_create(&ul,NULL,updateLoop,"randomargs");
	
	printf("ODD started.\n");
	
	for(int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = malloc(sizeof(odd_led_t));
		tempLeds[i] = malloc(sizeof(odd_led_t));
	}
	
	char input[80];
	while(!done)
	{
		printf("command > ");
		//scanf("%s", &input);
		getUserInput(input);
		//if(!strcmp(input, "network"))
		//	networkListen(input);
		if(!strcmp(input,"cyloneye") || !strcmp(input, "cyloneye -l") || !strcmp(input,"strobe") || !strcmp(input,"setall") || !strcmp(input, "smoothstrobe"))
		{
			double speed = -1, strength = -1, radius = -1;
			char modifier[20];
			modifier[0] = '\0';
			while(speed < 0) {
				printf("speed > ");
				scanf("%lf",&speed);
				flushInput();
			}
			while(strength < 0) {
				printf("strength > ");
				scanf("%lf",&strength);
				flushInput();
			}
			while(radius < 0) {
				printf("radius > ");
				scanf("%lf",&radius);
				flushInput();
			}
			while(strcmp(modifier,"add") && strcmp(modifier,"subtract") && strcmp(modifier,"replace") && strcmp(modifier,"inversesubtract")) {
				printf("modifier > ");
				getUserInput(modifier);
				//networkListen(modifier);
			}
			void(*animation_function)(double, double, double);
			void(*animation_modifier)( void ) = addLeds;
			if(!strcmp(input,"cyloneye"))
				animation_function = cylonEye;
			if(!strcmp(input, "cyloneye -l"))
				animation_function = cylonEye_Linear;
			if(!strcmp(input,"strobe"))
				animation_function = strobe;
			if(!strcmp(input,"setall"))
				animation_function = setAll;
			if(!strcmp(input,"smoothstrobe"))
				animation_function = smoothStrobe;
			if(!strcmp(modifier,"add"))
				animation_modifier = addLeds;
			if(!strcmp(modifier,"subtract"))
				animation_modifier = subtractLeds;
			if(!strcmp(modifier,"replace"))
				animation_modifier = replaceLeds;
			if(!strcmp(modifier,"inversesubtract"))
				animation_modifier = replaceLeds;
			
			addAnimation(animation_function,speed,strength,radius,animation_modifier); 
		}
		
		if(!strcmp(input,"exit"))
			done = 1;
		if(!strcmp(input,"remove"))
		{
			int index = -1;
			printf("index > ");
			scanf("%d", &index);
			flushInput();
			removeAnimation(index);
			
		}
	}
	printf("Exiting...\n");
	done = 1;
	pthread_join(ul, NULL);
	for(int i = 0; i < numAnimations; i++)
		free(animations[numAnimations]);
	for(int i = 0; i < NUM_LEDS; i++)
	{
		free(leds[i]);
		free(tempLeds[i]);
	}
}

