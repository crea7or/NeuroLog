// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "Core.h"
#include "Ws2tcpip.h"
#include <stdlib.h>
#include "RawSubnet.h"

#define SUBNETS_CACHE_FILE _T( "\\NeuroLog.Subnets" )
#define SUBNETS_STAT _T( "\\NeuroLog.Statistics.html" )

#define HTML_START _T("<html><head><style type=\"text/css\">body,table{font-family:Tahoma,Sans-serif;font-size:10pt;text-align:left}</style></head><body>")
#define HTML_START_DATA _T("<table cellspacing=\"3\" cellpadding=\"5\" width=\"50%\" style=\"float: left;\"><tr bgcolor=\"Moccasin\"><td>Subnet IP(first)</td><td>Country</td><td>Hits</td><td>Traffic</td></tr>")
#define HTML_MID _T("</table><table cellspacing=\"3\" cellpadding=\"5\" width=\"50%\" style=\"float: left;\"><tr bgcolor=\"Moccasin\"><td>Subnet IP(first)</td><td>Country</td><td>Hits</td><td>Traffic</td></tr>")
#define HTML_END _T("</table></body></html>")
#define HTML_BG_CELL_ODD _T("<tr bgcolor=\"#F5F5DC\">")
#define HTML_BG_CELL_EVEN _T("<tr bgcolor=\"#FFF8DC\">")

Core::Core()
{
}

Core::~Core()
{
	for ( auto ptr : ipMap )
	{
		if ( ptr != nullptr )
		{
			delete ptr;
		}
	}
}

void Core::ClearData()
{
	subnetsVector.clear();
	hitsVector.clear();
	uriMap.clear();
	refMap.clear();
	agentsMap.clear();
}

