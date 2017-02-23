// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include <chrono>

class ChronoTimer
{
public:

	ChronoTimer() {};
	~ChronoTimer() {};

	void Start();
	void Stop();

	std::wstring GetElapsedSeconds();
	std::wstring GetElapsedMilliseconds();
	std::chrono::milliseconds GetElapsedMillisecondsRaw();

private:

	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;

};

