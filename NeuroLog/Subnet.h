// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "Hit.h"
#include "RawSubnet.h"

class Subnet
{
public:
	Subnet()
	{
	}

	Subnet& operator=( const RawSubnet& rs )
	{
		startAddr = rs.startAddr;
		endAddr = rs.endAddr;
		countryId[0] = rs.countryId[0];
		countryId[1] = rs.countryId[1];
		return *this;
	}

	~Subnet()
	{
		Clear();
	}

	void Clear()
	{
		if ( pHits != nullptr )
		{
			delete pHits;
			pHits = nullptr;
		}
	}

	bool HitMatch( Hit* pHit )
	{
		return ( pHit->ipv4 >= startAddr && pHit->ipv4 <= endAddr );
	}

	void AddHit( Hit* pHit )
	{
		if ( pHits == nullptr )
		{
			pHits = new std::vector< LPHIT >();
		}
		pHits->push_back( pHit );
	}

	size_t HitsCount()
	{
		if ( pHits == nullptr )
		{
			return 0;
		}
		return pHits->size();
	}

	uint64 HitsSize()
	{
		if ( pHits == nullptr )
		{
			return 0;
		}
		uint64 size = 0;
		for ( LPHIT pHit : *pHits )
		{
			size += pHit->requestSize;
		}
		return size;
	}

	uint32 startAddr;
	uint32 endAddr;
	BYTE countryId[2];

	std::vector< LPHIT >* pHits = { nullptr };
};
typedef Subnet* LPSUBNET;

struct SubnetsSortHitsCount
{
	bool operator() ( LPSUBNET i, LPSUBNET j )
	{
		return ( i->HitsCount() > j->HitsCount() );
	}
};

struct SubnetsSortHitsSize
{
	bool operator() ( LPSUBNET i, LPSUBNET j )
	{
		return ( i->HitsSize() > j->HitsSize() );
	}
};

