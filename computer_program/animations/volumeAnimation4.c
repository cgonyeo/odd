#include "../odd.h"
#include "getAvgVolume.h"

void volumeAnimation4(double *params, double totalTime, odd_led_t *color, double *storage)
{
    (void)params;
    (void)totalTime;
    
    SAMPLE rAvg = 0;
    SAMPLE gAvg = 0;
    SAMPLE bAvg = 0;

    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    runFFT(soundBuffer);

    //If 48000Hz of sampling and 512 samples, then 46.875 Hz/bin
    //So let's say:
    //Lows = 0-300 Hz
    //Mids = 300-1400 Hz
    //Highs = 1400-5000 Hz
    //Lows = 0-4
    //Mids = 4-30
    //Highs = 30-107

    const int lowMax = 4;
    const int midMax = 30;
    const int highMax = 107;

    for(int i = 0; i < lowMax; i++)
    {
        rAvg += soundBuffer[i];
    }
    for(int i = lowMax; i < midMax; i++)
    {
        gAvg += soundBuffer[i];
    }
    for(int i = midMax; i < highMax; i++)
    {
        bAvg += soundBuffer[i];
    }
    //avg *= 100;
    //avg = log(avg + 1) / log(101);
    rAvg /= 800;
    while(rAvg > 1)
    {
        printf("rfuck\n");
        rAvg -= 1;
    }
    gAvg /= 800;
    while(gAvg > 1)
    {
        printf("gfuck\n");
        gAvg -= 1;
    }
    bAvg /= 800;
    while(bAvg > 1)
    {
        printf("bfuck\n");
        bAvg -= 1;
    }
    rAvg = logf(rAvg * 10 + 1) / logf(11);
    gAvg = logf(gAvg * 10 + 1) / logf(11);
    bAvg = logf(bAvg * 10 + 1) / logf(11);
    int rWidth = (int)(rAvg * NUM_LEDS / 3);
    int gWidth = (int)(gAvg * NUM_LEDS / 3);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i,'r', 0);
        setTempLED(i,'g', 0);
        setTempLED(i,'b', 0);
    }
    for(int i = 0; i < rWidth; i++)
    {
        setTempLED(i,'r', 1000 * rAvg);
    }
    for(int i = NUM_LEDS / 3; i < NUM_LEDS / 3 + gWidth; i++)
    {
        setTempLED(i,'g', 1000 * gAvg);
    }
    //for(int i = NUM_LEDS * 2 / 3; i < NUM_LEDS * 2 / 3 + bWidth; i++)
    //{
    //    setTempLED(i,'b', 1000 * bAvg);
    //}
}
