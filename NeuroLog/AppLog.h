// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include <mutex>

class AppLog
{
public:
	AppLog();
	~AppLog();

	void Add( std::wstring message );
	void SetListCtrl( CListBox* listCtrl )
	{
		listBoxCtrl = listCtrl;
	}

private:

	std::mutex accessLock;
	CListBox* listBoxCtrl;
};