void Core::AnalyzeSubnets()
{
	std::vector< LPSUBNET > subnetsPtrs;
	subnetsPtrs.reserve( subnetsVector.size());

	std::vector<Subnet>::iterator itSub;
	LPSUBNET pSubnet;

	for ( itSub = subnetsVector.begin(); itSub < subnetsVector.end(); itSub++ )
	{
		pSubnet = &*itSub;
		subnetsPtrs.push_back( pSubnet );
	}

	//////////////////////
	// Generate reports //
	///////////////////////

	SubnetsSortHitsCount sortObjCnt;
	std::sort( subnetsPtrs.begin(), subnetsPtrs.end(), sortObjCnt );

	CString logLine;
	CFile reportFile;
	CString subChacheFile = cacheFolder + SUBNETS_STAT;
	reportFile.Open( subChacheFile, CFile::modeWrite | CFile::modeCreate, NULL );

	CString htmlStart( HTML_START );
	CString htmlStartData( HTML_START_DATA );
	CString htmlMid( HTML_MID );
	CString htmlEnd( HTML_END );
	CString htmlCellEven( HTML_BG_CELL_EVEN );
	CString htmlCellOdd( HTML_BG_CELL_ODD );

	ConvertStringAndSave( &htmlStart, &reportFile );

	// Get time interval of analyzed log files
	BitDateTime startDt, endDt;
	startDt.SetRaw( 0xFFFFFFFF );
	for ( auto ht : hitsVector )
	{
		if ( ht.dateTime.GetRaw() < startDt.GetRaw() )
		{
			startDt.SetRaw( ht.dateTime.GetRaw() );
		}
		if ( ht.dateTime.GetRaw() > endDt.GetRaw() )
		{
			endDt.SetRaw( ht.dateTime.GetRaw() );
		}
	}

	logLine.Format( _T( "<p>Interval analyzed: %s  to  %s</p>"), startDt.GetDateTime(), endDt.GetDateTime() );
	ConvertStringAndSave( &logLine, &reportFile );

	logLine.Format( _T( "<p>Log files folder: %s\\%s</p>" ), logsFolder, logsMask );
	ConvertStringAndSave( &logLine, &reportFile );

	// Get time interval of analyzed log files

	ConvertStringAndSave( &htmlStartData, &reportFile );

	DWORD bgColor = 0;
	CString ipv4;
	for ( auto i : subnetsPtrs )
	{
		if ( i->HitsCount() > hitsLimit )
		{
			ipv4.Format( _T( "%d.%d.%d.%d" ), HIBYTE( HIWORD( i->startAddr ) ), LOBYTE( HIWORD( i->startAddr ) ), HIBYTE( LOWORD( i->startAddr ) ), LOBYTE( LOWORD( i->startAddr ) ) );
			logLine.Format( _T( "%s<td><a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:%s\" target=\"_blank\">%s</a></td><td>%c%c</td><td>%d</td><td>%s</td></tr>\n" ), bgColor & 1 ? htmlCellOdd : htmlCellEven, ipv4, ipv4, wchar_t( i->countryId[ 0 ] ), wchar_t( i->countryId[ 1 ] ), i->HitsCount(), MakeBytesSizeString( i->HitsSize() ) );
			ConvertStringAndSave( &logLine, &reportFile );
			++bgColor;
		}
	}


	ConvertStringAndSave( &htmlMid, &reportFile );

	///////////////////////

	SubnetsSortHitsSize sortObjSize;
	std::sort( subnetsPtrs.begin(), subnetsPtrs.end(), sortObjSize );

	bgColor = 0;
	for ( auto i : subnetsPtrs )
	{
		if ( i->HitsSize() > sizeLimit )
		{
			ipv4.Format( _T( "%d.%d.%d.%d" ), HIBYTE( HIWORD( i->startAddr ) ), LOBYTE( HIWORD( i->startAddr ) ), HIBYTE( LOWORD( i->startAddr ) ), LOBYTE( LOWORD( i->startAddr ) ) );
			logLine.Format( _T( "%s<td><a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:%s\" target=\"_blank\">%s</a></td><td>%c%c</td><td>%d</td><td>%s</td></tr>\n" ), bgColor & 1 ? htmlCellOdd : htmlCellEven, ipv4, ipv4, wchar_t( i->countryId[ 0 ] ), wchar_t( i->countryId[ 1 ] ), i->HitsCount(), MakeBytesSizeString( i->HitsSize() ) );
			ConvertStringAndSave( &logLine, &reportFile );
			++bgColor;
		}
	}

	ConvertStringAndSave( &htmlEnd, &reportFile );
	reportFile.Close();

	ShellExecute( NULL, L"open", subChacheFile, NULL, NULL, SW_NORMAL );
}

#pragma region Logs

bool Core::LoadLogs()
{
	bool result = false;

	// To test performance
	CString message;
	HighPrefTimer timer, timer2;
	// To test performance

	CStringArray resultNames;
	GetFilesByMask( &resultNames, logsFolder, logsMask );

	timer2.Start();

	try
	{
		for ( int index = 0; index < resultNames.GetSize(); index++ )
		{
			// To test performance
			//timer.Start();
			// To test performance

			ParseLogFile( resultNames.GetAt( index ) );

			// To test performance
			//timer.Stop();
			//message.Format( _T( "File \"%s\" parse time %u\n" ), resultNames.GetAt( index ), timer.GetResult() );
			//GetCore()->appLog.Add( message );

			// To test performance
		}



		//timer.Start();

		LPSUBNET pSubnet;
		LPHIT pHit;
		DWORD goodHits = 0, badHits = 0;

		std::vector< Hit >::iterator itHit;
		for ( itHit = hitsVector.begin(); itHit < hitsVector.end(); itHit++ )
		{
			pHit = &*itHit;
			if ( ipMap[ HIWORD( pHit->ipv4 ) ] != nullptr )
			{
				pSubnet = ipMap[ HIWORD( pHit->ipv4 ) ]->AddHit( pHit );
				if ( pSubnet != nullptr )
				{
					++goodHits;
				}
				else
				{

				}
			}
			else
			{
				++badHits;
			}
		}

		//message.Format( _T( " GoodHits: %d  BadHits: %d  Tot: %d\n" ), goodHits, badHits, goodHits + badHits );
		//GetCore()->appLog.Add( message );


		//timer.Stop();
		// To test performance
		//message.Format( _T( "Logs parsing by subnets time %u\n" ), timer.GetResult() );
		//GetCore()->appLog.Add( message );
		// To test performance
	}
	catch ( ... )
	{

	}

	timer2.Stop();
	message.Format( _T( "Hits: %d  Time: %d\n" ), hitsVector.size(), timer2.GetResult());
	GetCore()->appLog.Add( message );

	return result;
}

