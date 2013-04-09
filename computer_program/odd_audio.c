#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "portaudio.h"
#include "odd_audio.h"

SAMPLE soundBuffer[FRAMES_PER_BUFFER];
PaStream* stream;

void getSoundBuffer(SAMPLE* buf)
{
	for(int i = 0; i < FRAMES_PER_BUFFER; i++)
		buf[i] = soundBuffer[i];
}

static int recordCallback( const void *inputBuffer, void *outputBuffer,
			unsigned long framesPerBuffer, 
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData )
{
	const SAMPLE *input = (const SAMPLE*)inputBuffer;
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	for(int i = 0; i < framesPerBuffer; i++)
	{
		soundBuffer[i] = input[i];
	}
//	printf("Callback called\n");
	return paContinue;
}

void audioInitialization()
{
	PaStreamParameters inputParameters;
	PaError err = paNoError;
	
	err = Pa_Initialize();
	if(err != paNoError)
	{
		printf("Error calling PaInitialize (line 198)\n");
		exit(EXIT_FAILURE);
	}

	inputParameters.device = Pa_GetDefaultInputDevice();
	if(inputParameters.device == paNoDevice)
	{
		printf("Error: no default input device\n");
		exit(EXIT_FAILURE);
	}

	inputParameters.channelCount = 2;
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,
		recordCallback,
		NULL);
	
	if(err != paNoError)
	{
		printf("Error opening stream\n");
		exit(EXIT_FAILURE);
	}

	err = Pa_StartStream(stream);
	if(err != paNoError)
	{
		printf("Error starting stream\n");
		exit(EXIT_FAILURE);
	}

	printf("Audio setup complete\n");
}

void audioStop()
{
	PaError err = paNoError;
	err = Pa_CloseStream(stream);
	if(err != paNoError)
	{
		printf("Error closing audio stream\n");
		exit(EXIT_FAILURE);
	}
}