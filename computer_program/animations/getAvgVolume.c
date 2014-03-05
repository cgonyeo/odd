#include "../odd.h"
#include "getAvgVolume.h"

float getAvgVolume()
{
    //The average volume
    SAMPLE avg = 0;

    //The sound
    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    getSoundBuffer(soundBuffer);

    //Averaging the absolute values of all the sound
    for(int i = 0; i < FFT_INPUT_SIZE; i++)
    {
        if(soundBuffer[i] > 0)
            avg += soundBuffer[i];
        else
            avg -= soundBuffer[i];
    }
    avg /= FFT_INPUT_SIZE;
    //Applying a logarithmic scale to it
    avg *= 100;
    avg = log(avg + 1) / log(101);

    return avg;
}
