// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "BitDateTime.h"

///////////////////////////////////////////////////////////////////
//
// By default, request URI and referrer fields are not collected,
// since currently they are not in use and they taking +25% processing time (if enabled).
// However code is ready to work with them, but not with user agent(it's not ready yet).
//
// Comment defines after this block to enable it.
//
///////////////////////////////////////////////////////////////////

#define DO_NOT_COLLECT_REQUEST_URI
#define DO_NOT_COLLECT_REFFERER
#define DO_NOT_COLLECT_USER_AGENT


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
#ifndef DO_NOT_COLLECT_USER_AGENT
	uint32 userAgentIndex;
#endif
#ifndef DO_NOT_COLLECT_REQUEST_URI
	uint32 requestUriIndex;
#endif
#ifndef DO_NOT_COLLECT_REFFERER
	uint32 referrerIndex;
#endif

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

