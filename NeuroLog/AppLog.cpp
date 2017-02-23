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

void AppLog::Add(CString message)
{
	appLog.Add( message );
	TRACE( message );
	if (listBoxCtrl != NULL)
	{
		listBoxCtrl->AddString(message);
	}
}

