// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "BitDateTime.h"

#pragma pack (1)

class Hit
{
public:
	Hit();
	~Hit();

	DWORD ipv4;
	DWORD requestSize;

	WORD statusCode;
	BYTE requestTypeIndex;
	BYTE reserved;

	DWORD userAgentIndex;
	DWORD requestUriIndex;
	DWORD referrerIndex;

	BitDateTime dateTime;

	enum RequestType : BYTE
	{
		rGet = 0,
		rPost = 1,
		rHead = 2,
		rOptions = 3,
		rDelete = 4,
		rPut = 5,
		rPatch = 6,
		rConnect = 7,
		rUnknown = 0xFF
	};

	DWORD ParseLine( LPBYTE byteBuffer, DWORD remainBytes );
	BYTE ParseRequest( LPBYTE buffer );
};
typedef Hit* LPHIT;

