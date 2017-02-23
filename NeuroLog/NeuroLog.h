// Copyright (c) CEZEO software Ltd. http://www.cezeo.com
// NeuroLog.h : main header file for the PROJECT_NAME application

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNeuroLogApp:
// See CNeuroLog.cpp for the implementation of this class
//

class CNeuroLogApp : public CWinAppEx
{
public:
	CNeuroLogApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNeuroLogApp theApp;