bool Core::ParseLogFile( CString logFileName )
{
	LPBYTE byteBuffer = NULL, wokringBuffer;
	Hit hit;

	// To test performance
	//CString message;
	//HighPrefTimer timer;
	// To test performance

	try
	{
		// File opening
		CFile logFile;
		logFile.Open( logFileName, CFile::modeRead, NULL );
		DWORD64 fileSize = logFile.GetLength();
		if ( fileSize > 0 && fileSize < 1024*1024*1024 ) // 1gb is enough for everyone
		{
			// File loading
			byteBuffer = ( LPBYTE )malloc( DWORD( fileSize ) + 1 ); // bigger buffer to guarantee last char as null terminator
			logFile.Read( byteBuffer, DWORD( fileSize ) );
			byteBuffer[ DWORD( fileSize ) ] = 0x00;

			// sample line
			// 1.2.3.4 - - [11/Feb/2017:00:00:07 -0400] "GET /omm/ HTTP/1.1" 200 7520 "https://www.google.co.nz/" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36" "3.41"
			// we need ip, date&time, request, return code, size, referer

			//timer.Start();

			wokringBuffer = byteBuffer;
			DWORD lines = 0, badLines = 0;
			DWORD processedBytes;
			for ( DWORD cnt = 0; cnt < fileSize; ++cnt )
			{
				wokringBuffer = byteBuffer + cnt;
				processedBytes = hit.ParseLine( wokringBuffer, DWORD( fileSize ) - cnt );

				if ( hit.ipv4 != 0 ) // ip is ok - everything is good
				{
					hitsVector.push_back( hit );
					++lines;
				}
				else
				{
					++badLines;
				}
				cnt += processedBytes;
			}

			//timer.Stop();

			//CString message;
			//message.Format( _T( "  L: %d  B: %d\n" ), lines, badLines );
			//GetCore()->appLog.Add( message );
		}
	}
	catch ( ... )
	{
	}

	if ( byteBuffer != NULL )
	{
		free( byteBuffer );
	}

	return true;
}


#pragma endregion

#pragma region Subnets

bool Core::LoadSubnets()
{
	bool result = false;

	// To test performance
	CString message;
	HighPrefTimer timer;
	// To test performance

	try
	{
		CString subChacheFile = cacheFolder + SUBNETS_CACHE_FILE;
		CFileStatus fileStat;

		std::vector<RawSubnet> rawSubnets;
		//RawSubnet* pRawSubnet = NULL;
		//DWORD numbersOfSubnets = 0;

		if ( CFile::GetStatus( LPCTSTR( subChacheFile ), fileStat, NULL ) )
		{
			// Load precreated database
			LoadSubnetsCache( &rawSubnets );
		}
		else
		{
			// Create database from *.txt files
			BuildSubnets( &rawSubnets );
		}

		timer.Start();

		// Fill fast access array with pointers to objects in vector
		subnetsVector.reserve( rawSubnets.size() );

		std::vector<RawSubnet>::iterator itRawSub;
		LPSUBNET pSubnet;
		Subnet subnet;
		DWORD index = 0;
		DWORD subnetsUsed = 0, subnetsVectors = 0;
		for ( itRawSub = rawSubnets.begin(); itRawSub < rawSubnets.end(); itRawSub++, index++ )
		{
			subnet = *itRawSub;
			subnetsVector.push_back( subnet );
			pSubnet = &*subnetsVector.rbegin();
			for ( DWORD x1 = HIWORD( pSubnet->startAddr ); x1 <= HIWORD( pSubnet->endAddr ); x1++ )
			{
				++subnetsUsed;
				if ( ipMap[ x1 ] == nullptr )
				{
					ipMap[ x1 ] = new SubnetHit();
				}
				else
				{
					++subnetsVectors;
				}
				ipMap[ x1 ]->AddSubnet( pSubnet );
			}
		}

		timer.Stop();
		// To test performance
		message.Format( _T( "Subnets: %d build time %u\n" ), subnetsVector.size(), timer.GetResult() );
		GetCore()->appLog.Add( message );

		message.Format( _T( "SubnetHits calls: %d  SubVectors use: %d\n" ), subnetsUsed, subnetsVectors );
		GetCore()->appLog.Add( message );
		// To test performance

		if ( subnetsVector.size() != rawSubnets.size() )
		{
			// Alarm! Something bad happened
		}
		else
		{
			result = true;
		}
	}
	catch ( ... )
	{

	}

	return result;
}

