#include "../odd.h"
#include "getAvgVolume.h"

//Generates a volume meter, centered on the display. It's color varies from 1 to 2 based on the strength of the bass
void volumeAnimation5(double *params, double totalTime, odd_led_t *color, double *lastAvg)
{
    (void)params;
    (void)totalTime;
    
    //An average for the sound, and an average for the strength of the bass
    SAMPLE avg = 0;
    SAMPLE bassAvg = 0;

    //The sound buffer
    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    getSoundBuffer(soundBuffer);

    //The frequency strengths
    SAMPLE fftOutput[FFT_OUTPUT_SIZE];
    runFFT(fftOutput);

    //Calculate the average volume of the sound
    for(int i = 0; i < FFT_INPUT_SIZE; i++)
    {
        if(soundBuffer[i] > 0)
            avg += soundBuffer[i];
        else
            avg -= soundBuffer[i];
    }
    avg /= FFT_INPUT_SIZE;
    //Apply a logarithmic scale to it
    avg *= 100;
    avg = log(avg + 1) / log(101);

    //Get the average strength of the bass
    for(int i = 0; i < 4; i++)
    {
        bassAvg += fftOutput[i];
    }
    bassAvg /= 300;
    while(bassAvg > 1)
    {
        printf("bassfuck\n");
        bassAvg -= 1;
    }

    //Let's dampen reductions in volume by 50%, in an attempt to prevent flickering
    if(avg < *lastAvg)
        avg = (avg + *lastAvg) / 2;

    //Calculate the width of the leds, and the gap on either side
    int width = (int)(avg * NUM_LEDS);
    int gap = (int)((NUM_LEDS - width) / 2);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i,'r', 0);
        setTempLED(i,'g', 0);
        setTempLED(i,'b', 0);
    }
    //Fill in the leds
    for(int i = gap; i < gap+width; i++)
    {
        if(color->next != NULL) {
            //strength varies with avg, color varies with bassAvg
            int rVal = color->R * avg * 1 / bassAvg + color->next->R * bassAvg;
            int gVal = color->G * avg * 1 / bassAvg + color->next->G * bassAvg;
            int bVal = color->B * avg * 1 / bassAvg + color->next->B * bassAvg;
            setTempLED(i,'r', rVal);
            setTempLED(i,'g', gVal);
            setTempLED(i,'b', bVal);
        }
    }
    *lastAvg = avg;
}
