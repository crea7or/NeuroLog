// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

class HighPrefTimer
{
public:
	HighPrefTimer();
	~HighPrefTimer();

	void Start();
	void Stop();

	DWORD64 GetResult();

private:

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds, Frequency;
};

