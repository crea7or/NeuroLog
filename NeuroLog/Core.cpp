// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "Core.h"
#include "Ws2tcpip.h"
#include "RawSubnet.h"
#include "ChronoTimer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define SUBNETS_CACHE_FILENAME L"\\NeuroLog.Subnets"
#define SUBNETS_REPORT_FILENAME L"\\NeuroLog.Statistics.html"

#define HTML_START "<html><head><style type=\"text/css\">body,table{font-family:Tahoma,Sans-serif;font-size:10pt;text-align:left}</style><script>function elm(e){$el=e.target.parentNode.querySelector('div.el');if($el.style.display=='none'){$el.style.display='block'}else{$el.style.display='none'};}</script></head><body>"
#define HTML_START_DATA "<table cellspacing=\"3\" cellpadding=\"5\" width=\"50%\" style=\"float: left;\"><tr bgcolor=\"Moccasin\"><td>Subnet IP(first)</td><td>Country</td><td>Hits</td><td>Traffic</td></tr>"
#define HTML_MID "</table><table cellspacing=\"3\" cellpadding=\"5\" width=\"50%\" style=\"float: left;\"><tr bgcolor=\"Moccasin\"><td>Subnet IP(first)</td><td>Country</td><td>Hits</td><td>Traffic</td></tr>"
#define HTML_IP_DIV_START "&nbsp;&nbsp;<a href=\"#\" onclick=\"elm(event); return false;\">#</a><div style=\"display: none\" class=\"el\"><p>"
#define HTML_IP_DIV_END "</p></div>"
#define HTML_END "</table></body></html>"
#define HTML_BG_CELL_ODD "<tr bgcolor=\"#F5F5DC\">"
#define HTML_BG_CELL_EVEN "<tr bgcolor=\"#FFF8DC\">"

//
uint32 addresToCIDR[ 32 ] = { 2147483648, 1073741824, 536870912, 268435456, 134217728,
67108864, 33554432, 16777216, 8388608, 4194304, 2097152, 1048576, 524288, 262144, 131072,
65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128,
64, 32, 16, 8, 4, 2, 1 };

Core::Core()
{
}

Core::~Core()
{
	for ( size_t index = 0; index < 65536; index++ )
	{
		if ( ipMap[ index ] != nullptr )
		{
			delete ipMap[ index ];
		}
	}
}



void Core::ClearData()
{
	std::vector<Subnet>::iterator itSub;
	for ( itSub = subnetsVector.begin(); itSub < subnetsVector.end(); itSub++ )
	{
		itSub->Clear();
	}

	hitsVector.clear();
	uriMap.clear();
	refMap.clear();

	//agentsMap.clear();
}

