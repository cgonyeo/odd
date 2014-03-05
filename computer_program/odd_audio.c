#include "odd.h"

#if 0
For calculating the frequencies determined by the FFTs,
Frequency range per item = Frequency (48000) / N items (512) * 2
 = 46.875 Hz/bin
#endif

//Jack things
//jack_port_t *input_port;
//jack_port_t *output_port;

//FFTW things
fftw_complex *in, *out;
fftw_plan plan;

pthread_mutex_t storageLock = PTHREAD_MUTEX_INITIALIZER;

SAMPLE storage[FFT_INPUT_SIZE];

void getSoundBuffer(SAMPLE* buf)
{
    pthread_mutex_lock(&storageLock);
	for(int i = 0; i < FFT_OUTPUT_SIZE; i++)
		buf[i] = storage[i] * hann_window(i, FFT_OUTPUT_SIZE);
    pthread_mutex_unlock(&storageLock);
}

void runFFT(SAMPLE* buf)
{
	//("runFFT called\n");
    pthread_mutex_lock(&storageLock);
	for(int i = 0; i < FFT_INPUT_SIZE; i++)
		in[i][0] = storage[i];
    pthread_mutex_unlock(&storageLock);
	fftw_execute(plan);
	for(int i = 0; i < FFT_OUTPUT_SIZE; i++)
	{
		buf[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
	}
	//("runFFT finished\n");
}

/**
 * Update loop for recording audio
 */
void *processAudio(void *arg) {
    snd_pcm_t *capture_handle = (snd_pcm_t *)arg;
    short buf[BUFSIZE];
    int err;
    while(1) {
        if ((err = snd_pcm_readi (capture_handle, buf, BUFSIZE)) != BUFSIZE) {
            fprintf (stderr, "read from audio interface failed (%s)\n", 
            snd_strerror (err)); 
            exit (1); 
        } 
        pthread_mutex_lock(&storageLock);
        for(int i = 0; i < BUFSIZE; i++) {
            storage[i] = (SAMPLE) buf[i] / 32768.0;
        }
        pthread_mutex_unlock(&storageLock);
    }

    snd_pcm_close (capture_handle);
}

/**
 * This is the shutdown callback for this jack application.
 * It is called by jack if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
    printf("Jack has shut down\n");
}

int audioInitialization()
{
    //Misc things
	plan=(void*)0;

	for(int i = 0; i < FFT_INPUT_SIZE; i++)
		storage[i] = 0;

    //fftw
	printf("Beginning FFT initialization\n");

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_INPUT_SIZE);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_INPUT_SIZE);
	for(int i = 0; i < FFT_INPUT_SIZE; i++)
	{
		in[i][0] = 0;
		in[i][1] = 0;
	}

	plan = fftw_plan_dft_1d(FFT_INPUT_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);
	printf("FFT initialization complete\n");

    //alsa
    printf("Beginning alsa initialization\n");
    int err;
    char *device = "hw:1,0";
    unsigned int rate = 48000;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
        device,
        snd_strerror (err));
        exit (1);
    }
     
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
        snd_strerror (err));
        exit (1);
    }
     
    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
        snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n", 
        snd_strerror (err)); 
        exit (1); 
    } 

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n", 
        snd_strerror (err)); 
        exit (1); 
    } 

    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n", 
        snd_strerror (err)); 
        exit (1); 
    } 

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n", 
        snd_strerror (err)); 
        exit (1); 
    } 

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
        snd_strerror (err)); 
        exit (1); 
    } 

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n", 
        snd_strerror (err)); 
        exit (1); 
    } 

    pthread_t al;
    pthread_create(&al,NULL,processAudio,capture_handle);

    printf("alsa initialization complete\n");

    return 0;
}
