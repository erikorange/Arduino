/*
  Debounce.cpp - Library for debouncing a switched input.
  Created by Erik Orange, 1/16/2012.
  erikorange@gmail.com
*/

#include "WProgram.h"

Debounce::Debounce(int pin, long debounceWidth)
{
	_pin = pin;
	_debounceWidth = debounceWidth;
	_buttonState = HIGH;
	_previousButtonState = HIGH;
	_currentReading = HIGH;
	_lastDebounceTime = 0;
	pinMode(_pin, INPUT);
}

int Debounce::checkInput()
{
	_currentReading = digitalRead(_pin);

	// If the switch changed, due to bounce or pressing...
	if (_currentReading != _previousButtonState)
	{
		_lastDebounceTime = millis();    // reset the debouncing timer
	}

	if ((millis() - _lastDebounceTime) > _debounceWidth)
	{
		_buttonState = _currentReading;
	}

	// Save the last reading so we keep a running tally
	_previousButtonState = _currentReading;

	// Invert so this appears normal (LOW=open, HIGH=closed)
	if (_buttonState == HIGH)
	{
		return LOW;
	}
	else
	{
		return HIGH;
	}
}

