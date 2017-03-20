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
	if (listBoxCtrl != NULL)
	{
		accessLock.lock();
		listBoxCtrl->SetCurSel( listBoxCtrl->AddString( message.c_str()));
		accessLock.unlock();
	}
}

