/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */

void setup()
{
  int pin;

  for (pin=2; pin<14; pin++)
  {
    pinMode(pin, OUTPUT);
  }  
}

void loop()
{
  goUp(30);
  goDown(30);
  spread();
  collapse();
  spread();
  collapse();
  sparkle();
}

void goUp(int delayTime)
{
  int pin;

  for (pin=2; pin<14; pin++)
  {
    digitalWrite(pin, HIGH);
    delay(delayTime);
    digitalWrite(pin, LOW); 
  }
}

void goDown(int delayTime)
{
  int pin;

  for (pin=13; pin>1; pin--)
  {
    digitalWrite(pin, HIGH);
    delay(delayTime);
    digitalWrite(pin, LOW); 
  }
}

void sparkle()
{
  int count;
  int pin;
  int dly;

  dly = 25;
  for (count=0; count<50; count++)
  {
    pin = random(2,14);
    digitalWrite(pin, HIGH);
    delay(dly);
    digitalWrite(pin, LOW);
    delay(dly);
  }
}

void spread()
{
  int lwr;
  int upr;
  int cnt;

  lwr = 7;
  upr = 8;

  for (cnt=0; cnt <6; cnt++)
  {
    digitalWrite(lwr, HIGH);
    digitalWrite(upr, HIGH);
    delay(25);
    digitalWrite(lwr, LOW);
    digitalWrite(upr, LOW);
    delay(25);
    lwr--;
    upr++;
  }

}

void collapse()
{
  int lwr;
  int upr;
  int cnt;

  lwr = 2;
  upr = 13;

  for (cnt=0; cnt <6; cnt++)
  {
    digitalWrite(lwr, HIGH);
    digitalWrite(upr, HIGH);
    delay(25);
    digitalWrite(lwr, LOW);
    digitalWrite(upr, LOW);
    delay(25);
    lwr++;
    upr--;
  }

}








