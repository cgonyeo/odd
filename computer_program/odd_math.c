#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "odd_math.h"


//Returns the odd_remainder of dividend / divisor
double odd_remainder(double dividend, int divisor) 
{
	int quotient = (int) dividend / divisor;
	return dividend - divisor * quotient;
}

//Returns the value of X raised to Y
//Y should be an integer. Ignore the fact that it isn't.
double pow(double x, double y)
{
	if(y > 0)
		return pow(x, y - 1) * x;
	else
		return 1.0;
}

//Returns a taylor series approximation for (sin((x - 0.5) * pi) + 1) / 2
//Put in a number between 0 and 1
//and it returns a number between 0 and 1
double sin(double x)
{
	x -= .5;
	x = PI * x - ( pow(PI, 3) * pow(x, 3) ) / 6 + ( pow(PI, 5) * pow(x, 5) ) / 120;
	x++;
	x /= 2;
	return x;
}

//Returns the time as a double (formatted to be smaller)
double formatTime(long int seconds, long int useconds)
{
	long double time = seconds % 10000 + (useconds - useconds % 1000) / 1000000.0;
	
	if(time > 100000)
		time = odd_remainder(time, 100000);
	
	return time;
}
