// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "Core.h"
#include "Registry.h"

LPCORE pCore;
LPREGISTRY pRegistry;

void NewCore()
{
	// Registry first!
	pRegistry = new Registry;
	pCore = new Core;
}

void DeleteCore()
{
	delete pCore;
	// Registry last!
	delete pRegistry;
}

inline LPCORE GetCore()
{
	return pCore;
}

inline LPREGISTRY GetRegistry()
{
	return pRegistry;
}