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
#include "/usr/local/include/portaudio.h"
#include "tlc5940.h"
#include "odd_audio.h"
#include "odd_data_types.h"
#include "odd_math.h"
#include "odd_animations.h"
#include "odd_anim_modifiers.h"

#define INCPORT 10001
#define MAXRCVLEN 500
#define LISTENQ (1024)

#define DEV "/dev/ttyUSB0"

long double totalTime, elapsedTime;
int done = 0;
int numAnimations = 0;
int timeLoops = 0;

odd_led_t* leds[NUM_LEDS]; //All the LEDs in use
odd_led_t* tempLeds[NUM_LEDS]; //Current alterations to the LEDs, used with animations
animation_t* animations[50]; //All currently used animations.

//Writes the led array to the console
void write_console() {
	printf("\n");
	for(int i=0; i<NUM_LEDS; i++) {
		printf("%d, ", leds[i]->R);
		fflush(NULL);
	}
}

//Sends the news LED values to the hardware
void write_odd() {
	for(int j = 0; j < NUM_TLCS; j++)
		for(int i = 0; i < 8; i++)
		{
			setLed(j * 24 + i*3, leds[i]->R);
			setLed(j * 24 + i*3+1, leds[i]->G);
			setLed(j * 24 + i*3+2, leds[i]->B);
		}
	updateLeds();
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
			write_odd();

		usleep(500);
	}
	resetLeds();
	if(failed==0)
		write_odd();
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

void *networkListen(char *buffer)
{
	int list_s;		//Listening socket
	int conn_s;		//connection socket
	short int port = INCPORT;	//port number
	struct sockaddr_in servaddr;	//socket address struct
	char* tok;		//To split off tokens from input
	printf("Starting socket\n");
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
		
		double speed = 0, radius = 0;
		int r = 0, g = 0, b = 0;

		char* temp = buffer;
		char* line = buffer;
		
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
				if(write(conn_s, "ok", 2) < 0)
				{
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
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
				if(write(conn_s, "ok", 2) < 0)
				{
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				char* tmp = strchr(tok, ' ');
				*tmp = '\0';
				removeAnimation(atoi(tok));
			}
			else if(!strcmp(line,"time"))
			{
				if(write(conn_s, "ok", 2) < 0)
				{
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				printf("Time: %Lf\n", totalTime);
			}
			else if(!strcmp(line,"stop"))
			{
				if(write(conn_s, "ok", 2) < 0)
				{
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				for(int i = numAnimations; i > 0; i--)
					removeAnimation(0);
			}
			else if(!strcmp(line,"sup"))
			{
				printf("Sup received\n");
				char reply[4096];
				reply[0] = '\0';
				char *temp;
				char *temp2;
				if(numAnimations != 0)
				{
					for(int i = 0; i < numAnimations; i++)
					{
						printf("Processing animation %i\n", i);
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
						else if(animations[i]->modifier == subtractLeds)
							temp2 = "subtract";
						else if(animations[i]->modifier == multiplyLeds)
							temp2 = "multiply";

						sprintf(reply, "%s%s %f %f %i %i %i %s !", reply, temp, animations[i]->speed, animations[i]->radius, animations[i]->color->R, animations[i]->color->G, animations[i]->color->B, temp2);
						
					}

					printf("Sending:%s\n", reply);

					if(write(conn_s, reply, strlen(reply)) < 0)
					{
						printf("Error writing\n");
						exit(EXIT_FAILURE);
					}

					printf("Animation sent.\n");
				}
				else
					if(write(conn_s, "Not much, you?", 14) < 0)
					{
						printf("Error writing\n");
						exit(EXIT_FAILURE);
					}
			}
			else
			{
				if(write(conn_s, "ok", 2) < 0)
				{
					printf("Error writing\n");
					exit(EXIT_FAILURE);
				}
				
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

				if(r < 0)
					r = 0;
				if(b < 0)
					b = 0;
				if(g < 0)
					g = 0;
				if(r > 4095)
					r = 4095;
				if(b > 4095)
					b = 4095;
				if(g > 4095)
					g = 4095;
				
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
				color->R = r;
				color->G = g;
				color->B = b;
			
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
			line = temp;
			temp = strchr(buffer, '!');
			if(temp != NULL)
				*temp++ = '\0';
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

	tlc5940init();

	audioInitialization();

	odd_led_t* color = malloc(sizeof(odd_led_t));
	color->R = 100;
	color->G = 0;
	color->B = 0;

	addAnimation(dammitAnimation, 0.05, 0.5, color, addLeds);
	
	networkListen(input);
	printf("Exiting...\n");
	done = 1;
	pthread_join(ul, NULL);
	tlc5940cleanup();
	audioStop();
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

