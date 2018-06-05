#include <LiquidCrystal.h>

// LCD rows
const int LCDROW1 = 0;
const int LCDROW2 = 1;
const int LCDROW3 = 2;
const int LCDROW4 = 3;

// I/O pins
const int BUTTON_DOWN = 6;
const int BUTTON_UP = 7;
const int BUTTON_MAN = 8;
const int BUTTON_SCAN = 9;
const int BACKLIGHT = 13;

// Definitions for button HIGH/LOW based on using pullup resistors
const int BUTTON_OPEN = HIGH;
const int BUTTON_CLOSED = LOW;


// BC780 serial commands that return OK as response
const int CMD_SCAN = 1;      // scan mode
const int CMD_MAN = 2;       // manual mode
const int CMD_UP = 3;        // channel Up
const int CMD_DOWN = 4;      // channel Down
const int CMD_SQDENOTIFY = 5;  // squelch notify
const int CMD_LOCKOUT_ON = 6;  // turn lockout on for channel
const int CMD_LOCKOUT_OFF = 7; // turn lockout off for channel


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

byte bank10[8] = {
  B10111,
  B10101,
  B10101,
  B10101,
  B10101,
  B10101,
  B10111,
  B00000,
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
  // Configure input pushbuttons
  pinMode(BUTTON_DOWN, INPUT);
  digitalWrite(BUTTON_DOWN, HIGH);     // turn on the built in pull-up resistor
  
  pinMode(BUTTON_UP, INPUT);
  digitalWrite(BUTTON_UP, HIGH);       // turn on the built in pull-up resistor
  
  pinMode(BUTTON_MAN, INPUT);
  digitalWrite(BUTTON_MAN, HIGH);      // turn on the built in pull-up resistor
  
  pinMode(BUTTON_SCAN, INPUT);
  digitalWrite(BUTTON_SCAN, HIGH);     // turn on the built in pull-up resistor

  // Configure backlight
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  // create the Signal Strength chars
  lcd.createChar(0, ss_1);
  lcd.createChar(1, ss_12);
  lcd.createChar(2, ss_3);
  lcd.createChar(3, ss_34);
  lcd.createChar(4, ss_5);
  lcd.createChar(5, ss_56);
  lcd.createChar(6, ess);    // the little s
  
  // create the "10" character for bank 10
  lcd.createChar(7, bank10);
  
  // define LCD as 20x4
  lcd.begin(20, 4);
  lcd.clear();

  // Configure serial port
  Serial.begin(19200);
}


void loop()
{

  // check for diagnostic mode
  if (digitalRead(BUTTON_UP) == BUTTON_CLOSED && digitalRead(BUTTON_DOWN) == BUTTON_CLOSED)
  {
    DiagMode();  // this function never returns
  }
    
  // Display opening title
  DisplayTitle();
  
  
  // Detect the scanner.  If not detected, then go to diagnostic mode.
  if (!DetectScanner())
  {
    DiagMode();
  }
  
  
  // start scanning, and alternate to manual mode as required.
  while (true)
  {
    ScanningMode();
    ManualMode();
  }
}


void ManualMode()
{
  boolean manualMode;
  
  manualMode = true;
  SendCommand(CMD_MAN);
  ClearLCDLine(LCDROW1);    // clear whatever active banks that were displayed
  UpdateDisplay();
  DisplayLockout();

  do
  {
    DisplaySignalStrength();
    
    // check if UP button was pressed
    if (digitalRead(BUTTON_UP) == BUTTON_CLOSED)
    {
      SendCommand(CMD_UP);
      UpdateDisplay();
      DisplayLockout();
    }
    
    // check if DOWN button was pressed
    if (digitalRead(BUTTON_DOWN) == BUTTON_CLOSED)
    {
      SendCommand(CMD_DOWN);
      UpdateDisplay();
      DisplayLockout();
    }
    
    // check if MAN button was pressed; this toggles lockout
    if (digitalRead(BUTTON_MAN) == BUTTON_CLOSED)
    {
      if (DisplayLockout())    // channel is already locked out, so unlock
      {
        SendCommand(CMD_LOCKOUT_OFF);
        DisplayLockout();      // lockouts are only displayed in manual mode
      }
      else
      {
        SendCommand(CMD_LOCKOUT_ON);
        DisplayLockout();      // lockouts are only displayed in manual mode
      }
      delay(200);
    }
    
    // check if SCAN button was pressed
    if (digitalRead(BUTTON_SCAN) == BUTTON_CLOSED)
    {
      manualMode = false;
    }
    
    // check if banks should be toggled
    CheckBankSetMode();

  }
  while (manualMode);
  
  return; 
}


