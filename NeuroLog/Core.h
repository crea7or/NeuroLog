// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "Subnet.h"
#include "AppLog.h"
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include "RawSubnet.h"
#include "SubnetHit.h"


class Core
{
public:

	Core();
	~Core();

	bool LoadSubnets();
	bool LoadLogs();
	void AnalyzeSubnets();
	void ClearData();

	CString subnetsDbFolder;
	CString logsFolder;
	CString cacheFolder;
	CString logsMask;

	DWORD sizeLimit; //  Do not include subnets into report when they generated less traffic that this value
	DWORD hitsLimit; //  Do not include subnets into report when they generated less hits that this value

	// Logging object
	AppLog appLog;


	// Array of subnets
	std::vector< Subnet > subnetsVector;

	// Array of Hits files
	std::vector< Hit > hitsVector;

	// Map of Request URIs
	std::unordered_map< std::string, DWORD > uriMap;

	// Map of Referrers
	std::unordered_map< std::string, DWORD > refMap;

	// Map of User Agents
	std::unordered_map< std::string, DWORD > agentsMap;

private:

	#pragma region Subnets

	bool LoadSubnetsCache( std::vector<RawSubnet>* pRawSubnets );
	bool BuildSubnets( std::vector<RawSubnet>* pRawSubnets );
	bool ParseSubnetsFile( std::vector<RawSubnet>* pRawSubnets, CString subnetsFilePath );

	#pragma endregion

	#pragma region System

	DWORD GetFilesByMask( CStringArray* resultNames, CString folder, CString mask );
	CString MakeBytesSizeString( DWORD64 value );
	LPSTR UNICODEtoASCII( CString* pUnicodeSrt, UINT codePage );
	bool ConvertStringAndSave( CString* pString, CFile* pFile );

	#pragma endregion

	#pragma region Logs

	bool ParseLogFile( CString logFile );

	#pragma endregion

	LPSUBNETHIT ipMap[65536]{}; // using two octets of IP address to find subnet block faster

};
typedef Core* LPCORE;

