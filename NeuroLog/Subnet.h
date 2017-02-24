// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include "Hit.h"
#include "RawSubnet.h"
#include <unordered_map>
#include <map>

#pragma region Helper structs/classes

struct SubnetHitStatCnt
{
	SubnetHitStatCnt( uint32 lipv4, uint32 lcount )
	{
		ipv4 = lipv4;
		count = lcount;
	}
	uint32 ipv4;
	uint32 count;
};

struct SubnetHitStatCntSort
{
	bool operator() ( SubnetHitStatCnt& i, SubnetHitStatCnt& j )
	{
		return ( i.count > j.count );
	}
};

struct SubnetHitStatSize
{
	SubnetHitStatSize( uint32 lipv4, uint64 lsize )
	{
		ipv4 = lipv4;
		size = lsize;
	}
	uint32 ipv4;
	uint64 size;
};

struct SubnetHitStatSizeSort
{
	bool operator() ( SubnetHitStatSize& i, SubnetHitStatSize& j )
	{
		return ( i.size > j.size );
	}
};

#pragma endregion

class Subnet
{
public:
	Subnet() {}

	Subnet& operator=( const RawSubnet& rs )
	{
		startAddr = rs.startAddr;
		endAddr = rs.endAddr;
		countryId[0] = rs.countryId[0];
		countryId[1] = rs.countryId[1];
		cidr = rs.cidr;
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

			delete pHitsMap;
			pHitsMap = nullptr;

			delete pHitsSizeMap;
			pHitsSizeMap = nullptr;
		}
	}

	bool HitMatch( LPHIT pHit )
	{
		return ( pHit->ipv4 >= startAddr && pHit->ipv4 <= endAddr );
	}

	void AddHit( LPHIT pHit )
	{
		if ( pHits == nullptr )
		{
			pHits = new std::vector< LPHIT >();
			pHitsMap = new std::unordered_map< uint32, uint32 >();
			pHitsSizeMap = new std::unordered_map< uint32, uint64 >();
		}
		pHits->push_back( pHit );

		//// hits
		std::unordered_map< uint32, uint32>::iterator itHitsMap;
		itHitsMap = pHitsMap->find( pHit->ipv4 );
		if ( itHitsMap != pHitsMap->end() )
		{
			itHitsMap->second = itHitsMap->second + 1;
		}
		else
		{
			std::pair< uint32, uint32> newHit( pHit->ipv4, 1 );
			pHitsMap->insert( newHit );
		}
		//// size
		std::unordered_map< uint32, uint64>::iterator itHitsSizeMap;
		itHitsSizeMap = pHitsSizeMap->find( pHit->ipv4 );
		if ( itHitsSizeMap != pHitsSizeMap->end() )
		{
			itHitsSizeMap->second = itHitsSizeMap->second + pHit->requestSize;
		}
		else
		{
			std::pair< uint32, uint64> newHit( pHit->ipv4, pHit->requestSize );
			pHitsSizeMap->insert( newHit );
		}
		////
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
	byte countryId[2];
	byte cidr = { 0 };

	/*
	std::unordered_map< uint32, uint32 > UniqueHitsMap()
	{
		std::unordered_map< uint32, uint32 > hitMap;
		if ( pHits != nullptr && pHits->size() > 0)
		{
			hitMap.reserve( pHits->size());
			std::unordered_map< uint32, uint32>::iterator itHitsMap;
			std::vector<LPHIT>::iterator itHits;
			for ( itHits = pHits->begin(); itHits < pHits->end(); itHits++ )
			{
				itHitsMap = hitMap.find( (*itHits)->ipv4 );
				if ( itHitsMap != hitMap.end() )
				{
					itHitsMap->second = itHitsMap->second + 1;
				}
				else
				{
					std::pair< uint32, uint32> newHit(( *itHits )->ipv4, 1 );
					hitMap.insert( newHit );
				}
			}
		}
		return hitMap;
	}
	*/


	std::unordered_map< uint32, uint64 >* pHitsSizeMap = { nullptr };
	std::unordered_map< uint32, uint32 >* pHitsMap = { nullptr };
	std::vector< LPHIT >* pHits = { nullptr };
};
typedef Subnet* LPSUBNET;


#pragma region Helper structs/classes

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

#pragma endregion