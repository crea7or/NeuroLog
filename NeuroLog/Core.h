// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <mutex>

#include "Subnet.h"
#include "RawSubnet.h"
#include "SubnetHit.h"
#include "AppLog.h"

#define POOL_THREADS 4
#define DO_REPORT // comment to turn off report generatiron for performance tests
//#define EACH_HIT_LOCK_STRATEGY // faster on small files and slower on big files
#define CRITICAL_SECTION_AS_MUTEX // many times faster than std::mutex

class Core
{
public:

	Core();
	~Core();

	bool LoadSubnets();
	bool LoadLogs();
	void AnalyzeSubnets();
	void ClearData();

	std::wstring subnetsFolder;
	std::wstring outputFolder;
	std::wstring logsFolder;
	std::wstring logsMask;

	uint32 sizeLimit; //  Do not include subnets into report when they generated less traffic that this value
	uint32 hitsLimit; //  Do not include subnets into report when they generated less hits that this value

	// Logging object
	AppLog appLog;

	// Map of Request URIs
	std::unordered_map< std::string, uint32 > uriMap;
	std::mutex uriMapLock;

	// Map of Referrers
	std::unordered_map< std::string, uint32 > refMap;
	std::mutex refMapLock;

	// Map of User Agents
	//std::unordered_map< std::string, uint32 > agentsMap;

	static void Parser();
	std::wstring PopLogFile();
#ifndef	EACH_HIT_LOCK_STRATEGY
	void AppendHits( std::vector< Hit >& hits );
#endif

	size_t totalHitsSession;
	size_t totalLogFiles;
	size_t totalLogFilesSize;

private:

	#pragma region Subnets

	bool LoadSubnetsCache( std::vector<RawSubnet>* pRawSubnets );
	bool BuildSubnets( std::vector<RawSubnet>* pRawSubnets );
	bool ParseSubnetsFile( std::vector<RawSubnet>* pRawSubnets, std::wstring subnetsFilePath );
	bool ParseSubnetsCsvFile( std::vector<RawSubnet>* pRawSubnets, std::wstring subnetsFilePath );

	// Array of subnets
	std::vector< Subnet > subnetsVector;

	#pragma endregion

	#pragma region System

	size_t GetFilesByMask( std::vector< std::wstring >* fileNames, std::wstring folder, std::wstring mask );
	std::string MakeBytesSizeString( uint64 value );
	std::wstring MakeBytesSizeWString( uint64 originalValue );
	std::string Ipv4ToString( uint32 ipv4 );
	byte SetCIDR( uint32 ips );

	#pragma endregion

	#pragma region Logs

	bool ParseLogFile( std::wstring fileName, std::vector< Hit >& hits );
	bool ParseLogFileEmplace( std::wstring fileName, std::vector< Hit >& hits );

	// Array of Hits
	std::vector< Hit > hitsVector;

	// Log files
	std::vector< std::wstring > fileNames;
	std::mutex logFilesLock;

	#pragma endregion

	std::mutex appendHitsLock;

	LPSUBNETHIT ipMap[65536]{}; // using two octets of IP address to find subnet block faster

#ifdef CRITICAL_SECTION_AS_MUTEX
	CRITICAL_SECTION cs;
#endif

};
typedef Core* LPCORE;

