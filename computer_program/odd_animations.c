#include "odd.h"

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double *params, double totalTime, odd_led_t *color, double *storage) {
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
    odd_led_t *color2 = color->next;
    if(color2 == NULL)
    {
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
    else
    {
        //If an LED has a positive brightness, set it.
        for(int i = 0; i < NUM_LEDS; i++)
        {
            if(ledDistances[i] > 0)
            {
                double colorDistance = pow(ledDistances[i], 1);
                odd_led_t colorTemp;
                colorTemp.R = color->R * colorDistance + color2->R * (0.1 / colorDistance);
                colorTemp.G = color->G * colorDistance + color2->G * (0.1 / colorDistance);
                colorTemp.B = color->B * colorDistance + color2->B * (0.1 / colorDistance);

                double colorStrength = (16/3) * (pow(((1.0 - ledDistances[i]) * 2) + 2, -2) - (1.0 / 15.5));
                setTempLED(i, 'r', colorTemp.R * colorStrength);
                setTempLED(i, 'g', colorTemp.G * colorStrength);
                setTempLED(i, 'b', colorTemp.B * colorStrength);
            }
        }
    }
}

//Animation: Turns the LEDs off and on and off and on and off and on and off...
void strobe(double *params, double totalTime, odd_led_t *color, double *storage)
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
void setAll(double *params, double totalTime, odd_led_t *color, double *storage)
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
void smoothStrobe(double *params, double totalTime, odd_led_t *color, double *storage)
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
void sinAnimation(double *params, double totalTime, odd_led_t *color, double *storage)
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

//A given led's neighbors are comprised of the three leds to the left and right
//4, 5 or 6 neighbors = dead due to overpopulation
//2 or 3 neighbors = lives
//0 or 1 neighbors = dead due to underpopulation
void calculateNextStepOfGame(double *in, double *out)
{    
    int range = 3; 
    for(int i = 0; i < NUM_LEDS; i++)
    {
        int neighborCount = 0;
        for(int j = i - range; j <= i + range; j++)
        {
            if(j == i)
                continue;
            int index = j;
            if(index < 0)
                index += NUM_LEDS;
            if(index > NUM_LEDS)
                index -= NUM_LEDS;
            neighborCount += in[index];
        }
       * (out + i) = neighborCount == 2 || neighborCount == 3 || neighborCount == 4;
    }
}

void printBoard(double *board)
{
    printf("game: ");
    for(int i = 0; i < NUM_LEDS; i++)
        printf("%d ", (int)board[i]);
    printf("\n");
}

void gameOfLife(double *params, double totalTime, odd_led_t *color, double *storage)
{
    int counterIndex = NUM_LEDS*2;

    double speed = *params;

    if(storage[counterIndex] == 0)
    {
        //First run, let's initialize some stuff
        *(storage + counterIndex) = totalTime + 1 / speed;
        int seed = (int)(params[1]) % NUM_LEDS;
        for(int i = 0; i < NUM_LEDS; i++)
        {
            *(storage + i) = (seed & i) != 0;
        }
        //printBoard(storage);
        calculateNextStepOfGame(storage, storage+NUM_LEDS);
        //printBoard(storage+NUM_LEDS);
    }

    if(totalTime > storage[counterIndex])
    {
        *(storage + counterIndex) = totalTime + 1 / speed;
        memcpy(storage, storage + NUM_LEDS, sizeof(double) * NUM_LEDS);
        calculateNextStepOfGame(storage, storage + NUM_LEDS);
        //printBoard(storage+NUM_LEDS);
    }

    double progress = 1 - (storage[counterIndex] - totalTime) / (1 / speed);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        double strength = storage[i] * progress + storage[i + NUM_LEDS] * (1 - progress);
        setTempLED(i, 'r', color->R * strength);
        setTempLED(i, 'g', color->G * strength);
        setTempLED(i, 'b', color->B * strength);
    }
}

void volumeAnimation1(double *params, double totalTime, odd_led_t *color, double *storage)
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

void volumeAnimation2(double *params, double totalTime, odd_led_t *color, double *storage)
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

void volumeAnimation3(double *params, double totalTime, odd_led_t *color, double *storage)
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

void dammitAnimation(double *params, double totalTime, odd_led_t *color, double *storage)
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
