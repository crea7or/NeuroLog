// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include <vector>
#include "Subnet.h"

class SubnetHit
{
public:

	SubnetHit(){};
	~SubnetHit()
	{
		if ( pSubnets != nullptr )
		{
			delete pSubnets;
		}
	};

	LPSUBNET AddHit( Hit* pHit )
	{
		if ( pSubnets != nullptr )
		{
			for ( LPSUBNET lpSubnet : *pSubnets )
			{
				if ( lpSubnet->HitMatch( pHit ) )
				{
					lpSubnet->AddHit( pHit );
					return lpSubnet;
				}
			}
		}
		else
		{
			if ( pSubnet != nullptr )
			{
				if ( pSubnet->HitMatch( pHit ) )
				{
					pSubnet->AddHit( pHit );
					return pSubnet;
				}
			}
		}
		return nullptr;
	}

	void AddSubnet( LPSUBNET pSubnetToAdd )
	{
		if ( pSubnets != nullptr )
		{
			pSubnets->push_back( pSubnetToAdd );
		}
		else
		{
			if ( pSubnet == nullptr )
			{
				pSubnet = pSubnetToAdd;
			}
			else
			{
				pSubnets = new std::vector<LPSUBNET>();
				pSubnets->push_back( pSubnet );
				pSubnets->push_back( pSubnetToAdd );
				pSubnet = nullptr;
			}
		}
		if ( pSubnets == nullptr )
		{
			pSubnets = new std::vector<LPSUBNET>();
		}
		pSubnets->push_back( pSubnetToAdd );
	}

	LPSUBNET pSubnet{ nullptr };
	std::vector<LPSUBNET>* pSubnets {nullptr};
};

typedef SubnetHit* LPSUBNETHIT;