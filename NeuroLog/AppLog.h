// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

class AppLog
{
public:
	AppLog();
	~AppLog();

	void Add( std::wstring message );

	CListBox* listBoxCtrl;
};

