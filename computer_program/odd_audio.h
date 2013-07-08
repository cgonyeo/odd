#ifndef ODD_AUDIO
#define ODD_AUDIO

/* Select sample format. */
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

#define FRAMES_PER_BUFFER (128)
#define FFT_INPUT_SIZE (2048)
#define FFT_OUTPUT_SIZE (2048)
#define SAMPLE_RATE (44100)

void getSoundBuffer(SAMPLE* buf);
void runFFT(SAMPLE* buf);
int recordCallback( const void *inputBuffer, void *outputBuffer,
			unsigned long framesPerBuffer, 
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData );
void audioInitialization(void);
void audioStop(void);

#endif
