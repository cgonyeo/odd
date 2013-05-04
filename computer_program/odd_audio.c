#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <fftw3.h>
#include "/usr/local/include/portaudio.h"
#include "odd_audio.h"
#include "odd_math.h"

PaStream* stream;
fftw_complex *in, *out;
fftw_plan plan;

void getSoundBuffer(SAMPLE* buf)
{
	for(int i = 0; i < FRAMES_PER_BUFFER; i++)
		buf[i] = in[i][0];
}

void runFFT(SAMPLE* buf)
{
	//printf("\n[");
	fftw_execute(plan);
	for(int i = 0; i < FRAMES_PER_BUFFER; i++)
	{
		buf[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
	//	printf("%f,",buf[i]);
	}
	//printf("]\n");
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
		in[i][0] = input[i];
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
	
	printf("Getting devices...\n");
	const PaDeviceInfo *deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	printf("%i devices found\n", numDevices);
	for(int i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("Device: %s\n", deviceInfo->name);
	}

	inputParameters.device = Pa_GetDefaultInputDevice();
	if(inputParameters.device == paNoDevice)
	{
		printf("Error: no default input device\n");
		exit(EXIT_FAILURE);
	}

	inputParameters.channelCount = 1;
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
		printf("Error opening stream\n%s\n", Pa_GetErrorText(err));
		exit(EXIT_FAILURE);
	}

	err = Pa_StartStream(stream);
	if(err != paNoError)
	{
		printf("Error starting stream\n");
		exit(EXIT_FAILURE);
	}

	printf("Audio setup complete\n");

	printf("Beginning FFT initialization\n");

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);

	for(int i = 0; i < FRAMES_PER_BUFFER; i++)
	{
		in[i][0] = 0;
		in[i][1] = 0;
	}

	plan = fftw_plan_dft_1d(FRAMES_PER_BUFFER, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	printf("FFT initialization complete\n");
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
	fftw_destroy_plan(plan);
	fftw_free(in);
	fftw_free(out);
}
