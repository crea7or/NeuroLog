// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "AppLog.h"

AppLog::AppLog()
{
	listBoxCtrl = NULL;
}

AppLog::~AppLog()
{
}

void AppLog::Add( std::wstring message )
{
	TRACE( message.c_str() );
	if (listBoxCtrl != NULL)
	{
		listBoxCtrl->SetCurSel( listBoxCtrl->AddString( message.c_str()));
	}
}