bool Core::LoadSubnetsCache( std::vector<RawSubnet>* pRawSubnets )
{
	LPBYTE byteBuffer = NULL;
	CString subChacheFile = cacheFolder + SUBNETS_CACHE_FILE;
	// File opening
	CFile subnetFile;
	DWORD version;
	LPRAWSUBNET pRawSubnet;

	try
	{
		subnetFile.Open( subChacheFile, CFile::modeRead, NULL );
		DWORD64 fileSize = subnetFile.GetLength();
		if ( fileSize > 3 && fileSize < 64000000 ) // 64mbs is enough for everyone
		{
			// File loading
			// Read version (strictly 4 bytes)
			subnetFile.Read( &version, sizeof( version ));

			if ( fileSize >= ( 4 + sizeof( RawSubnet ))) // At least one rawSubnet should be there
			{
				byteBuffer = ( LPBYTE )malloc( DWORD( fileSize - 4 ) );
				subnetFile.Read( byteBuffer, DWORD( fileSize - 4 ) );
				pRawSubnet = LPRAWSUBNET( byteBuffer );
				for ( DWORD cnt = 0; cnt < ( ( fileSize - 4 ) / sizeof( RawSubnet ) ); cnt++ )
				{
					pRawSubnets->push_back( *pRawSubnet );
					pRawSubnet++;
				}
			}
		}
		subnetFile.Close();
	}
	catch (...)
	{

	}

	if ( byteBuffer != NULL )
	{
		free( byteBuffer );
	}

	TRACE( "subnets loaded %d\n", pRawSubnets->size() );

	return true;
}

bool Core::BuildSubnets( std::vector<RawSubnet>* pRawSubnets )
{
	CStringArray resultNames;
	GetFilesByMask( &resultNames, subnetsDbFolder, _T( "*.txt" ) );

	// To test performance
	CString message;
	HighPrefTimer timer;
	timer.Start();
	// To test performance

	for ( int index = 0; index < resultNames.GetSize(); index++ )
	{
		ParseSubnetsFile( pRawSubnets, resultNames.GetAt( index ) );
	}

	// To test performance
	timer.Stop();
	message.Format( _T( "parse time %u" ), timer.GetResult() );
	GetCore()->appLog.Add( message );
	timer.Start();
	// To test performance

	RawSubnetsSort sortObj;
	std::sort( pRawSubnets->begin(), pRawSubnets->end(), sortObj );

	// To test performance
	timer.Stop();
	message.Format( _T( "sort time %u" ), timer.GetResult() );
	GetCore()->appLog.Add( message );
	// To test performance

	pRawSubnets->shrink_to_fit();

	// Save DB to cache
	DWORD version = 0;
	CFile subnetFile;
	CString subChacheFile = cacheFolder + SUBNETS_CACHE_FILE;
	subnetFile.Open( subChacheFile, CFile::modeWrite | CFile::modeCreate, NULL );
	// Write version
	subnetFile.Write( &version, sizeof( version ));
	for ( auto i : *pRawSubnets )
	{
		subnetFile.Write( &i, sizeof( i ) );
	}
	subnetFile.Close();

	return true;
}

