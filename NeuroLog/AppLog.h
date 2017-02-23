// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

class AppLog
{
public:
	AppLog();
	~AppLog();

	void Add(CString message);


	CListBox* listBoxCtrl;
	CStringArray appLog;
};

