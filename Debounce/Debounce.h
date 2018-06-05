/*
  Debounce.h - Library for debouncing a switched input.
  Created by Erik Orange, 1/16/2012.
  erikorange@gmail.com
*/
#ifndef Debounce_h
#define Debounce_h

#include "WProgram.h"

class Debounce
{
  public:
  	debounce(int pin, long debounceWidth);
	int checkInput();
  private:
	int _pin;
	long _debounceWidth;
	int _buttonState;
	int _previousButtonState;
	int _currentReading;
	long _lastDebounceTime;
};

#endif