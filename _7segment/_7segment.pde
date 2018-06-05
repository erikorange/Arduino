/*
  7 segment LED driver
 
 */

const int A=3;
const int B=4;
const int C=5;
const int D=6;

const int REDLED=8;
const int GREENLED=9;

const int LED=13;


void setup()
{
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(LED, OUTPUT);
}


void loop()
{
  int idx;

  for (idx=0; idx<10; idx++)
  {
    displayDigit(idx);
    flipLED(idx);
    blinkLED(3);
  }
}

void flipLED(int i)
{
  if (i%2==0)
  {
    digitalWrite(REDLED, HIGH);
    digitalWrite(GREENLED, LOW);
  }
  else
  {
    digitalWrite(REDLED, LOW);
    digitalWrite(GREENLED, HIGH);
  }

}
void blinkLED(int times)
{
  int b;
  for (b=0; b<times; b++)
  {
    digitalWrite(LED, HIGH);
    delay(150);
    digitalWrite(LED, LOW);
    delay(150);
  } 
}



void displayDigit(int digit)
{
  switch (digit)
  {
  case 0: 
    zero();
    break;

  case 1: 
    one();
    break;

  case 2: 
    two();
    break;

  case 3: 
    three();
    break;

  case 4: 
    four();
    break;

  case 5: 
    five();
    break;

  case 6: 
    six();
    break;

  case 7: 
    seven();
    break;

  case 8: 
    eight();
    break;

  case 9: 
    nine();
    break;
  }
}


void zero()
{
  digitalWrite(A, LOW);
  digitalWrite(B, LOW); 
  digitalWrite(C, LOW); 
  digitalWrite(D, LOW);
}

void one()
{
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW); 
  digitalWrite(C, LOW); 
  digitalWrite(D, LOW);
}

void two()
{
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH); 
  digitalWrite(C, LOW); 
  digitalWrite(D, LOW);
}

void three()
{
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH); 
  digitalWrite(C, LOW); 
  digitalWrite(D, LOW);
}

void four()
{
  digitalWrite(A, LOW);
  digitalWrite(B, LOW); 
  digitalWrite(C, HIGH); 
  digitalWrite(D, LOW);
}

void five()
{
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW); 
  digitalWrite(C, HIGH); 
  digitalWrite(D, LOW);
}

void six()
{
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH); 
  digitalWrite(C, HIGH); 
  digitalWrite(D, LOW);
}

void seven()
{
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH); 
  digitalWrite(C, HIGH); 
  digitalWrite(D, LOW);
}

void eight()
{
  digitalWrite(A, LOW);
  digitalWrite(B, LOW); 
  digitalWrite(C, LOW); 
  digitalWrite(D, HIGH);
}

void nine()
{
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW); 
  digitalWrite(C, LOW); 
  digitalWrite(D, HIGH);
}






