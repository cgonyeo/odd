#include "Tlc5940.h"

int counter;

void setup()
{
  Serial.begin(115200);
  Tlc.init();
  delay(10);
  Tlc.setAll(400);
  delay(10);
  Tlc.update();
  counter = 0;
}

void loop()
{
  if(Serial.available())
  {
    int input = Serial.read();
    
    if(input == 255)
    {
      counter = 0;
      Tlc.update();
    }
    else
      writeLed(input);
  }
}

void writeLed(int strength)
{
  Tlc.set(counter, strength * 16);
  counter++;
  if(counter > 29)
    counter = 0;
}
