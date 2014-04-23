#include "odd.h"

//Returns the value of X raised to Y
//Y should be an integer. Ignore the fact that it isn't.
double odd_pow(double x, double y)
{
	if(y > 0)
		return odd_pow(x, y - 1) * x;
	else
		return 1.0;
}

//Returns a taylor series approximation for (sin((x - 0.5) * pi) + 1) / 2
//Put in a number between 0 and 1
//and it returns a number between 0 and 1
double odd_sin(double x)
{
	x -= .5;
	x = PI * x - ( odd_pow(PI, 3) * odd_pow(x, 3) ) / 6 + ( odd_pow(PI, 5) * odd_pow(x, 5) ) / 120;
	x++;
	x /= 2;
	return x;
}

float hann_window(int sample, int num_samples)
{
    return 0.5 * (1 - cos((2 * PI * sample) / (num_samples - 1)));
}

//Returns the time as a double (formatted to be smaller)
double formatTime(long int seconds, long int useconds)
{
	long double time = seconds % 10000 + (useconds - useconds % 1000) / 1000000.0;
	
	if(time > 100000)
		time = fmod(time, 100000);
	
	return time;
}
