// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#pragma pack(1)
struct RawSubnet
{
	DWORD startAddr;
	DWORD endAddr;
	BYTE countryId[2];
};
typedef RawSubnet* LPRAWSUBNET;

struct RawSubnetsSort
{
	bool operator() ( RawSubnet& i, RawSubnet& j )
	{
		return ( i.startAddr < j.startAddr );
	}
};
