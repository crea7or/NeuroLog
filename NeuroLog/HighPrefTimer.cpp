// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "HighPrefTimer.h"


HighPrefTimer::HighPrefTimer()
{
	QueryPerformanceFrequency(&Frequency);
}

HighPrefTimer::~HighPrefTimer()
{
}

void HighPrefTimer::Start()
{
	QueryPerformanceCounter(&StartingTime);
}

void HighPrefTimer::Stop()
{
	QueryPerformanceCounter(&EndingTime);
}

DWORD64 HighPrefTimer::GetResult()
{
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	return ElapsedMicroseconds.QuadPart;
}