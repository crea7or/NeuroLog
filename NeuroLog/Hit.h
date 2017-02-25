// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "BitDateTime.h"

class Hit
{
public:
	Hit();
	~Hit();

	uint32 ipv4;
	uint32 requestSize;
	////
	uint16 statusCode;
	byte requestTypeIndex;
	byte reserved;
	////
	uint32 userAgentIndex;
	uint32 requestUriIndex;
	uint32 referrerIndex;

	BitDateTime dateTime;

	enum RequestType : char
	{
		rGet = 0,
		rPost = 1,
		rHead = 2,
		rOptions = 3,
		rDelete = 4,
		rPut = 5,
		rPatch = 6,
		rConnect = 7,
		rUnknown = -1
	};

	size_t ParseLine( pbyte byteBuffer, size_t remainBytes );
	byte ParseRequest( pbyte buffer );
};
typedef Hit* LPHIT;

