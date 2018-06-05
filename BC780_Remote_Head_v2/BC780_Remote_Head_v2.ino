#include <Debounce.h>
#include <LiquidCrystal.h>

// LCD rows
const int LCDROW1 = 0;
const int LCDROW2 = 1;
const int LCDROW3 = 2;
const int LCDROW4 = 3;

// I/O pins
const int BUTTON_SCAN = 7;
const int BACKLIGHT = 13;

// BC780 serial commands that return OK as response
const int CMD_SCAN = 1;      // scan mode
const int CMD_MAN = 2;       // manual mode
const int CMD_UP = 3;        // channel Up
const int CMD_DOWN = 4;      // channel Down
const int CMD_SQDENOTIFY = 5;  // squelch notify


// Debounce parameter in ms
const int DEBOUNCE_TIME = 50;

// Signal Strength bars
byte ss_1[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
};


byte ss_12[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B11011,
};

byte ss_3[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B11000,
  B11000,
};

byte ss_34[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B11011,
  B11011,
  B11011,
};


byte ss_5[8] = {
  B00000,
  B00000,
  B00000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};

byte ss_56[8] = {
  B00000,
  B00000,
  B00011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
};

byte ess[8] = {
  B00000,
  B00000,
  B00000,
  B00011,
  B00100,
  B00010,
  B00001,
  B00110,
};




// Connections:
// rs (LCD pin 4) to Arduino pin 12
// rw (LCD pin 5) to Arduino pin 11
// enable (LCD pin 6) to Arduino pin 10
// LCD pin 15 to Arduino pin 13
// LCD pins d4, d5, d6, d7 to Arduino pins 5, 4, 3, 2
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

void setup()
{
  // Configure display
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  // create the Signal Strength chars
  lcd.createChar(0, ss_1);
  lcd.createChar(1, ss_12);
  lcd.createChar(2, ss_3);
  lcd.createChar(3, ss_34);
  lcd.createChar(4, ss_5);
  lcd.createChar(5, ss_56);
  lcd.createChar(6, ess);
  
  
  lcd.begin(20, 4);              // define as a 20x4 LCD
  lcd.clear();
  
  // Configure inputs
  pinMode(BUTTON_SCAN, INPUT);
  digitalWrite(BUTTON_SCAN, HIGH);       // turn on the built in pull-up resistor
  Debounce scanButton(BUTTON_SCAN, DEBOUNCE_TIME);
  
  // Configure serial port
  Serial.begin(19200);
}


void loop()
{
  boolean squelchIsOpen;
  
  // Display opening title
  DisplayTitle();
  
  // Display setup message
  lcd.clear();
  DisplaySetupMessage();
  
  SendCommand(CMD_SQDENOTIFY);  // ensure BC780 doesn't send an async +/- for squelch
  lcd.clear();
  DisplayScanMessage();
  squelchIsOpen = false;
  SendCommand(CMD_SCAN);      // start scanning
 
  while (true)
  {
    if (IsSquelchOpen())
    {
      if (!squelchIsOpen)         // only execute this section once when sq is first detected open
      {
        squelchIsOpen = true;
        delay(125);               // give the scanner time to actually switch to manual mode
        DisplayChannelNum();
        DisplayFrequency();
        DisplayMode();
        DisplayChannelTag();
        DisplayBankTag();
      }
      
      // update signal strength here
      DisplaySignalStrength();
      
    }  // squelch is open
    
    else
    {
      if (squelchIsOpen)    // only execute this section once when sq is first detected closed
      {
        squelchIsOpen = false;
        delay(2000);  // BC780 delay time
        ClearLCDLine(LCDROW1);
        DisplayScanMessage();
        ClearLCDLine(LCDROW3);
        ClearLCDLine(LCDROW4);
      }
      
      DisplayChannelNum();
      
        // animate scanning display here
    }  // squelch is closed
  }
  
  
}


