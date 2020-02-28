#include "NixieLib.h"

Clock::Clock(int En, int Buzz, int Button)
{
	EnablePin = En;
	BuzzerPin = Buzz;
	ButtonPin = Button;
}

AP::AP(int ToneI, int DelayI)
{
	Tone = ToneI;
	Delay = DelayI;
}

void Clock::Init()
{
	pinMode(HourReg.SER, OUTPUT);
	pinMode(HourReg.RCLK, OUTPUT);
	pinMode(HourReg.SRCLK, OUTPUT);

	pinMode(MinReg.SER, OUTPUT);
	pinMode(MinReg.RCLK, OUTPUT);
	pinMode(MinReg.SRCLK, OUTPUT);

	pinMode(EnablePin, OUTPUT);
	pinMode(BuzzerPin, OUTPUT);
	pinMode(ButtonPin, INPUT);

	SetHours(0);
	SetMin(0);
}

void Clock::Loop()
{
	//Check Button
	if (digitalRead(ButtonPin))
	{
		ButtonPress();
	}

	//Play Alarm
	if (bShouldPlayAlarm)
	{
		PlayAlarm();
	}

	//Set New Minute
	if (millis() / 1000 >= SecsAtNextMin)
	{
		SecsAtNextMin = millis() / 1000 + 60;

		Minutes == 59 ? Minutes = 0, Hours++ : Minutes++;
		Hours <= 23 ? : Hours = 0;
		SetHours(Hours);
		SetMin(Minutes);

		checkAlarm();
	}

	if (bShowAlarmOnClock)
	{
		if (millis() - WhenWasAlarmShowed >= DelayToShowAlarm)
		{

			SetHours(Hours);
			SetMin(Minutes);

			bShowAlarmOnClock = false;

		}
		else
		{

			SetHours(AlarmH);
			SetMin(AlarmM);
		}
	}
}

void Clock::SetEnable(bool En)
{
	digitalWrite(EnablePin, En);
}

void Clock::Buzz(int Tone)
{
	analogWrite(BuzzerPin, Tone);
}

void Clock::ButtonPress()
{
	if (ClockState == State::Normal)
	{
		if (bIsAlarmSet)
		{
        BlinkAlarm();
		}
	}
	else if (ClockState == State::Alarm)
	{
		ResetAlarm();
	}
}

void Clock::WriteLL(int Number, ShiftRegister Reg)
{
	if ((Number <= 99 and Number >= 0))
	{
		int Enica = Number % 10;
		int Desetica = Number / 10;

		byte EnicaByte = NumberCodes[Enica];
		byte DeseticaByte = NumberCodes[Desetica];

		//Desetice
		for (int i = 0; i < 4; i++)
		{
			digitalWrite(Reg.SER, DeseticaByte & B1000);
			DeseticaByte <<= 1;

			digitalWrite(Reg.SRCLK, LOW);
			digitalWrite(Reg.SRCLK, HIGH);
		}

		//Enice
		for (int i = 0; i < 4; i++)
		{
			digitalWrite(Reg.SER, EnicaByte & B1000);
			EnicaByte <<= 1;

			digitalWrite(Reg.SRCLK, LOW);
			digitalWrite(Reg.SRCLK, HIGH);
		}

		PushToReg(Reg);
	}
}

void Clock::PushToReg(ShiftRegister Reg)
{
	digitalWrite(Reg.RCLK, LOW);
	digitalWrite(Reg.RCLK, HIGH);
}

void Clock::SetHours(int Hours)
{
	WriteLL(Hours, HourReg);
}

void Clock::SetMin(int Mins)
{
	WriteLL(Mins, MinReg);
}

void Clock::SetTime(int unix)
{
	Minutes = unix / 60 % 60;
	Hours = unix / 60 / 60 % 24;

	SecsAtNextMin = millis() / 1000 + 60 - unix % 60;

	Hours != 23 ? Hours++ : Hours = 0;

	SetHours(Hours);
	SetMin(Minutes);

}

void Clock::SetAlarm(int H, int M)
{
	AlarmH = H;
	AlarmM = M;

	bIsAlarmSet = true;
}

void Clock::PlayAlarm()
{
	static int LastTimeAlarmPlay = 0;
	if (millis() - LastTimeAlarmPlay >= AlarmParts[PartOfAlarm].Delay)
	{
		Buzz(AlarmParts[PartOfAlarm].Tone);
		PartOfAlarm < NumOfAlarmParts - 1 ? PartOfAlarm++ : PartOfAlarm = 0;

		LastTimeAlarmPlay = millis();

		bIsEnabled = !bIsEnabled;
		SetEnable(bIsEnabled);
	}
}

void Clock::checkAlarm()
{
	if (bIsAlarmSet && Hours == AlarmH && Minutes == AlarmM)
	{
		ClockState = State::Alarm;
		bShouldPlayAlarm = true;
	}
}

void Clock::BlinkAlarm()
{
	bShowAlarmOnClock = true;
	WhenWasAlarmShowed = millis();
}

void Clock::ResetAlarm()
{
	Buzz(0);

	ClockState = State::Normal;
	bShouldPlayAlarm = false;
	bIsAlarmSet = false;
	bIsEnabled = true;
	SetEnable(bIsEnabled);
}