bool Core::ParseSubnetsFile( std::vector<RawSubnet>* pRawSubnets, CString subnetsFilePath )
{
	LPBYTE byteBuffer = NULL;
	RawSubnet subnet;
	IN_ADDR ipAddr;
	BYTE ipv4Marker[ 4 ] = { 0x69, 0x70, 0x76, 0x34 };

	try
	{
		// File opening
		CFile subnetFile;
		subnetFile.Open( subnetsFilePath, CFile::modeRead, NULL );
		DWORD64 fileSize = subnetFile.GetLength();
		if ( fileSize > 0 && fileSize < 64000000 ) // 64mbs is enough for everyone
		{
			// File loading
			byteBuffer = ( LPBYTE )malloc( DWORD( fileSize ) + 1 ); // bigger buffer to guarantee last char as null terminator
			subnetFile.Read( byteBuffer, DWORD( fileSize ) );
			byteBuffer[ DWORD( fileSize ) ] = 0x00;
			LPBYTE workingByteBuffer = byteBuffer;
			// sample line
			// afrinic|BF|ipv4|41.138.96.0|8192|20090715|allocated
			// we need country, network type (ipv4), starting IP and number of addresses allocated, so values 2,3,4 and 5.
			DWORD substring = 0;
			bool validEntry = false;
			for ( DWORD cnt = 0; cnt < fileSize; ++cnt )
			{
				// traverse between separators
				if ( byteBuffer[ cnt ] == 0x7C ) // | separator
				{
					++substring;
					if ( substring == 2 ) // country
					{
						subnet.countryId[ 0 ] = byteBuffer[ cnt - 2 ];
						subnet.countryId[ 1 ] = byteBuffer[ cnt - 1 ];

						if ( ( cnt + 5 ) < fileSize ) // the end is not near
						{
							// seems faster than comparing with dword
							workingByteBuffer = byteBuffer + cnt + 1;
							if ( memcmp( ipv4Marker, workingByteBuffer, sizeof( ipv4Marker ) ) == 0 ) // is it ipv4 entry i.e. valid?
							{
								validEntry = true;
							}
						}
					}
					else if ( substring == 4 && validEntry )
					{
						byteBuffer[ cnt ] = 0x00;
						// workingByteBuffer to byteBuffer[cnt] possilbe ipv4 address
						if ( inet_pton( AF_INET, ( PCSTR )workingByteBuffer, &ipAddr ) == 1 )
						{
							// ok ipv4 here
							subnet.startAddr = ntohl( ipAddr.S_un.S_addr );
						}
						else
						{
							// no, not valid
							// do not even check for final value
							validEntry = false;
						}
					}
					else if ( substring == 5 && validEntry )
					{
						byteBuffer[ cnt ] = 0x00;
						// workingByteBuffer to byteBuffer[cnt] possilbe number of ip addresses (int)
						int number = atoi( ( PCSTR )workingByteBuffer );
						if ( number < INT_MAX && number > 0 )
						{
							// cool, something valid
							subnet.endAddr = subnet.startAddr + ( number - 1 );
							// Add element to vector
							pRawSubnets->push_back( subnet );
						}
						else
						{
							// Bad value
						}
					}
					workingByteBuffer = byteBuffer + cnt + 1; // next char after |
				}
				else if ( byteBuffer[ cnt ] < 0x20 ) // end of line
				{
					workingByteBuffer = byteBuffer + cnt + 1; // start of the line
					substring = 0;
					validEntry = false;
				}
			}

			free( byteBuffer );
			byteBuffer = NULL;
		}
		else
		{
			appLog.Add( _T( "subnet db file too small or too big" ) );
		}
		subnetFile.Close();
	}
	catch ( ... )
	{
		if ( byteBuffer != NULL )
		{
			free( byteBuffer );
		}
		appLog.Add( _T( "subnet db file exception" ) );
	}

	TRACE( "subnets parsed %d\n", pRawSubnets->size() );

	return true;
}

