// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

// stdafx.cpp : source file that includes just the standard includes
// NeuroLogConsole.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "..\NeuroLog\Core.h"

LPCORE pCore;
CListBox* pFakeListBox;

void NewCore()
{
	pCore = new Core;
	pFakeListBox = new CListBox();
	pCore->appLog.listBoxCtrl = pFakeListBox;
}

void DeleteCore()
{
	pCore->appLog.listBoxCtrl = nullptr;
	delete pFakeListBox;
	delete pCore;
}

inline LPCORE GetCore()
{
	return pCore;
}
