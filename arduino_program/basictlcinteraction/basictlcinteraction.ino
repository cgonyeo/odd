#include "Tlc5940.h"

int counter;

void setup()
{
  Serial.begin(57600);
  Tlc.init();
  delay(10);
  Tlc.setAll(4000);
  delay(10);
  Tlc.update();
  counter = 0;
}

void loop()
{
  if(Serial.available())
  {
    int input = Serial.read();
    //Serial.write(input);
    
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
  if(counter == 16)
    counter++;
  Tlc.set(counter, strength * 16);
  counter++;
  if(counter > 30)
    counter = 0;
}
