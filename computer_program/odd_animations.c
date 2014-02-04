#include "odd.h"

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]) {
    double speed = params[0];
    double radius = params[1];

    //scale the time by our speed to alter the rate of tf the animation
    double time = totalTime * speed;
    //double to keep track of the location of the center
    double center = 0.0;
    //we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
    int numLeds = NUM_LEDS - 1;
    //clear the tempLeds array so we can use it
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }
    
    //Calculate the center
    if((int)time % 2 == 1)
        center = odd_remainder(time, 1);
    else
        center = 1 - odd_remainder(time, 1);
    center = odd_sin(center);
    center *= numLeds;
    
    
    //Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
    double ledDistances[NUM_LEDS];
    for(int i = 0; i < NUM_LEDS; i++)
    {
        if(center - i > 0)
            ledDistances[i] = center - i;
        else
            ledDistances[i] = i - center;
        ledDistances[i] -= numLeds;
        ledDistances[i] *= -1;
        ledDistances[i] -= numLeds - radius;
        ledDistances[i] *= 1 / radius;
    }
    //If an LED has a positive brightness, set it.
    for(int i = 0; i < NUM_LEDS; i++)
    {
        if(ledDistances[i] > 0)
        {
            setTempLED(i, 'r', color->R * ledDistances[i]);
            setTempLED(i, 'g', color->G * ledDistances[i]);
            setTempLED(i, 'b', color->B * ledDistances[i]);
        }
    }
}

//Animation: Turns the LEDs off and on and off and on and off and on and off...
void strobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    double speed = params[0];
    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = 1;
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}

//Animation: Sets all LEDs to color
void setAll(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R);
        setTempLED(i, 'g', color->G);
        setTempLED(i, 'b', color->B);
    }
}

//Animation: Like strobe but fades LEDs in and out and pauses when they're off.
void smoothStrobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    double speed = params[0];

    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = odd_remainder(time, 1);
    else
        power = 1 - odd_remainder(time, 1);
    power = odd_pow(power, 2);
    power *= 1.5;
    power -= 0.5;
    if(power < 0)
        power = 0;
    for(int i = 0; i < NUM_LEDS; i++) {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}

//Animation that makes the brightness of the LEDs follow a sin curve
void sinAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    double speed = params[0];

    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = odd_remainder(time, 1);
    else
        power = 1 - odd_remainder(time, 1);
    power = odd_sin(power);
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}

void volumeAnimation1(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;
    (void)totalTime;
    
    SAMPLE avg = 0;

    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    getSoundBuffer(soundBuffer);

    for(int i = 0; i < FFT_INPUT_SIZE; i++)
    {
        if(soundBuffer[i] > 0)
            avg += soundBuffer[i];
        else
            avg -= soundBuffer[i];
    }
    avg /= FFT_INPUT_SIZE;
    avg *= 100;
    avg = log(avg + 1) / log(101);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * avg);
        setTempLED(i, 'g', color->G * avg);
        setTempLED(i, 'b', color->B * avg);
    }
}

void volumeAnimation2(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;
    (void)totalTime;
    
    SAMPLE avg = 0;

    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    getSoundBuffer(soundBuffer);

    for(int i = 0; i < FFT_INPUT_SIZE; i++)
    {
        if(soundBuffer[i] > 0)
            avg += soundBuffer[i];
        else
            avg -= soundBuffer[i];
    }
    avg /= FFT_INPUT_SIZE;
    avg *= 100;
    avg = log(avg + 1) / log(101);

    for(int i = 0; i < avg * NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * avg);
        setTempLED(i, 'g', color->G * avg);
        setTempLED(i, 'b', color->B * avg);
    }
    for(int i = avg * NUM_LEDS; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }
}

void volumeAnimation3(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;
    (void)totalTime;
    
    SAMPLE avg = 0;

    SAMPLE soundBuffer[FFT_INPUT_SIZE];
    getSoundBuffer(soundBuffer);

    for(int i = 0; i < FFT_INPUT_SIZE; i++)
    {
        if(soundBuffer[i] > 0)
            avg += soundBuffer[i];
        else
            avg -= soundBuffer[i];
    }
    avg /= FFT_INPUT_SIZE;
    avg *= 100;
    avg = log(avg + 1) / log(101);

    int width = (int)(avg * NUM_LEDS);
    int gap = (int)((NUM_LEDS - width) / 2);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i,'r', 0);
        setTempLED(i,'g', 0);
        setTempLED(i,'b', 0);
    }
    for(int i = gap; i < gap+width; i++)
    {
        setTempLED(i,'r', color->R * avg);
        setTempLED(i,'g', color->G * avg);
        setTempLED(i,'b', color->B * avg);
    }
}

void volumeAnimation4(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
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

    const int lowMax = 6;
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
    rAvg /= 700;
    while(rAvg > 1)
        rAvg -= 1;
    gAvg /= 700;
    while(gAvg > 1)
        gAvg -= 1;
    bAvg /= 700;
    while(bAvg > 1)
        bAvg -= 1;
    rAvg = logf(rAvg * 100 + 1) / logf(101);
    gAvg = logf(gAvg * 100 + 1) / logf(101);
    bAvg = logf(bAvg * 100 + 1) / logf(101);
    int rWidth = (int)(rAvg * NUM_LEDS);
    int rGap = (int)((NUM_LEDS - rWidth) / 2);
    int gWidth = (int)(gAvg * NUM_LEDS);
    int gGap = (int)((NUM_LEDS - gWidth) / 2);
    int bWidth = (int)(bAvg * NUM_LEDS);
    int bGap = (int)((NUM_LEDS - bWidth) / 2);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i,'r', 0);
        setTempLED(i,'g', 0);
        setTempLED(i,'b', 0);
    }
    for(int i = rGap; i < rGap+rWidth; i++)
    {
        setTempLED(i,'r', 1000 * rAvg);
    }
    for(int i = gGap; i < gGap+gWidth; i++)
    {
        setTempLED(i,'g', 1000 * gAvg);
    }
    for(int i = bGap; i < bGap+bWidth; i++)
    {
        setTempLED(i,'b', 1000 * bAvg);
    }
}

void dammitAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;

    SAMPLE soundBuffer[FFT_OUTPUT_SIZE];
    memset(soundBuffer, 0, sizeof(*soundBuffer * FFT_OUTPUT_SIZE));
    runFFT(soundBuffer);

    float rangePerLed = logf(FFT_OUTPUT_SIZE / 2 / 16.0 + 1) / NUM_LEDS;

    SAMPLE ledSums[NUM_LEDS];
    int ledCounts[NUM_LEDS];

    memset(ledSums, 0, sizeof(*ledSums * NUM_LEDS));
    memset(ledCounts, 0, sizeof(*ledCounts) * NUM_LEDS);

    for(int i = 0; i < FFT_OUTPUT_SIZE/2; i++)
    {
        int location = (int)(logf(i / 16.0 + 1) / rangePerLed);

        ledSums[location] += soundBuffer[i];
        ledCounts[location]++;
    }
    for(int i = 0; i < NUM_LEDS; i++)
        ledSums[i] /= ledCounts[i];
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'g', (int)(ledSums[i]));
    }
}