void DisplaySignalStrength()
{
  int idx;
  char buffer[128];
  boolean flag;
  int ch1;
  int ch2;
  int ch3;
  
  // ch[x] indicates which custom character to display; -1 means a space (no signal strength bar)
  ch1 = -1;
  ch2 = -1;
  ch3 = -1;
  
  // get the channel num from the scanner
  Serial.print("LCD SMT\r");
  do
  {
    flag = ReadResponse(buffer, 11);
  }
  while (!flag);
  
  // determine 1st character
  if (buffer[4] == '+' && buffer[5] == '-')
  {
    ch1 = 0;
  }
  if (buffer[4] == '+' && buffer[5] == '+')
  {
    ch1 = 1;
  }
  
  // determine 2nd character
  if (buffer[6] == '+' && buffer[7] == '-')
  {
    ch2 = 2;
  }
  if (buffer[6] == '+' && buffer[7] == '+')
  {
    ch2 = 3;
  }
  
  // determine 2nd character
  if (buffer[8] == '+' && buffer[9] == '-')
  {
    ch3 = 4;
  }
  if (buffer[8] == '+' && buffer[9] == '+')
  {
    ch3 = 5;
  }
  
  lcd.setCursor(15, LCDROW2);
  lcd.write(6);
  
  // display the 1st character
  lcd.setCursor(16, LCDROW2);
  if (ch1 == -1)
  {
    lcd.print(" ");
  }
  else
  {
    lcd.write(ch1);
  }
  
  // display the 2nd character
  lcd.setCursor(17, LCDROW2);
  if (ch2 == -1)
  {
    lcd.print(" ");
  }
  else
  {
    lcd.write(ch2);
  }
  
    // display the 2nd character
  lcd.setCursor(18, LCDROW2);
  if (ch3 == -1)
  {
    lcd.print(" ");
  }
  else
  {
    lcd.write(ch3);
  }
  
  return;
}


void DisplayFrequency()
{
  int idx;
  char buffer[128];
  boolean flag;
  
//  lcd.setCursor(0, LCDROW1);
//  lcd.print("Req...");
  
  // get the freq from the scanner
  Serial.print("LCD FRQ\r");
  do
  {
    flag = ReadResponse(buffer, 16);
  }
  while (!flag);
  
  // clear the line and display it
  ClearLCDLine(LCDROW2);
  for (idx=0; idx<9; idx++)
  {
    lcd.setCursor(idx, LCDROW2);
    lcd.print(buffer[idx+5]);
  }
  return;
}


void DisplayChannelNum()
{
  int idx;
  char buffer[128];
  boolean flag;
  
  // get the channel num from the scanner
  Serial.print("LCD CHN\r");
  do
  {
    flag = ReadResponse(buffer, 10);
  }
  while (!flag);
 
  // clear the line and display it
  for (idx=0; idx<3; idx++)
  {
    lcd.setCursor(idx+1, LCDROW1);
    lcd.print(buffer[idx+5]);
  }
  return;
}

void DisplayMode()
{
  char buffer[128];
  boolean flag;
  boolean am;
  boolean fm;
  boolean wfm;
  
  // ask about AM from the scanner
  Serial.print("LCD AM\r");
  do
  {
    flag = ReadResponse(buffer, 5);
  }
  while (!flag);
  if (buffer[3] == '+')
  {
    lcd.setCursor(10, LCDROW2);
    lcd.print("AM");
    return;
  }
  
  // ask about FM from the scanner
  Serial.print("LCD FM\r");
  do
  {
    flag = ReadResponse(buffer, 5);
  }
  while (!flag);
  if (buffer[3] == '+')
  {
    lcd.setCursor(10, LCDROW2);
    lcd.print("NFM");
    return;
  }
  
    // ask about WFM from the scanner
  Serial.print("LCD WFM\r");
  do
  {
    flag = ReadResponse(buffer, 6);
  }
  while (!flag);
  if (buffer[4] == '+')
  {
    lcd.setCursor(10, LCDROW2);
    lcd.print("WFM");
    return;
  }
  
  lcd.setCursor(11, LCDROW2);
  lcd.print("???");
  return;
 
}


