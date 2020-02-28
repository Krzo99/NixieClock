#pragma once
#include "Stdio.h"
#include "Arduino.h"


enum State {
	Normal,
	Alarm
};

struct ShiftRegister {
	int SER;
	int RCLK;
	int SRCLK;
};

//Alarm Part
class AP {

public:
	AP(int ToneI, int DelayI);

	int Tone = 0;
	int Delay = 0;

};

class Clock
{
public:
	Clock(int EnablePin, int BuzzerPin, int ButtonPin);

	void SetEnable(bool En);

	//Pins
	int EnablePin = -1;
	int BuzzerPin = -1;
	int ButtonPin = -1;

	void WriteLL(int Number, ShiftRegister Reg);
	void PushToReg(ShiftRegister Reg);

	void Init();
	void Loop();
	void SetHours(int Hours);
	void SetMin(int Mins);
	void SetTime(int unix);
	void Buzz(int Tone);
	void ButtonPress();
	

	unsigned int SecsAtNextMin = 1000;
	int Minutes = 0;
	int Hours = 0;
	State ClockState = State::Normal;
	bool bIsEnabled = true;

	ShiftRegister HourReg = { D4, D2, D3 };
	ShiftRegister MinReg = { D5, D8, D7 };
	byte NumberCodes[10] = { B0000, B0001, B0010, B0011, B0100, B0101, B0110, B0111, B1000, B1001 };

	//Alarm
	void SetAlarm(int Hours, int Min);
	void PlayAlarm();
	void checkAlarm();
	void ResetAlarm();
	void BlinkAlarm();
	bool bIsAlarmSet = false;
	int AlarmH = 0;
	int AlarmM = 0;
	bool bShouldPlayAlarm = false;
	bool bShowAlarmOnClock = false;
	int DelayToShowAlarm = 5000;
	int WhenWasAlarmShowed = 0;

	//Music
	int PartOfAlarm = 0;
	const static unsigned int NumOfAlarmParts = 4;
	AP AlarmParts[NumOfAlarmParts] = { {256, 500}, {20, 100},  {160, 350},  {75, 100} };


};

