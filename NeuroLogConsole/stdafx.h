// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

#define ASSERT assert

#pragma warning (disable:4351)

class CListBox
{
public:
	void SetCurSel( int ) {}
	int AddString( std::wstring str )
	{
		std::wcout << str << std::endl;
		return 0;
	}
};

#include "..\NeuroLog\Types.h"
#include "..\NeuroLog\Core.h"

extern inline LPCORE GetCore();
extern void NewCore();
extern void DeleteCore();

#pragma message( "Linking Ws2_32.lib" )
#pragma comment( lib, "Ws2_32.lib" )