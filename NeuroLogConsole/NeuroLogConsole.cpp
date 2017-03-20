// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

// NeuroLogConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>

void PrintParamsHelp()
{
	std::wcout << std::endl;
	std::wcout << L" -sub path     - subnets folder" << std::endl;
	std::wcout << L" -log path     - logs folder" << std::endl;
	std::wcout << L" -out path     - output folder" << std::endl;
	std::wcout << L" -mask *.log   - mask for log files in log folder" << std::endl;
	std::wcout << L" -hitl number  - minimum hits from subnet to report (optional)" << std::endl;
	std::wcout << L" -sizel number - minimum download size from subnet to report (optional)" << std::endl;
}

int _tmain( int argc, _TCHAR* argv[] )
{
	std::wcout << L"NeuroLog 1.1 by CEZEO software Ltd. http://www.cezeo.com" << std::endl;

	NewCore();

	std::wstring subnetsFolder, logsFolder, outputFolder, logMask;
	uint32 limitHits = 200, limitSize = 2000000;

	for ( int count = 0; count < argc; count++ )
	{
		//std::wcout << L"  argv[" << count << L"]   " << std::wstring( argv[ count ] ) << L"\n";
		if ( std::wstring( argv[ count ] ) == L"-sub" && (( count + 1 ) < argc ))
		{
			subnetsFolder = std::wstring( argv[ count + 1 ] );
			++count;
		}

		if ( std::wstring( argv[ count ] ) == L"-log" && ( ( count + 1 ) < argc ) )
		{
			logsFolder = std::wstring( argv[ count + 1 ] );
			++count;
		}

		if ( std::wstring( argv[ count ] ) == L"-out" && ( ( count + 1 ) < argc ) )
		{
			outputFolder = std::wstring( argv[ count + 1 ] );
			++count;
		}

		if ( std::wstring( argv[ count ] ) == L"-mask" && ( ( count + 1 ) < argc ) )
		{
			logMask = std::wstring( argv[ count + 1 ] );
			++count;
		}

		if ( std::wstring( argv[ count ] ) == L"-hitl" && ( ( count + 1 ) < argc ) )
		{
			limitHits = std::stoul( std::wstring( argv[ count + 1 ] ) );
			++count;
		}

		if ( std::wstring( argv[ count ] ) == L"-sizel" && ( ( count + 1 ) < argc ) )
		{
			limitSize = std::stoul( std::wstring( argv[ count + 1 ] ) );
			++count;
		}
	}

	if ( subnetsFolder.size() < 1 )
	{
		std::wcout << L"No subnets folder defined" << std::endl;
		PrintParamsHelp();
		return -1;
	}
	if ( logsFolder.size() < 1 )
	{
		std::wcout << L"No logs folder defined" << std::endl;
		return -1;
	}
	if ( outputFolder.size() < 1 )
	{
		std::wcout << L"No logs folder defined" << std::endl;
		return -1;
	}
	if ( logMask.size() < 1 )
	{
		std::wcout << L"No mask for log files defined" << std::endl;
		return -1;
	}

	GetCore()->hitsLimit = limitHits;
	GetCore()->sizeLimit = limitSize;

	GetCore()->subnetsFolder = subnetsFolder;
	GetCore()->logsFolder = logsFolder;
	GetCore()->outputFolder = outputFolder;
	GetCore()->logsMask = logMask;

	GetCore()->LoadSubnets();
	GetCore()->LoadLogs();
	GetCore()->AnalyzeSubnets();
	GetCore()->ClearData();

	DeleteCore();

	return 0;
}

