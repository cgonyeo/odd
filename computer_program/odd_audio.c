#include "odd.h"

//Jack things
jack_port_t *input_port;
//jack_port_t *output_port;

//FFTW things
fftw_complex *in, *out;
fftw_plan plan;

SAMPLE storage[FFT_INPUT_SIZE];

void getSoundBuffer(SAMPLE* buf)
{
	for(int i = 0; i < FFT_OUTPUT_SIZE; i++)
		buf[i] = storage[i];
}

void runFFT(SAMPLE* buf)
{
	//("runFFT called\n");
	for(int i = 0; i < FFT_INPUT_SIZE; i++)
		in[i][0] = storage[i];
	fftw_execute(plan);
	for(int i = 0; i < FFT_OUTPUT_SIZE; i++)
	{
		buf[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
	}
	//("runFFT finished\n");
}

int counter = 0;

int processAudio(jack_nframes_t nframes, void *arg)
{
    //jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
    jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);

    //memcpy (out, in, sizeof (jack_default_audio_sample_t) * nframes);

    for(int i = 0; i < nframes; i++)
    {
        storage[i] = in[i];
    }

    return 0;
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


    //jack
	printf("Beginning jack initialization\n");

    jack_client_t *client;
    const char **ports;
    
    /* try to become a client of the jack server */
    client = jack_client_open("odd", JackNullOption, NULL);

    /* tell the jack server to call `processAudio()' whenever
        there is work to be done */
    jack_set_process_callback (client, processAudio, 0);

    /* tell the jack server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.  */

    jack_on_shutdown (client, jack_shutdown, 0);

    /* display the current sample rate.  */
    printf ("engine sample rate: %" PRIu32 "\n",
            jack_get_sample_rate (client));

    /* create two ports */
    input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    //output_port = jack_port_register (client, "output", jack_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* tell the jack server that we are ready to roll */
    if (jack_activate (client)) {
        fprintf (stderr, "cannot activate client");
        return 1;
    }

    /* connect the ports. Note: you can't do this before
        the client is activated, because we can't allow
        connections to be made to clients that aren't
        running. */
    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
        fprintf(stderr, "Cannot find any physical capture ports\n");
        return 1;
    }

    if (jack_connect (client, ports[0], jack_port_name (input_port))) {
        fprintf (stderr, "cannot connect input ports\n");
    }

    free (ports);

    //if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
    //    fprintf(stderr, "Cannot find any physical playback ports\n");
    //    return 1;
    //}

    //if (jack_connect (client, jack_port_name (output_port), ports[0])) {
    //    fprintf (stderr, "cannot connect output ports\n");
    //}

    //free(ports);

	printf("jack initialization complete\n");
    return 0;
}
