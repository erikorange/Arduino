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

int anPos;
long anCtr;

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

  
  // Configure serial port
  Serial.begin(19200);
}


void loop()
{
  boolean squelchIsOpen;
  long mark;
  
  Debounce scanButton(BUTTON_SCAN, DEBOUNCE_TIME);
    
    
  // Display opening title
  DisplayTitle();
  
  // Display setup message
  lcd.clear();
  DisplaySetupMessage();
  
  SendCommand(CMD_SQDENOTIFY);  // ensure BC780 doesn't send an async +/- for squelch
  lcd.clear();
  DisplayScanMessage();
  squelchIsOpen = false;
  anCtr = 0;
  anPos = 0;
  
  SendCommand(CMD_SCAN);      // start scanning
  
  while (true)
  {
    if (IsSquelchOpen())
    {
      // only execute this section once when sq is first detected open
      if (!squelchIsOpen)
      {
        squelchIsOpen = true;
        delay(150);               // give the scanner time to actually switch to manual mode
        DisplayPriority();
        DisplayChannelNum();
        DisplayBanks();
        DisplayFrequency();
        DisplayMode();
        DisplayChannelTag();
        DisplayBankTag();
      }
      
      // continually execute this code when the squelch is open
      // update signal strength
      DisplaySignalStrength();
      
      // display banks
      DisplayBanks();
      
      // go scan if button is pushed
      if (scanButton.checkInput() == HIGH)
      {
         SendCommand(CMD_SCAN);  
      }
      
    }  // squelch is open
    
    else
    {
      // only execute this section once when sq is first detected closed
      if (squelchIsOpen)
      {
        squelchIsOpen = false;
        
        // wait for 2 seconds seconds (default BC780 delay time) while updating the signal strength
        mark = millis();
        while (millis() - mark < 2000)
        {
          DisplaySignalStrength();
          DisplayBanks();
        }
        
        ClearLCDLine(LCDROW1);  //TODO - needed?
        ClearLCDLine(LCDROW2);
        DisplayScanMessage();
        ClearLCDLine(LCDROW4);
        anCtr = 0;
        anPos = 0;
      }
      
      // continually execute this code when the squelch is closed
      // update the channel number
      DisplayPriority();
      DisplayChannelNum();
      DisplayBanks();
      
      // animate scanning display
      if (millis() - anCtr > 250)
      {
	lcd.setCursor(anPos, LCDROW3);
	lcd.print(" ");
	lcd.setCursor(19-anPos, LCDROW3);
	lcd.print(" ");
	anPos++;
	if (anPos == 6)
	{
	  anPos = 0;
	}
	lcd.setCursor(anPos, LCDROW3);
	lcd.print(">");
	lcd.setCursor(19-anPos, LCDROW3);
	lcd.print("<");
	anCtr = millis();
      }
      
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
  
  // get the signal strength from the scanner
  Serial.print("LCD SMT\r");
  flag = ReadResponse(buffer, 11);

  
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
 
  // get the freq from the scanner
  Serial.print("LCD FRQ\r");
  flag = ReadResponseFrq(buffer);

  // clear the line and display it
  ClearLCDLine(LCDROW2);
  for (idx=0; idx<9; idx++)
  {
    // TODO - catch if there were 15, but we went too far?
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
  flag = ReadResponse(buffer, 10);
 
  // clear the line and display it
  for (idx=0; idx<3; idx++)
  {
    lcd.setCursor(idx+1, LCDROW1);
    lcd.print(buffer[idx+5]);
  }
  return;
}


void DisplayPriority()
{
  int idx;
  char buffer[128];
  boolean flag;
  
  // get the channel num from the scanner
  Serial.print("LCD P\r");
  flag = ReadResponse(buffer, 4);
 
  lcd.setCursor(0, LCDROW1);
  if (buffer[2] == '-')
  {
    lcd.print(" ");
  }
  else
  {
    lcd.print("P");
  }
  return;
}





void DisplayBanks()
{
  int idx;
  int dispIdx;
  char buffer[128];
  boolean flag;
  
  // get the active and scanning banks from the scanner
  Serial.print("LCD BNK\r");
  flag = ReadResponse(buffer, 15);
 
  // display it
  lcd.setCursor(6, LCDROW1);
  lcd.print("[");
  dispIdx = 7;
  
  // read status of 10 banks
  for (idx=0; idx<10; idx++)
  {
    lcd.setCursor(dispIdx++, LCDROW1);
    if (buffer[idx+4] == '-')        // bank is off
    {
      lcd.print(" ");
    }
    else if (buffer[idx+4] == '+')  // bank is on
    {
      if (idx == 9)  // display 0 instead of 10
      {
        lcd.print("0");
      }
      else
      {
        lcd.print(idx+1);  //print bank #
      }
    }
    else if (buffer[idx+4] == '*')  // bamk is currently scanning
    {
      lcd.print("*");
    }
  }
  lcd.setCursor(dispIdx, LCDROW1);
  lcd.print("]");
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
  flag = ReadResponse(buffer, 5);
  
  if (buffer[3] == '+')
  {
    lcd.setCursor(10, LCDROW2);
    lcd.print("AM");
    return;
  }
  
  // ask about FM from the scanner
  Serial.print("LCD FM\r");
  flag = ReadResponse(buffer, 5);
  
  if (buffer[3] == '+')
  {
    lcd.setCursor(10, LCDROW2);
    lcd.print("NFM");
    return;
  }
  
    // ask about WFM from the scanner
  Serial.print("LCD WFM\r");
  flag = ReadResponse(buffer, 6);

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
  flag = ReadResponse(buffer, 43);

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
  flag = ReadResponse(buffer, 43);
  
  // clear the line and display it
  ClearLCDLine(LCDROW4);
  DisplayTagBuffer(buffer, LCDROW4);
  return;
}






boolean ReadResponse(char buffer[], int minBytes)
{
int idx;
boolean timeout;
long mark;
int avail;

  // wait until the mininum number of bytes has arrived.  Flag a timeout if it takes too long.
  timeout = false;
  mark = millis();
  while ((Serial.available() < minBytes) && (!timeout))
  {
    if (millis() - mark > 500)
    {
      timeout = true;
      avail = Serial.available();
    }
  }
  
  // if a timeout occurred, then indicate as such and exit.
  if (timeout)
  {
//    ClearLCDLine(LCDROW1);
//    lcd.setCursor(0, LCDROW1);
//    lcd.print("Tm:");
//    lcd.setCursor(3, LCDROW1);
//    lcd.print(minBytes);
//    
//    lcd.setCursor(6, LCDROW1);
//    lcd.print(avail);
//    
//    lcd.setCursor(10, LCDROW1);
//    lcd.print(Serial.peek());
//    
//    while (true)
//    {
//    }
    return false;
  }
  
  // fill the buffer and return
  idx = 0;
  while (Serial.available() > 0)
  {
    buffer[idx++] = Serial.read();
  }
  // TODO:  don't need this next statement?
  buffer[idx] = 0;
  return true;
}


// version just for reading the frq
// could get back 15 or 16, so be tolerant and accept 15, but read 16 if necessary.
// 15 = no freq returned
// 16 = freq returned

boolean ReadResponseFrq(char buffer[])
{
int idx;
boolean timeout;
long mark;

  // wait until the mininum number of bytes has arrived.  Flag a timeout if it takes too long.
  timeout = false;
  mark = millis();
  while ((Serial.available() < 16) && (!timeout))
  {
    if (millis() - mark > 500)
    {
      timeout = true;
    }
  }
  
  // available bytes is either 15 or 16.  Get whatever is in the buffer.
  idx = 0;
  while (Serial.available() > 0)
  {
    buffer[idx++] = Serial.read();
  }
  // TODO:  don't need this next statement?
  buffer[idx] = 0;
  return true;
}






boolean IsSquelchOpen()
{
  char buffer[128];
  boolean flag;
  int idx;

  Serial.print("SQ\r");
  flag = ReadResponse(buffer, 2);
 
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
    flag = ReadResponse(buffer, 3);
    return;   
}


//
// LCD functions
//

void DisplayScanMessage()
{
  ClearLCDLine(LCDROW3);
  lcd.setCursor(6, LCDROW3);
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



