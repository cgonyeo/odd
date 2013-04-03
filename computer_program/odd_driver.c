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
#include "odd_data_types.h"
#include "odd_math.h"
#include "odd_animations.h"
#include "odd_anim_modifiers.h"

#define INCPORT 3490
#define OUTPORT 3491
#define MAXRCVLEN 500
#define LISTENQ (1024)

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

long double totalTime, elapsedTime;
int done = 0;
int numAnimations = 0;
int timeLoops = 0;
/*typedef struct {
	int frameIndex;
	int maxFrameIndex;
	SAMPLE *recordedSamples;
} paTestData;*/

odd_led_t* leds[NUM_LEDS]; //All the LEDs in use
odd_led_t* tempLeds[NUM_LEDS]; //Current alterations to the LEDs, used with animations
animation_t* animations[50]; //All currently used animations.

//Writes leds to the file stream fp.
void write_odd(FILE* fp) {
	unsigned char end = 255;
	for(int i=0; i<NUM_LEDS; i++) {
		fwrite(&((leds[i]->R)), 1, 1, fp);
		fflush(fp);
		fwrite(&((leds[i]->G)), 1, 1, fp);
		fflush(fp);
		fwrite(&((leds[i]->B)), 1, 1, fp);
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
/*
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
}*/

//Adds an animation
void addAnimation( void (*function)(double, double, double, odd_led_t*, odd_led_t *[NUM_LEDS]), double speed, double radius, odd_led_t* color, void (*modifier)( odd_led_t* leds[NUM_LEDS], odd_led_t *[NUM_LEDS] ))
{
	animation_t* derp = malloc(sizeof(animation_t));
	derp->function = function;
	derp->speed = speed;
	derp->radius = radius;
	derp->color = color;
	derp->modifier = modifier;
	animations[numAnimations++] = derp;
}

//Adds an animation
void updateAnimation( int index, double speed, double radius, odd_led_t* color)
{
	animations[index]->speed = speed;
	animations[index]->radius = radius;
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
	//Open the stream, exit if it fails
	int fd = open(DEV, O_WRONLY);
	if( fd == -1 ) {
		perror("open");
		failed = 1;
		exit(EXIT_FAILURE);
	}
	FILE *fp;
	if(!failed)
		fp  = fdopen(fd, "w");
	
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
			animations[i]->function(animations[i]->speed, animations[i]->radius, totalTime, animations[i]->color, tempLeds);
			animations[i]->modifier(leds, tempLeds);
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
/*
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
}*/

void *networkListen(char *buffer)
{
	int list_s;		//Listening socket
	int conn_s;		//connection socket
	short int port = 3357;	//port number
	struct sockaddr_in servaddr;	//socket address struct
	char* tok;		//To split off tokens from input
	printf("Starting socket");
	if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		printf("Error making listening socket\n");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	printf("Binding to socket\n");
	if(bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		printf("Error calling bind\n");
		exit(EXIT_FAILURE);
	}
	printf("Setting socket to listen\n");
	if(listen(list_s, LISTENQ) < 0)
	{
		printf("Error calling listen\n");
		exit(EXIT_FAILURE);
	}
	printf("Accepting on socket\n");
	if((conn_s = accept(list_s, NULL, NULL)) < 0)
	{
		printf("Error calling accept\n");
		exit(EXIT_FAILURE);
	}
	while(!done)
	{
		printf("Connection made\n");
		if(read(conn_s, buffer, 255) < 0)
		{
			printf("Error reading\n");
			exit(EXIT_FAILURE);
		}
		printf("Received: %s\n", buffer);
		
		double speed = -1, radius = -1;
		int r = -1, g = -1, b = -1;

		char* temp = buffer;
		char* line = buffer;
		/*char* temp = strchr(buffer, '!');
		*temp++ = '\0';
		char* line = buffer;

		line = strtok(buffer,"!");*/

		line = temp;
		temp = strchr(buffer, '!');
		if(temp != NULL)
			*temp++ = '\0';

		while(line != NULL && strcmp(line,""))
		{
			printf("Line: '%s'\n", line);
			tok = strchr(line, ' ');
			*tok++ = '\0';
			printf("Tok: %s\n", tok);
			if(!strcmp(line,"exit"))
			{
				done = 1;
				printf("Closing the connection\n");

				if(close(conn_s) < 0)
				{
					printf("Error closing\n");
					exit(EXIT_FAILURE);
				}
			}
			else if(!strcmp(line,"remove"))
			{
				char* tmp = strchr(tok, ' ');
				*tmp = '\0';
				removeAnimation(atoi(tok));
			}
			else if(!strcmp(line,"time"))
			{
				printf("Time: %Lf\n", totalTime);
			}
			else if(!strcmp(line,"stop"))
			{
				for(int i = numAnimations; i > 0; i--)
					removeAnimation(0);
			}
			else if(!strcmp(line,"sup?"))
			{
				char *reply;
				char *temp;
				char *temp2;
				if(numAnimations != 0)
					for(int i = 0; i < numAnimations; i++)
					{
						if(animations[i]->function == setAll)
							temp = "setall";
						else if(animations[i]->function == strobe)
							temp = "strobe";
						else if(animations[i]->function == smoothStrobe)
							temp = "smoothstrobe";
						else if(animations[i]->function == cylonEye)
							temp = "cyloneye";
						else if(animations[i]->function == sinAnimation)
							temp = "sinanimation";

						if(animations[i]->modifier == addLeds)
							temp2 = "add";
						if(animations[i]->modifier == subtractLeds)
							temp2 = "subtract";
						if(animations[i]->modifier == multiplyLeds)
							temp2 = "multiply";

						snprintf(reply, sizeof(reply), "%s%s %f %f %i %i %i %s !", reply, temp, animations[i]->speed, animations[i]->radius, animations[i]->color->R, animations[i]->color->G, animations[i]->color->B, temp2);
					}
				else
					strcpy(reply,"Not much, you?");
					/*if(write(conn_s, reply, strlen(reply)) < 0)
					{
						printf("Error writing\n");
						exit(EXIT_FAILURE);
					}*/
			}
			else
			{
				char* animName = line;

				char* speedc = tok;//strchr(animName,' ');
				//*speedc++ = '\0';
				
				char* radiusc = strchr(speedc, ' ');
				*radiusc++ = '\0';
				
				char* rc = strchr(radiusc, ' ');
				*rc++ = '\0';
				
				char* gc = strchr(rc, ' ');
				*gc++ = '\0';
				
				char* bc = strchr(gc, ' ');
				*bc++ = '\0';
				
				char* modifier = strchr(bc, ' ');
				*modifier++ = '\0';

				char* endSpace = strchr(modifier, ' ');
				*endSpace++ = '\0';

				speed = strtod(speedc, NULL);
				radius = strtod(radiusc,NULL);
				r = atoi(rc);
				g = atoi(gc);
				b = atoi(bc);

				if(speed < 0)
					speed = 0;
				if(radius < 0)
					radius = 0;

				if(r > 254)
					r = 254;
				if(b > 254)
					b = 254;
				if(g > 254)
					g = 254;
				if(r < 0)
					r = 0;
				if(b < 0)
					b = 0;
				if(g < 0)
					g = 0;
				
				void(*animation_function)(double, double, double, odd_led_t*, odd_led_t *[NUM_LEDS]) = NULL;
				void(*animation_modifier)( odd_led_t* leds[NUM_LEDS], odd_led_t *[NUM_LEDS] ) = NULL;
				if(!strcmp(animName,"cyloneye"))
					animation_function = cylonEye;
				if(!strcmp(animName, "cyloneye -l"))
					animation_function = cylonEye_Linear;
				if(!strcmp(animName,"strobe"))
					animation_function = strobe;
				if(!strcmp(animName,"setall"))
					animation_function = setAll;
				if(!strcmp(animName,"smoothstrobe"))
					animation_function = smoothStrobe;
				if(!strcmp(animName,"sin"))
					animation_function = sinAnimation;
				if(!strcmp(modifier,"add"))
					animation_modifier = addLeds;
				if(!strcmp(modifier,"subtract"))
					animation_modifier = subtractLeds;
				if(!strcmp(modifier,"inversesubtract"))
					animation_modifier = inverseSubtractLeds;
				if(!strcmp(modifier,"multiply"))
					animation_modifier = multiplyLeds;

				odd_led_t* color = malloc(sizeof(odd_led_t));
				color->R = (unsigned char)r;
				color->G = (unsigned char)g;
				color->B = (unsigned char)b;
			
				printf("Animation: '%s'\n",animName);
				printf("Speed: '%f'\n",speed);
				printf("Radius: '%f'\n",radius);
				printf("R: '%i'\n",color->R);
				printf("G: '%i'\n",color->G);
				printf("B: '%i'\n",color->B);
				printf("Modifier: '%s'\n",modifier);

				if (!strcmp(animName, "update"))
					updateAnimation(atoi(modifier), speed, radius, color);
				else if(animation_function == NULL || animation_modifier == NULL)
					printf("Bad input?!\n");
				else
					addAnimation(animation_function,speed,radius,color,animation_modifier);
				

			}
			/*printf("Current line: %s\n", line);
			printf("Buffer: %s\n", buffer);
			line = strtok(NULL,"!");
			printf("New line: %s\n", line);
			printf("Buffer: %s\n", buffer);*/
			line = temp;
			printf("Line processing done.\n1: '%s'\n",line);
			temp = strchr(buffer, '!');
			if(temp != NULL)
				*temp++ = '\0';
			printf("2: '%s'\n",line);
		}
	}
	
	if(close(list_s) < 0)
	{
		printf("Error closing list_s\n");
		exit(EXIT_FAILURE);
	}
	return NULL;
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
	
	char input[255];
	input[0] = '\0';
	while(!done)
	{
		networkListen(input);
		/*
		printf("command > ");
		getUserInput(input);
		//if(!strcmp(input, "network"))
		//	networkListen(input);
		if(!strcmp(input,"cyloneye") || !strcmp(input, "cyloneye -l") || !strcmp(input,"strobe") || !strcmp(input,"setall") || !strcmp(input, "smoothstrobe") || !strcmp(input,"sin"))
		{
			double speed = -1, radius = -1;
			int r = -1, g = -1, b = -1;
			char modifier[20];
			modifier[0] = '\0';
			while(speed < 0) {
				printf("speed > ");
				scanf("%lf",&speed);
				flushInput();
			}
			while(radius < 0) {
				printf("radius > ");
				scanf("%lf",&radius);
				flushInput();
			}
			while(r < 0 || r > 254) {
				printf("R > ");
				scanf("%u",&r);
				flushInput();
			}
			while(g < 0 || g > 254) {
				printf("G > ");
				scanf("%u",&g);
				flushInput();
			}
			while(b < 0 || b > 254) {
				printf("B > ");
				scanf("%u",&b);
				flushInput();
			}
			while(strcmp(modifier,"add") && strcmp(modifier,"subtract") && strcmp(modifier,"inversesubtract") && strcmp(modifier,"multiply")) {
				printf("modifier > ");
				getUserInput(modifier);
			}
			void(*animation_function)(double, double, double, odd_led_t*, odd_led_t *[NUM_LEDS]);
			void(*animation_modifier)( odd_led_t* leds[NUM_LEDS], odd_led_t *[NUM_LEDS] ) = addLeds;
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
			if(!strcmp(input,"sin"))
				animation_function = sinAnimation;
			if(!strcmp(modifier,"add"))
				animation_modifier = addLeds;
			if(!strcmp(modifier,"subtract"))
				animation_modifier = subtractLeds;
			if(!strcmp(modifier,"inversesubtract"))
				animation_modifier = inverseSubtractLeds;
			if(!strcmp(modifier,"multiply"))
				animation_modifier = multiplyLeds;

			odd_led_t* color = malloc(sizeof(odd_led_t));
			color->R = (unsigned char)r;
			color->G = (unsigned char)g;
			color->B = (unsigned char)b;
			
			addAnimation(animation_function,speed,radius,color,animation_modifier); 
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
			
		}*/
	}
	printf("Exiting...\n");
	done = 1;
	pthread_join(ul, NULL);
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