void Core::AnalyzeSubnets()
{
	// Performance test
	ChronoTimer timer;
	timer.Start();
	// Performance test

	std::vector< LPSUBNET > subnetsPtrs;
	subnetsPtrs.reserve( subnetsVector.size());

	LPSUBNET pSubnet;
	std::vector<Subnet>::iterator itSub;
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

	uint32 bgColor = 0;
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
	// Get time interval of analyzed log files

	std::wstring reportFileName = outputFolder + SUBNETS_REPORT_FILENAME;
	std::ofstream outputStream;
	outputStream.open( reportFileName, std::ios::binary );
	if ( outputStream.is_open() )
	{
		outputStream << HTML_START;

		// Write time interval
		outputStream << "<p>Interval analyzed: " << startDt.GetDateTime() << "  to  " << endDt.GetDateTime() << "</p>";
		outputStream << "<p>Log files folder: " << std::string( logsFolder.begin(), logsFolder.end() ) << "\\" << std::string( logsMask.begin(), logsMask.end() ) << "</p>";
		// Write time interval

		// First table header
		outputStream << HTML_START_DATA;
		// First table header


		std::vector< SubnetHitStatCnt > hitCountVector;
		std::vector< SubnetHitStatSize > hitSizeVector;

		std::vector< SubnetHitStatSize >::iterator itSubHitSize;
		std::vector< SubnetHitStatCnt >::iterator itSubHitCnt;;

		std::unordered_map< uint32, uint32 >::iterator itMpCnt;
		std::unordered_map< uint32, uint64 >::iterator itMpSz;

		// First table data
		for ( auto i : subnetsPtrs )
		{
			if ( i->HitsCount() > hitsLimit )
			{
				outputStream << (( bgColor & 1 ) ? HTML_BG_CELL_ODD : HTML_BG_CELL_EVEN );
				outputStream << "<td><a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:";
				outputStream << Ipv4ToString( i->startAddr );
				outputStream << "\" target=\"_blank\">";
				outputStream << Ipv4ToString( i->startAddr ) << "/" << std::to_string( i->cidr ) << "</a>";

				if ( i->pHits != nullptr && i->pHits->size() > 0 )
				{
					outputStream << HTML_IP_DIV_START;

					for ( itMpCnt = i->pHitsMap->begin(); itMpCnt != i->pHitsMap->end(); ++itMpCnt )
					{
						hitCountVector.push_back( SubnetHitStatCnt( itMpCnt->first, itMpCnt->second ) );
					}

					std::sort( hitCountVector.begin(), hitCountVector.end(), SubnetHitStatCntSort());
					for ( itSubHitCnt = hitCountVector.begin(); itSubHitCnt != hitCountVector.end(); ++itSubHitCnt )
					{
						outputStream << "<a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:";
						outputStream << Ipv4ToString( itSubHitCnt->ipv4 );
						outputStream << "\" target=\"_blank\">";
						outputStream << Ipv4ToString( itSubHitCnt->ipv4 ) << "</a> (" << itSubHitCnt->count << ")<br/>";
					}
					hitCountVector.clear();

					outputStream << HTML_IP_DIV_END;
				}

				outputStream << "</td><td>" << i->countryId[ 0 ] << i->countryId[ 1 ] << "</td><td>" << i->HitsCount() << "</td><td>" << MakeBytesSizeString( i->HitsSize());
				++bgColor;
			}
		}
		// First table data

		// Resort data
		SubnetsSortHitsSize sortObjSize;
		std::sort( subnetsPtrs.begin(), subnetsPtrs.end(), sortObjSize );
		// Resort data

		bgColor = 0;

		// Second table header
		outputStream << HTML_MID;
		// Second table header

		// Second table data
		for ( auto i : subnetsPtrs )
		{
			if ( i->HitsCount() > hitsLimit )
			{
				outputStream << ( ( bgColor & 1 ) ? HTML_BG_CELL_ODD : HTML_BG_CELL_EVEN );
				outputStream << "<td><a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:";
				outputStream << Ipv4ToString( i->startAddr );
				outputStream << "\" target=\"_blank\">";
				outputStream << Ipv4ToString( i->startAddr ) << "/" << std::to_string( i->cidr ) << "</a>";

				if ( i->pHits != nullptr && i->pHits->size() > 0 )
				{
					outputStream << HTML_IP_DIV_START;

					for ( itMpSz = i->pHitsSizeMap->begin(); itMpSz != i->pHitsSizeMap->end(); ++itMpSz )
					{
						hitSizeVector.push_back( SubnetHitStatSize( itMpSz->first, itMpSz->second ) );
					}

					std::sort( hitSizeVector.begin(), hitSizeVector.end(), SubnetHitStatSizeSort() );
					for ( itSubHitSize = hitSizeVector.begin(); itSubHitSize != hitSizeVector.end(); ++itSubHitSize )
					{
						outputStream << "<a href=\"https://mxtoolbox.com/SuperTool.aspx?action=arin:";
						outputStream << Ipv4ToString( itSubHitSize->ipv4 );
						outputStream << "\" target=\"_blank\">";
						outputStream << Ipv4ToString( itSubHitSize->ipv4 ) << "</a> ( " << MakeBytesSizeString( itSubHitSize->size) << " )<br/>";
					}
					hitSizeVector.clear();

					outputStream << HTML_IP_DIV_END;
				}
				outputStream << "</td><td>" << i->countryId[ 0 ] << i->countryId[ 1 ] << "</td><td>" << i->HitsCount() << "</td><td>" << MakeBytesSizeString( i->HitsSize() );
				++bgColor;
			}
		}
		// Second table data

		// Html close data
		outputStream << HTML_END;
		// Html close data

		outputStream.close();

		ShellExecute( nullptr, L"open", reportFileName.c_str(), nullptr, nullptr, SW_NORMAL );
	}

	// Performance test
	timer.Stop();
	appLog.Add( L"Report generation time: " + timer.GetElapsedMilliseconds() );
	// Performance test
}

