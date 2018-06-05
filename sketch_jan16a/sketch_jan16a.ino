#include <Debounce.h>

const int SWITCH1=4;
const int LED=13;

void setup()
{
  pinMode(SWITCH1, INPUT);
  digitalWrite(SWITCH1, HIGH);   // turn on the built in pull-up resistor
  pinMode(LED, OUTPUT);
  Debounce deb(SWITCH1, 50);
}


void loop()
{
  
  
}


