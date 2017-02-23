// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "ChronoTimer.h"
#include <chrono>

void ChronoTimer::Start()
{
	startTime = std::chrono::high_resolution_clock::now();
}

void ChronoTimer::Stop()
{
	endTime = std::chrono::high_resolution_clock::now();
}

std::chrono::milliseconds ChronoTimer::GetElapsedMillisecondsRaw()
{
	return std::chrono::duration_cast< std::chrono::milliseconds >( endTime - startTime );
}

std::wstring ChronoTimer::GetElapsedMilliseconds()
{
	std::wostringstream stringStream;
	stringStream << GetElapsedMillisecondsRaw().count() << " ms";
	return std::wstring( stringStream.str() );
}

std::wstring ChronoTimer::GetElapsedSeconds()
{
	std::wostringstream stringStream;
	stringStream << ((double)GetElapsedMillisecondsRaw().count() / 1000 ) << " s";
	return std::wstring( stringStream.str() );
}