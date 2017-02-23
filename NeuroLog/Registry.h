// Copyright (c) CEZEO software Ltd. http://www.cezeo.com
// Registry.h: interface for the Registry class.
#pragma once

enum enREGISTRY_FLAGS
{
	enREGISTRY_USE_CURRENT_USER = 0x01,
	enREGISTRY_USE_LOCAL_MACHINE = 0x02,
	enREGISTRY_USE_DEFAULT = 0xFF,
};

class Registry
{
public:

	Registry();
	~Registry();

	void SetRegistryKey( LPCTSTR lp_strRegistryKey, LPCTSTR lp_strAppName );

	HKEY GetSectionKey( LPCTSTR lpszSection, enREGISTRY_FLAGS l_enFlags , REGSAM l_rsAccess );
	HKEY GetAppRegistryKey( enREGISTRY_FLAGS l_enFlags , REGSAM l_rsAccess );

	UINT GetProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry,int nDefault, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT );
	CString GetProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT );
	BOOL GetProfileBinaryOrg( LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT );
	BOOL GetProfileBinary( LPCTSTR lp_strSection, LPCTSTR lp_strValue, LPBYTE lp_byteData, UINT l_uintBytes, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT );

	BOOL WriteProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT  );
	BOOL WriteProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT  );
	BOOL WriteProfileBinary( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes, enREGISTRY_FLAGS l_enFlags = enREGISTRY_USE_DEFAULT );


	void UseLocalMachineSettings( BOOL l_boolUseLocalMachineHive )
	{
		m_boolUseLocalMachineHive = l_boolUseLocalMachineHive;
	}
	BOOL IsLocalMachineSettings()
	{
		return m_boolUseLocalMachineHive;
	}

protected:

	void FreeStringMem();

	BOOL m_boolUseLocalMachineHive;

	LPCTSTR mp_strRegistryKey;
	LPCTSTR mp_strProfileName;

};

typedef Registry* LPREGISTRY;