void ScanningMode()
{
  boolean scanMode;
  boolean squelchIsOpen;
  boolean forceImmediateResume;
  long mark;
  int anPos;
  long anCtr;

  lcd.clear();
  DisplayScanMessage();
  squelchIsOpen = false;
  anCtr = 0;
  anPos = 0;
  SendCommand(CMD_SCAN);      // start scanning
  
  scanMode = true;
  do
  {
    if (IsSquelchOpen())
    {
      // only execute this section once when sq is first detected open
      if (!squelchIsOpen)
      {
        squelchIsOpen = true;
        forceImmediateResume = false;
        delay(150);               // give the scanner time to actually switch to manual mode
        UpdateDisplay();
      } // sq is first detected open
      
      // continually execute this code when the squelch is open
      DisplaySignalStrength();
      DisplayBanks();
      
      // check if banks should be toggled
      if (CheckBankSetMode())
      {
        DisplayScanMessage();
      }
      
      
      // check if we should lock out this channel
      if (digitalRead(BUTTON_UP) == BUTTON_CLOSED)
      {
         forceImmediateResume = true;
         SendCommand(CMD_LOCKOUT_ON);
         SendCommand(CMD_SCAN);
      }
      
      
      // go scan if button is pushed
      if (digitalRead(BUTTON_SCAN) == BUTTON_CLOSED)
      {
         forceImmediateResume = true;
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
        // unless we got here thanks to a channel lockout, in which case resume scanning immediately
        if (!forceImmediateResume)
        {
          mark = millis();
          while (millis() - mark < 2000)
          {
            DisplaySignalStrength();
            DisplayBanks();
          }
        }
        
        ClearLCDLine(LCDROW1);
        ClearLCDLine(LCDROW2);
        DisplayScanMessage();
        ClearLCDLine(LCDROW4);
        
        anCtr = 0;
        anPos = 0;
      }  // sq is first detected closed
      
      // continually execute this code when the squelch is closed
      // update the channel number
      DisplayPriority();
      DisplayChannelNum();
      DisplayBanks();
      
      // check if banks should be toggled
      if (CheckBankSetMode())
      {
        DisplayScanMessage();
      }
      
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
    
     
    // bail if man button is pushed
    if (digitalRead(BUTTON_MAN) == BUTTON_CLOSED)
    {
      scanMode = false;  
    }
    
  }
  while (scanMode);
  
  return;
}


void UpdateDisplay()
{
  DisplayPriority();
  DisplayChannelNum();
  DisplayBanks();
  DisplayFrequency();
  DisplayMode();
  DisplayChannelTag();
  DisplayBankTag();
  return;
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


boolean DisplayLockout()
{
  int idx;
  char buffer[128];
  boolean flag;
  
  // get the channel lockout status from the scanner
  Serial.print("LO\r");
  flag = ReadResponse(buffer, 4);
 
  lcd.setCursor(5, LCDROW1);
  if (buffer[2] == 'N')
  {
    lcd.print("L");
    return true;
  }
  lcd.print(" ");
  return false;
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
  
  // read status of all 10 banks
  dispIdx = 7;
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
        lcd.write(7);  // bank 10 character
      }
      else
      {
        lcd.print(idx+1);  //print bank #
      }
    }
    else if (buffer[idx+4] == '*')  // bamk is currently scanning
    {
      lcd.print("<");
      lcd.setCursor(dispIdx++, LCDROW1);
      if (idx == 9)  // display 0 instead of 10
      {
        lcd.write(7);  // bank 10 character
      }
      else
      {
        lcd.print(idx+1);  //print bank #
      }
      lcd.setCursor(dispIdx++, LCDROW1);
      lcd.print(">");
    }
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
  lcd.print("   ");
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

boolean DetectScanner()
{
  char buffer[128];
  boolean flag;
  boolean stillWaiting;
  boolean notDetected;
  long timeoutCtr;
  
  long anCtr;
  int anIdx;
  
  lcd.clear();
  lcd.setCursor(0, LCDROW2);
  lcd.print("Detecting Scanner");
  
  notDetected = true;
  stillWaiting = true;
  
  anCtr = 0;
  anIdx = 0;
  
  // start our 10 seconc countdown
  timeoutCtr = millis();
  anCtr = millis();
  
  do
  {
    // animate the periods...
    if (millis() - anCtr > 333)
    {
      lcd.setCursor(17, LCDROW2);
      switch (anIdx)
      {
        case 0:
        lcd.print(".  ");
        break;
        
        case 1:
        lcd.print(".. ");
        break;
        
        case 2:
        lcd.print("...");
        break;
        
        case 3:
        lcd.print("   ");
        break;
      }
      
      if (++anIdx == 4)
      {
        anIdx = 0;
      }
       
      anCtr = millis();
    }
      
    // tell BC780 to disable squelch notify mode
    Serial.print("QUF\r");
    
    // now see if we got the data (OK<cr>)
    flag = ReadResponse(buffer, 3);
    if (flag && (buffer[0]=='O' && buffer[1]=='K' && buffer[2]=='\r'))
    {
      notDetected = false;
    }
    
    // bail if 10 seconds has passed
    if (millis() - timeoutCtr > 10000)
    {
      stillWaiting = false;
    }

  }
  while (notDetected && stillWaiting);
  
  // somehow we got here
  // Did we get the response?
  if (notDetected == false)
  {
    return true;  // all is well;
  }
  
  return false;  // we timed out
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
    return false;
  }
  
  // fill the buffer and return
  idx = 0;
  while (Serial.available() > 0)
  {
    buffer[idx++] = Serial.read();
  }
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
      
    case CMD_LOCKOUT_ON:
      Serial.print("LON\r");
      break;
      
    case CMD_LOCKOUT_OFF:
      Serial.print("LOF\r");
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
  lcd.setCursor(4, LCDROW3);
  lcd.print("Version 2.20");
  lcd.setCursor(2, LCDROW4);
  lcd.print("(c) Erik Orange");
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


void DiagMode()
{
  lcd.clear();
  lcd.setCursor(2, LCDROW1);
  lcd.print("Diagnostic  Mode");
  lcd.setCursor(0, LCDROW2);
  lcd.print("SCAN:       UP:");
  lcd.setCursor(0, LCDROW3);
  lcd.print(" MAN:     DOWN:");
  
  while (true)
  {
    // Scan button
    if (digitalRead(BUTTON_SCAN) == BUTTON_OPEN)
    {
      lcd.setCursor(5, LCDROW2);
      lcd.print("OFF");
    }
    else
    {
      lcd.setCursor(5, LCDROW2);
      lcd.print("ON ");
    }
    
    // Man button
    if (digitalRead(BUTTON_MAN) == BUTTON_OPEN)
    {
      lcd.setCursor(5, LCDROW3);
      lcd.print("OFF");
    }
    else
    {
      lcd.setCursor(5, LCDROW3);
      lcd.print("ON ");
    }
    
    // Up button
    if (digitalRead(BUTTON_UP) == BUTTON_OPEN)
    {
      lcd.setCursor(15, LCDROW2);
      lcd.print("OFF");
    }
    else
    {
      lcd.setCursor(15, LCDROW2);
      lcd.print("ON ");
    }
    
    // Down button
    if (digitalRead(BUTTON_DOWN) == BUTTON_OPEN)
    {
      lcd.setCursor(15, LCDROW3);
      lcd.print("OFF");
    }
    else
    {
      lcd.setCursor(15, LCDROW3);
      lcd.print("ON ");
    }
  }
  
  
}



// toggle the banks
void ToggleBanks()
{
  
  char bankFlags[10];    // in-memory status of banks (on/off)
  char bankBuffer[128];  // serial response from scanner containing bank info
  int pos[] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};  // X positions of bank indicators on LCD
  
  boolean flag;
  int curPos = 0;
  int idx;
  char ch;
  
  // clear the bank Flags array
  for (idx=0; idx<10; idx++)
  {
    bankFlags[idx] = ' ';
  }
  
  // Set up the Display
  lcd.clear();
  lcd.setCursor(0, LCDROW1);
  lcd.print("1 2 3 4 5 6 7 8 9");
  lcd.setCursor(18, LCDROW1);
  lcd.write(7);  // write the 10 char
  lcd.setCursor(0, LCDROW3);
  lcd.print("^");
  lcd.setCursor(2, LCDROW4);
  lcd.print("<    >   Flip Exit");
  
  
  // get the current banks from the scanner
  flag = ReadBanks(bankBuffer);
  
  // Now load in-memory buffer with bank status
  idx = 0;
  while (bankBuffer[idx] != '\r')
  {
    ch = bankBuffer[idx++];
    switch (ch)
    {
      case 'A':
        bankFlags[0] = '*';
        break;
    
      case 'B':
        bankFlags[1] = '*';
        break;
        
      case 'C':
        bankFlags[2] = '*';
        break;

      case 'D':
        bankFlags[3] = '*';
        break;

      case 'E':
        bankFlags[4] = '*';
        break;

      case 'F':
        bankFlags[5] = '*';
        break;

      case 'G':
        bankFlags[6] = '*';
        break;

      case 'H':
        bankFlags[7] = '*';
        break;

      case 'I':
        bankFlags[8] = '*';
        break;
    
      case 'J':
        bankFlags[9] = '*';
        break;            
    }
  }

  // display bank statuses
  DisplayBankStatus(bankFlags, pos);
 
  // monitor all buttons; drop out if Exit button is pressed.
  flag = true;
  while (flag)
  {
    // Left button
    if (digitalRead(BUTTON_SCAN) == BUTTON_CLOSED)
    {
      curPos--;
      if (curPos == -1)
      {
        curPos = 9;
      }
      ClearLCDLine(LCDROW3);
      lcd.setCursor(pos[curPos], LCDROW3);
      lcd.print("^");
      delay(200);
    }
    
    // Right button
    if (digitalRead(BUTTON_MAN) == BUTTON_CLOSED)
    {
      curPos++;
      if (curPos == 10)
      {
        curPos = 0;
      }
      ClearLCDLine(LCDROW3);
      lcd.setCursor(pos[curPos], LCDROW3);
      lcd.print("^");
      delay(200);
    }
    
    // Flip button
    if (digitalRead(BUTTON_UP) == BUTTON_CLOSED)
    {
      if (bankFlags[curPos] == '*')
      {
        bankFlags[curPos] = ' ';
      }
      else
      {
        bankFlags[curPos] = '*';
      }
      DisplayBankStatus(bankFlags, pos);
      delay(200);
    }
    
    
    // Exit button
    if (digitalRead(BUTTON_DOWN) == BUTTON_CLOSED)
    {
      Serial.print("SB ");
      if (bankFlags[0] == '*')
      {
        Serial.print("A");
      }
      if (bankFlags[1] == '*')
      {
        Serial.print("B");
      }
      if (bankFlags[2] == '*')
      {
        Serial.print("C");
      }
      if (bankFlags[3] == '*')
      {
        Serial.print("D");
      }
      if (bankFlags[4] == '*')
      {
        Serial.print("E");
      }
      if (bankFlags[5] == '*')
      {
        Serial.print("F");
      }
      if (bankFlags[6] == '*')
      {
        Serial.print("G");
      }
      if (bankFlags[7] == '*')
      {
        Serial.print("H");
      }
      if (bankFlags[8] == '*')
      {
        Serial.print("I");
      }
      if (bankFlags[9] == '*')
      {
        Serial.print("J");
      }
      Serial.print("\r");
      
      // wait a little, then discard the response.
      delay(500);
      while (Serial.available() > 0)
      {
        ch = Serial.read();
      }
      
      // drop out
      flag = false;
    }

  }
  return;
}


void DisplayBankStatus(char bankFlags[], int pos[])
{
  int i;
  
  for (i=0; i<10; i++)
  {
    lcd.setCursor(pos[i], LCDROW2);
    lcd.print(bankFlags[i]);
  }
  return;
}


boolean ReadBanks(char buffer[])
{
int idx;
char c;

  // ask the scanner what banks are set
  Serial.print("SB\r");
  
  // response will be a variable number of characters.  Wait a little, then get the response.
  delay(500);
  
  // discard the first 3 characters, which will be SB[space]
  c = Serial.read();
  c = Serial.read();
  c = Serial.read();
  
  // Now get whatever is in the buffer, which will be at least 1 bank letter, terminated by a <cr>.
  idx = 0;
  while (Serial.available() > 0)
  {
    buffer[idx++] = Serial.read();
  }
  return true;
}


boolean CheckBankSetMode()
{
  boolean flag;
  
  flag = false;
  // check for bank set mode
  if (digitalRead(BUTTON_SCAN) == BUTTON_CLOSED && digitalRead(BUTTON_DOWN) == BUTTON_CLOSED)
  {
    ToggleBanks();
    lcd.clear();
    flag = true;  // signal that we actually did it
  }
  return flag;
}
