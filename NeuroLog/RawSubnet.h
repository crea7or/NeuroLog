// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once



#pragma pack(1)
struct RawSubnet
{
	uint32 startAddr = { 0 };
	uint32 endAddr = { 0 };
	byte countryId[ 2 ];
	byte cidr = { 0 };
	byte reserved = { 0 };
};
typedef RawSubnet* LPRAWSUBNET;

struct RawSubnetsSort
{
	bool operator() ( RawSubnet& i, RawSubnet& j )
	{
		return ( i.startAddr < j.startAddr );
	}
};