#pragma endregion

#pragma region System

bool Core::ConvertStringAndSave( CString* pString, CFile* pFile )
{
	LPSTR pNewStr = UNICODEtoASCII( pString, CP_UTF8 );
	if ( pNewStr != nullptr )
	{
		pFile->Write( pNewStr, DWORD( strlen( pNewStr )));
		free( pNewStr );
		return true;
	}
	return false;
}

DWORD Core::GetFilesByMask( CStringArray* resultNames, CString folder, CString mask )
{
	WIN32_FIND_DATA l_stFindFileData;
	HANDLE l_hHandle;
	CString folderToPopulate = folder + _T( "\\" );
	CString folderWithMask = folderToPopulate + mask;

	l_hHandle = FindFirstFile( folderWithMask, &l_stFindFileData );
	while ( l_hHandle != INVALID_HANDLE_VALUE )  // First file find for enumeration
	{
		if ( ( l_stFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			resultNames->Add( folderToPopulate + l_stFindFileData.cFileName );
		}
		if ( !FindNextFile( l_hHandle, &l_stFindFileData ) )
		{
			break;
		}
	}
	FindClose( l_hHandle );

	return DWORD( resultNames->GetSize());
}

CString Core::MakeBytesSizeString( DWORD64 originalValue )
{
	DWORD64 nicerValue = originalValue;
	if (( originalValue & 0xFF00000000000000 ) == 0 )
	{
	//	nicerValue = originalValue * 100;
	}
	wchar_t type[ 4 ] = L" b ";
	if ( nicerValue > 999999999999999 )
	{
		nicerValue /= 1125899906842624;
		wcscpy_s( type, 4, L" Pb" );
	}
	else if ( nicerValue > 999999999999 )
	{
		nicerValue /= 1099511627776;
		wcscpy_s( type, 4, L" Tb" );
	}
	else if ( nicerValue > 999999999 )
	{
		nicerValue /= 1073741824;
		wcscpy_s( type, 4, L" Gb" );
	}
	else if ( nicerValue > 1048576 /*999999*/ )
	{
		nicerValue /= 1048576;
		wcscpy_s( type, 4, L" Mb" );
	}
	else if ( nicerValue > 1024 /*999*/)
	{
		nicerValue /= 1024;
		wcscpy_s( type, 4, L" Kb" );
	}
	wchar_t buffer[ 32 ]{};
	_ui64tow_s( nicerValue, buffer, 32, 10 );
	if ( ( originalValue & 0xFF00000000000000 ) == 0 )
	{
		/*
		buffer[ 31 ] = 0;
		DWORD len = wcslen( buffer );
		buffer[ len ] = buffer[ len - 1 ];
		buffer[ len - 1 ] = buffer[ len - 2 ];
		buffer[ len - 2 ] = wchar_t(".");
		*/
	}
	wcscat_s( buffer, 32, type );
	return CString( buffer );
}

LPSTR Core::UNICODEtoASCII( CString* pUnicodeSrt, UINT codePage )
{
	LPSTR pNewASCII = NULL;
	int newSize = WideCharToMultiByte( codePage, NULL, LPCWSTR( *pUnicodeSrt ), pUnicodeSrt->GetLength(), NULL, NULL, NULL, NULL );
	if ( newSize > 0 )
	{
		pNewASCII = LPSTR( malloc( newSize + 1 ) );
		memset( pNewASCII, 0, newSize + 1 );
		newSize = WideCharToMultiByte( codePage, NULL, LPCWSTR( *pUnicodeSrt ), pUnicodeSrt->GetLength(), pNewASCII, newSize, NULL, NULL );
	}
	return pNewASCII;
}

#pragma endregion