#pragma region Logs

bool Core::LoadLogs()
{
	bool result = false;

	totalHitsSession = 0;
	std::vector< std::wstring > fileNames;
	GetFilesByMask( &fileNames, logsFolder, logsMask );

	try
	{
		// Performance test
		ChronoTimer timer;
		timer.Start();
		appLog.Add( L"Log files found: " + std::to_wstring( fileNames.size()));
		// Performance test

		for ( size_t index = 0; index < fileNames.size(); index++ )
		{
			ParseLogFile( fileNames[ index ] );
		}

		// Performance test
		timer.Stop();
		appLog.Add( L"Logs parsing time: " + timer.GetElapsedMilliseconds() );
		appLog.Add( L"Total hits: " + std::to_wstring( hitsVector.size()));
		// Performance test

		// Performance test
		timer.Start();
		// Performance test

		LPSUBNET pSubnet;
		LPHIT pHit;
		uint32 goodHits = 0, badHits = 0;

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

		// Performance test
		timer.Stop();
		appLog.Add( L"Hits -> subnets parsing time: " + timer.GetElapsedMilliseconds());
		// Performance test
	}
	catch ( ... )
	{

	}

	totalHitsSession = hitsVector.size();

	return result;
}

bool Core::ParseLogFile( std::wstring logFileName )
{
	bool result = true;

	pbyte byteBuffer = nullptr;
	pbyte wokringBuffer;
	Hit hit;

	std::size_t found = logFileName.find_last_of( L"/\\" );
	std::wstring message = L"Log file: " + logFileName.substr( found + 1 );

	uint32 lines = 0, badLines = 0;

	try
	{
		size_t fileSize;
		std::ifstream inputStream;
		inputStream.open( logFileName, std::ios::binary | std::ios::ate );
		if ( inputStream.is_open() )
		{
			fileSize = (size_t)inputStream.tellg();
			inputStream.seekg( 0 );
			if ( fileSize > 0 && fileSize < 1024 * 1024 * 1024 ) // 1gb is enough for everyone (c)
			{
				// File loading
				byteBuffer = ( pbyte )malloc( fileSize );
				inputStream.read((pchar)byteBuffer, fileSize );

				// sample line
				// 1.2.3.4 - - [11/Feb/2017:00:00:07 -0400] "GET /omm/ HTTP/1.1" 200 7520 "https://www.google.co.nz/" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36" "3.41"

				wokringBuffer = byteBuffer;

				size_t processedBytes;
				for ( size_t cnt = 0; cnt < fileSize; ++cnt )
				{
					wokringBuffer = byteBuffer + cnt;
					processedBytes = hit.ParseLine( wokringBuffer, fileSize - cnt );

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

			}
		}
	}
	catch ( ... )
	{
		result = false;
	}

	if ( byteBuffer != nullptr )
	{
		free( byteBuffer );
	}

	message += L"  [ hits: " + std::to_wstring( lines ) + L" ] [ skipped: " + std::to_wstring( badLines ) + L" ]";

	appLog.Add( message );

	return result;
}

#pragma endregion

#pragma region Subnets

bool Core::LoadSubnets()
{
	bool result = false;

	if ( subnetsVector.size() > 0 )
	{
		// Already loaded
		appLog.Add( L"Subnets already loaded" );
		return true;
	}

	try
	{
		// Performance test
		ChronoTimer timer;
		timer.Start();
		// Performance test

		std::wstring subnetsCacheFileName = subnetsFolder + SUBNETS_CACHE_FILENAME;
		std::vector<RawSubnet> rawSubnets;
		if ( LoadSubnetsCache( &rawSubnets ) == false )
		{
			// Create database from *.txt files
			appLog.Add( L"Building subnets database from scratch" );
			BuildSubnets( &rawSubnets );
		}

		// Performance test
		timer.Stop();
		appLog.Add( L"Load/Build time total: " + timer.GetElapsedMilliseconds());
		appLog.Add( L"Subnets found: " + std::to_wstring( rawSubnets.size()));
		// Performance test

		// Performance test
		timer.Start();
		// Performance test

		// Fill fast access array with pointers to objects in vector
		subnetsVector.reserve( rawSubnets.size() );

		std::vector<RawSubnet>::iterator itRawSub;
		LPSUBNET pSubnet;
		Subnet subnet;
		uint32 index = 0;
		uint32 subnetsUsed = 0, subnetsVectors = 0;
		for ( itRawSub = rawSubnets.begin(); itRawSub < rawSubnets.end(); itRawSub++, index++ )
		{
			subnet = *itRawSub;
			subnetsVector.push_back( subnet );
			pSubnet = &*subnetsVector.rbegin();
			for ( uint32 x1 = HIWORD( pSubnet->startAddr ); x1 <= HIWORD( pSubnet->endAddr ); x1++ )
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

		// Performance test
		timer.Stop();
		appLog.Add( L"Subnets in memory cache build time: " + timer.GetElapsedMilliseconds() );
		// Performance test

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
	bool result = false;
	std::wstring cacheFile = outputFolder + SUBNETS_CACHE_FILENAME;
	pbyte byteBuffer = nullptr;
	uint32 version;
	LPRAWSUBNET pRawSubnet;
	size_t fileSize;
	std::ifstream inputStream;

	inputStream.open( cacheFile, std::ios::binary | std::ios::ate );
	if ( inputStream.is_open())
	{
		appLog.Add( L"Prebuild subnets database is used" );

		fileSize = ( size_t )inputStream.tellg();
		inputStream.seekg( 0 );
		if ( fileSize >= ( 4 + sizeof( RawSubnet )))
		{
			// Read version (strictly 4 bytes)
			inputStream.read((pchar)( &version ), sizeof( version ) );

			byteBuffer = (pbyte)malloc( fileSize - 4 );
			inputStream.read((pchar)( byteBuffer ), fileSize - 4 );
			pRawSubnet = LPRAWSUBNET( byteBuffer );
			for ( uint32 cnt = 0; cnt < (( fileSize - 4 ) / sizeof( RawSubnet )); cnt++ )
			{
				pRawSubnets->push_back( *pRawSubnet );
				pRawSubnet++;
			}
		}
		inputStream.close();
		result = true;
	}
	else
	{
		// No cache
	}

	if ( byteBuffer != nullptr )
	{
		free( byteBuffer );
	}
	return result;
}

bool Core::BuildSubnets( std::vector<RawSubnet>* pRawSubnets )
{
	std::vector< std::wstring > fileNames;
	GetFilesByMask( &fileNames, subnetsFolder, L"*.txt" );

	appLog.Add( L"Subnets *.txt db files found: " + std::to_wstring( fileNames.size()));

	for ( size_t index = 0; index < fileNames.size(); index++ )
	{
		ParseSubnetsFile( pRawSubnets, fileNames[ index ] );
	}

	if ( fileNames.size() == 0 )
	{
		GetFilesByMask( &fileNames, subnetsFolder, L"*.csv" );

		appLog.Add( L"Subnets *.csv db files found: " + std::to_wstring( fileNames.size() ) );

		for ( size_t index = 0; index < fileNames.size(); index++ )
		{
			ParseSubnetsCsvFile( pRawSubnets, fileNames[ index ] );
		}
	}

	if ( fileNames.size() == 0 )
	{
		appLog.Add( L"Subnets files are not found, please, download");
	}

	RawSubnetsSort sortObj;
	std::sort( pRawSubnets->begin(), pRawSubnets->end(), sortObj );

	pRawSubnets->shrink_to_fit();

	// Save DB to cache
	if ( pRawSubnets->size() > 0 )
	{
		uint32 version = 0;
		std::wstring cacheFile = outputFolder + SUBNETS_CACHE_FILENAME;
		std::ofstream outputStream;
		outputStream.open( cacheFile, std::ios::binary );
		if ( outputStream.is_open() )
		{
			outputStream.write( ( pchar )&version, sizeof( version ) );
			for ( auto subnet : *pRawSubnets )
			{
				outputStream.write( ( pchar )&subnet, sizeof( subnet ) );
			}
			outputStream.close();
		}
	}

	return true;
}

bool Core::ParseSubnetsFile( std::vector<RawSubnet>* pRawSubnets, std::wstring subnetsFilePath )
{
	RawSubnet subnet;
	IN_ADDR ipAddr;
	byte ipv4Marker[ 4 ] = { 0x69, 0x70, 0x76, 0x34 };
	size_t fileSize;
	pbyte byteBuffer = nullptr;

	try
	{
		std::ifstream inputStream;
		inputStream.open( subnetsFilePath, std::ios::binary | std::ios::ate );
		if ( inputStream.is_open() )
		{
			fileSize = ( size_t )inputStream.tellg();
			inputStream.seekg( 0 );

			if ( fileSize > 0 && fileSize < 64000000 ) // 64mbs is enough for everyone
			{
				// File loading
				byteBuffer = ( pbyte )malloc( fileSize );
				inputStream.read( ( pchar )byteBuffer, fileSize );
				pbyte workingByteBuffer = byteBuffer;

				// Performance test
				ChronoTimer timer;
				timer.Start();
				size_t lines = 0;
				size_t subnets = 0;
				// Performance test

				// sample line
				// afrinic|BF|ipv4|41.138.96.0|8192|20090715|allocated
				// we need country, network type (ipv4), starting IP and number of addresses allocated, so values 2,3,4 and 5.
				size_t substring = 0;
				bool validEntry = false;
				for ( size_t cnt = 0; cnt < fileSize; ++cnt )
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
								// seems faster than comparing with uint32
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
							if ( inet_pton( AF_INET, ( pchar )workingByteBuffer, &ipAddr ) == 1 )
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
							// workingByteBuffer to byteBuffer[cnt] possilbe number of ip addresses (uint32)
							// stoul 10% slower
							//uint32 addrs = std::stoul(( pchar )workingByteBuffer );
							int number = atoi( ( pchar )workingByteBuffer );
							if ( number < INT_MAX && number > 0 )
							{
								// cool, something valid
								subnet.endAddr = subnet.startAddr + ( number - 1 );
								subnet.cidr = SetCIDR( uint32( number ));
								// Add element to vector
								pRawSubnets->push_back( subnet );
								subnets++;
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
						lines++;
					}
				}

				// Performance test
				timer.Stop();
				appLog.Add( L"Subnets build time: " + std::to_wstring( subnets ) + L" per " + timer.GetElapsedMilliseconds());
				// Performance test
			}
			else
			{
				// subnet db file too small or too big
			}
			inputStream.close();
		}
	}
	catch( ... )
	{
		appLog.Add( L"Oh, some exception..." );
	}

	if ( byteBuffer != nullptr )
	{
		free( byteBuffer );
	}

	return true;
}

bool Core::ParseSubnetsCsvFile( std::vector<RawSubnet>* pRawSubnets, std::wstring subnetsFilePath )
{
	RawSubnet subnet;
	IN_ADDR ipAddr;
	size_t fileSize;
	pbyte byteBuffer = nullptr;

	try
	{
		std::ifstream inputStream;
		inputStream.open( subnetsFilePath, std::ios::binary | std::ios::ate );
		if ( inputStream.is_open() )
		{
			fileSize = ( size_t )inputStream.tellg();
			inputStream.seekg( 0 );

			if ( fileSize > 0 && fileSize < 128000000 ) // 128mbs is enough for everyone
			{
				// File loading
				byteBuffer = ( pbyte )malloc( fileSize );
				inputStream.read( ( pchar )byteBuffer, fileSize );
				pbyte workingByteBuffer = byteBuffer;

				// Performance test
				ChronoTimer timer;
				timer.Start();
				size_t lines = 0;
				size_t subnets = 0;
				// Performance test

				// sample line
				// "104.167.227.232","104.167.231.255","US"
				// we need all of them
				byte stopByte = 0x22; // used to skip invalid lines
				size_t substring = 0;
				for ( size_t cnt = 0; cnt < fileSize; ++cnt )
				{
					// traverse between separators
					if ( byteBuffer[ cnt ] == stopByte ) // separator
					{
						++substring;
						if ( substring == 2 ) // start address
						{
							byteBuffer[ cnt ] = 0x00;
							if (( fileSize - cnt ) > 5 && ( workingByteBuffer[ 5 ] == 0x3a )) // : in ipv6 line
							{
								stopByte = 0xff;
							}
							else
							{
								// workingByteBuffer to byteBuffer[cnt] possilbe ipv4 address
								if ( inet_pton( AF_INET, ( pchar )workingByteBuffer, &ipAddr ) == 1 )
								{
									// ok ipv4 here
									subnet.startAddr = ntohl( ipAddr.S_un.S_addr );
								}
								else
								{
									// no, not valid
									// do not even check for final value
									stopByte = 0xff;
								}
							}
						}
						else if ( substring == 4 ) // end address
						{
							byteBuffer[ cnt ] = 0x00;
							// workingByteBuffer to byteBuffer[cnt] possilbe ipv4 address
							if ( inet_pton( AF_INET, ( pchar )workingByteBuffer, &ipAddr ) == 1 )
							{
								// ok ipv4 here
								subnet.endAddr = ntohl( ipAddr.S_un.S_addr );
								uint32 number = ( subnet.endAddr - subnet.startAddr ) + 1; //calculating number of addresses in range
								subnet.cidr = SetCIDR( uint32( number ) );
							}
							else
							{
								// no, not valid
								// do not even check for final value
								stopByte = 0xff;
							}
						}
						else if ( substring == 6 ) // country
						{
							subnet.countryId[ 0 ] = workingByteBuffer[ 0 ];
							subnet.countryId[ 1 ] = workingByteBuffer[ 1 ];
							// Add element to vector
							pRawSubnets->push_back( subnet );
							subnets++;
						}
						workingByteBuffer = byteBuffer + cnt + 1; // next char after stopByte
					}
					else if ( byteBuffer[ cnt ] < 0x20 ) // end of line
					{
						workingByteBuffer = byteBuffer + cnt + 1; // start of the line
						substring = 0;
						stopByte = 0x22;
						lines++;
					}
				}

				// Performance test
				timer.Stop();
				appLog.Add( L"Subnets build time: " + std::to_wstring( subnets ) + L" per " + timer.GetElapsedMilliseconds() );
				// Performance test
			}
			else
			{
				// subnet db file too small or too big
			}
			inputStream.close();
		}
	}
	catch ( ... )
	{
		appLog.Add( L"Oh, some exception..." );
	}

	if ( byteBuffer != nullptr )
	{
		free( byteBuffer );
	}

	return true;
}

#pragma endregion

#pragma region System

size_t Core::GetFilesByMask( std::vector< std::wstring >* fileNames, std::wstring folder, std::wstring mask )
{
	WIN32_FIND_DATA stFindFileData;
	HANDLE hHandle;
	std::wstring folderToPopulate = folder + L"\\";
	std::wstring folderWithMask = folderToPopulate + mask;
	hHandle = FindFirstFile( folderWithMask.c_str(), &stFindFileData );
	while ( hHandle != INVALID_HANDLE_VALUE )
	{
		if ( ( stFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			fileNames->push_back( folderToPopulate + stFindFileData.cFileName );
		}
		if ( !FindNextFile( hHandle, &stFindFileData ) )
		{
			break;
		}
	}
	FindClose( hHandle );
	return fileNames->size();
}

std::string Core::MakeBytesSizeString( uint64 originalValue )
{
	double dp = ( double)originalValue;
	std::string type;
	if ( dp > 999999999999999 )
	{
		dp /= 1125899906842624;
		type = " Pb";
	}
	else if ( dp > 999999999999 )
	{
		dp /= 1099511627776;
		type = " Tb";
	}
	else if ( dp > 999999999 )
	{
		dp /= 1073741824;
		type = " Gb";
	}
	else if ( dp > 1048576 /*999999*/ )
	{
		dp /= 1048576;
		type = " Mb";
	}
	else if ( dp > 1024 /*999*/ )
	{
		dp /= 1024;
		type = " Kb";
	}
	else
	{
		type = " b";
	}
	char buffer[ 32 ];
	// _CRT_SECURE_NO_WARNINGS required with std::sprintf
	sprintf_s( buffer, 32, "%.2f%s", dp, type.c_str());
	return std::string( buffer );
}

std::string Core::Ipv4ToString( uint32 ipv4 )
{
	std::ostringstream stringStream;
	stringStream << (int)(HIBYTE(HIWORD(ipv4))) << "." << (int)(LOBYTE(HIWORD(ipv4))) << "." << (int)(HIBYTE(LOWORD(ipv4))) << "." << (int)(LOBYTE(LOWORD(ipv4)));
	return std::string( stringStream.str() );
}

// Numbers of addresses to CIDR
byte Core::SetCIDR( uint32 ips )
{
	for ( byte index = 0; index < 32; ++index )
	{
		if ( addresToCIDR[ index ] == ips )
		{
			return index + 1;
		}
	}
	return 0;
}

#pragma endregion