void DisplayChannelTag()
{
  char buffer[128];
  boolean flag;
  
  // get the channel tag from the scanner
  Serial.print("LCD LINE1\r");
  do
  {
    flag = ReadResponse(buffer, 43);
  }
  while (!flag);
  
  // clear the line and display it
  ClearLCDLine(LCDROW3);
  DisplayTagBuffer(buffer, LCDROW3);
  return;
}


void DisplayBankTag()
{
  char buffer[128];
  boolean flag;
  
  // get the channel tag from the scanner
  Serial.print("LCD LINE2\r");
  do
  {
    flag = ReadResponse(buffer, 43);
  }
  while (!flag);
  
  // clear the line and display it
  ClearLCDLine(LCDROW4);
  DisplayTagBuffer(buffer, LCDROW4);
  return;
}


// if (deb.checkInput() == HIGH)



boolean ReadResponse(char buffer[], int minBytes)
{
int idx;
char c;
//    ClearLCDLine(LCDROW1);
//    lcd.setCursor(15, LCDROW1);
//    lcd.print(Serial.available());
//    
//    lcd.setCursor(15, LCDROW1);
//    c = Serial.peek();
//    lcd.print(c);
//    lcd.setCursor(10, LCDROW1);
//    lcd.print("M:");
//    lcd.setCursor(12, LCDROW1);
//    lcd.print(minBytes);
    
  if (Serial.available() < minBytes)
  {
    return false;
  }
  else
  {
    idx = 0;
    while (Serial.available() > 0)
    {
      buffer[idx++] = Serial.read();
    }
    buffer[idx] = 0;
    return true;
  }
}


boolean IsSquelchOpen()
{
  char buffer[128];
  boolean flag;
  int idx;

  Serial.print("SQ\r");
  do
  {
    flag = ReadResponse(buffer, 2);
  }
  while (!flag);
 
  if (buffer[0] == '+')
  {
    return true;
  }
  else
  {
    return false;
  }
}

void SendCommand(int cmd)
{
  char buffer[128];
  boolean flag;
  
  // send the command
  switch (cmd)
  {
    case CMD_SCAN:
      Serial.print("KEY00\r");
      break;
      
    case CMD_MAN:
      Serial.print("KEY01\r");
      break;

    case CMD_UP:
      Serial.print("KEY07\r");
      break;
      
    case CMD_DOWN:
      Serial.print("KEY08\r");
      break;
      
    case CMD_SQDENOTIFY:
      Serial.print("QUF\r");
      break;
  }

  // now get the data (OK<cr>), which we will discard
  do
  {
    flag = ReadResponse(buffer, 3);
  }
  while (!flag);
   
}


//
// LCD functions
//

void DisplayScanMessage()
{
  ClearLCDLine(LCDROW2);
  lcd.setCursor(6, LCDROW2);
  lcd.print("SCANNING");
  return;
}

void DisplayManualMessage()
{
  ClearLCDLine(LCDROW1);
  lcd.setCursor(7, LCDROW1);
  lcd.print("MANUAL");
  return;
}

void DisplaySetupMessage()
{
  lcd.setCursor(2, LCDROW2);
  lcd.print("Scanner Setup...");
  return;
}

// Clear the specified line of the LCD
void ClearLCDLine(int line)
{
  lcd.setCursor(0, line);
  lcd.print("                    ");
  return;
}

// Display the opening title
void DisplayTitle()
{
  lcd.setCursor(5, LCDROW1);
  lcd.print("BC-780 XLT");
  lcd.setCursor(4, LCDROW2);
  lcd.print("Remote  Head");
  lcd.setCursor(2, LCDROW3);
  lcd.print("(c) Erik Orange");
  lcd.setCursor(7, LCDROW4);
  lcd.print("KA3FYU");
  delay(2000);
}

// Display the channel tag, or bank tag, on the specified LCD line
void DisplayTagBuffer(char buffer[], int lcdLine)
{
  int idx;
  
  ClearLCDLine(lcdLine);
  for (idx=0; idx<16; idx++)
  {
    lcd.setCursor(idx, lcdLine);
    lcd.print(buffer[idx+7]);
  }
  return;
}


