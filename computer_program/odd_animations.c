#include "odd.h"

//Helper method. Returns the length of the array each animation is expecting in double* params.
//If the function is not found, returns -1
int numParams(void (*function)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]))
{

    if(function == cylonEye)
        return 2;
    if(function == strobe)
        return 1;
    if(function == setAll)
        return 0;
    if(function == smoothStrobe)
        return 1;
    if(function == sinAnimation)
        return 1;
    if(function == volumeAnimation)
        return 0;
    if(function == dammitAnimation)
        return 0;
    return -1;
}

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

void volumeAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
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
        tempLeds[i]->R = color->R * avg;
        tempLeds[i]->G = color->G * avg;
        tempLeds[i]->B = color->B * avg;
    }
}

void dammitAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
    (void)params;

    SAMPLE soundBuffer[FFT_OUTPUT_SIZE];
    runFFT(soundBuffer);
    int binsPerLed = 20;
    float ledsPerSection = NUM_LEDS / 9.0f;

    float average = 0;
    
    for(int i = 0; i < FFT_OUTPUT_SIZE; i++)
    {
        average += soundBuffer[i];
    }
    printf("%f\n", (average / FFT_OUTPUT_SIZE));
}
