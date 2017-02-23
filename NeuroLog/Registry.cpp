// Copyright (c) CEZEO software Ltd. http://www.cezeo.com
// Registry.cpp: implementation of the Registry class.

#include "stdafx.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


Registry::Registry()
{
	mp_strRegistryKey = NULL;
	mp_strProfileName = NULL;

	SetRegistryKey( _T("CEZEO software"), _T("NeuroLog"));

	// need to be set to prevent uninitialized usage
	m_boolUseLocalMachineHive = FALSE;
}

Registry::~Registry()
{
	FreeStringMem();
}

void Registry::FreeStringMem()
{
	if ( mp_strRegistryKey != NULL )
	{
		free(( void* )mp_strRegistryKey );
		mp_strRegistryKey = NULL;
	}
	if ( mp_strProfileName != NULL )
	{
		free(( void* )mp_strProfileName );
		mp_strProfileName = NULL;
	}
}

void Registry::SetRegistryKey( LPCTSTR lp_strRegistryKey, LPCTSTR lp_strAppName )
{
	FreeStringMem();
	mp_strRegistryKey = _tcsdup( lp_strRegistryKey );
	mp_strProfileName = _tcsdup( lp_strAppName );
}


// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY Registry::GetAppRegistryKey( enREGISTRY_FLAGS l_enFlags, REGSAM l_rsAccess )
{
	ASSERT( mp_strRegistryKey != NULL );
	ASSERT( mp_strProfileName != NULL );

	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	HKEY hLocalMachine = HKEY_CURRENT_USER;
	if ( l_enFlags == enREGISTRY_USE_DEFAULT )
	{
		if ( IsLocalMachineSettings())
		{
			hLocalMachine = HKEY_LOCAL_MACHINE;
		}
	}
	else if ( l_enFlags == enREGISTRY_USE_LOCAL_MACHINE )
	{
		hLocalMachine = HKEY_LOCAL_MACHINE;
	}
	else if (  l_enFlags == enREGISTRY_USE_CURRENT_USER )
	{
		hLocalMachine = HKEY_CURRENT_USER;
	}

	if ( RegOpenKeyEx( hLocalMachine, _T( "software" ), 0, l_rsAccess, &hSoftKey ) == ERROR_SUCCESS )
	{
		DWORD dw;
		if ( RegCreateKeyEx( hSoftKey, mp_strRegistryKey, 0, REG_NONE,REG_OPTION_NON_VOLATILE, l_rsAccess, NULL,	&hCompanyKey, &dw ) == ERROR_SUCCESS )
		{
			RegCreateKeyEx( hCompanyKey, mp_strProfileName, 0, REG_NONE, REG_OPTION_NON_VOLATILE, l_rsAccess, NULL, &hAppKey, &dw );
		}
	}
	if ( hSoftKey != NULL )
	{
		RegCloseKey( hSoftKey );
	}
	if ( hCompanyKey != NULL )
	{
		RegCloseKey( hCompanyKey );
	}
	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
HKEY Registry::GetSectionKey( LPCTSTR lpszSection,  enREGISTRY_FLAGS l_enFlags, REGSAM l_rsAccess  )
{
	ASSERT( lpszSection != NULL );

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey( l_enFlags, l_rsAccess );
	if ( hAppKey == NULL )
	{
		return NULL;
	}
	DWORD dw;
	RegCreateKeyEx( hAppKey, lpszSection, 0, REG_NONE, REG_OPTION_NON_VOLATILE, l_rsAccess, NULL, &hSectionKey, &dw ); // KEY_WRITE|KEY_READ
	RegCloseKey( hAppKey );
	return hSectionKey;
}

UINT Registry::GetProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	ASSERT( lpszEntry != NULL );

	if ( mp_strRegistryKey != NULL ) // use registry
	{
		HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_READ );
		if ( hSecKey == NULL )
		{
			return nDefault;
		}
		DWORD dwValue;
		DWORD dwType;
		DWORD dwCount = sizeof( DWORD );
		LONG lResult = RegQueryValueEx( hSecKey, ( LPTSTR )lpszEntry, NULL, &dwType, ( LPBYTE )&dwValue, &dwCount );
		RegCloseKey( hSecKey );
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_DWORD );
			ASSERT( dwCount == sizeof( dwValue ));
			return ( UINT )dwValue;
		}
		return nDefault;
	}
	else
	{
		ASSERT( mp_strProfileName != NULL );
		return ::GetPrivateProfileInt( lpszSection, lpszEntry, nDefault, mp_strProfileName );
	}
}

AFX_DATADEF TCHAR EafxChNil = '\0';

CString Registry::GetProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	ASSERT( lpszEntry != NULL );
	if ( mp_strRegistryKey != NULL )
	{
		HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_READ );
		if ( hSecKey == NULL )
		{
			return lpszDefault;
		}
		CString strValue;
		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx( hSecKey, ( LPTSTR )lpszEntry, NULL, &dwType, NULL, &dwCount );
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_SZ );
			lResult = RegQueryValueEx( hSecKey, ( LPTSTR )lpszEntry, NULL, &dwType, ( LPBYTE )strValue.GetBuffer( dwCount/sizeof( TCHAR )), &dwCount );
			strValue.ReleaseBuffer();
		}
		RegCloseKey( hSecKey );
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_SZ );
			return strValue;
		}
		return lpszDefault;
	}
	else
	{
		ASSERT( mp_strProfileName != NULL );

		if ( lpszDefault == NULL )
		{
			lpszDefault = &EafxChNil;    // don't pass in NULL
		}
		TCHAR szT[4096];

		DWORD dw = ::GetPrivateProfileString( lpszSection, lpszEntry, lpszDefault, szT, sizeof(szT)/sizeof(szT[0]), mp_strProfileName );
		ASSERT( dw < 4095 );
		return szT;
	}
}


BOOL Registry::GetProfileBinary( LPCTSTR lp_strSection, LPCTSTR lp_strValue, LPBYTE lp_byteData, UINT l_uintBytes, enREGISTRY_FLAGS l_enFlags )
{
	BOOL l_boolResult = FALSE;
	UINT	l_uintAllocatedSize;
	LPBYTE	lp_byteAllocatedBuffer;
	if ( GetProfileBinaryOrg( lp_strSection , lp_strValue , &lp_byteAllocatedBuffer , &l_uintAllocatedSize, l_enFlags ) == TRUE )
	{
		if ( l_uintAllocatedSize > 0 )
		{
			if ( l_uintAllocatedSize > l_uintBytes )
			{
				l_uintAllocatedSize = l_uintBytes;
			}
			memcpy( LPVOID( lp_byteData ), lp_byteAllocatedBuffer , l_uintAllocatedSize );
			delete [] lp_byteAllocatedBuffer;
			l_boolResult = TRUE;
		}
	}
	return l_boolResult;
}

BOOL Registry::GetProfileBinaryOrg( LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	ASSERT( lpszEntry != NULL );
	ASSERT( ppData != NULL );
	ASSERT( pBytes != NULL );
	*ppData = NULL;
	*pBytes = 0;
	if ( mp_strRegistryKey != NULL )
	{
		HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_READ );
		if ( hSecKey == NULL )
		{
			return FALSE;
		}
		DWORD dwType, dwCount;
		LONG lResult = RegQueryValueEx( hSecKey, ( LPTSTR )lpszEntry, NULL, &dwType,NULL, &dwCount );
		*pBytes = dwCount;
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_BINARY );
			*ppData = new BYTE[*pBytes];
			lResult = RegQueryValueEx( hSecKey, ( LPTSTR )lpszEntry, NULL, &dwType, *ppData, &dwCount );
		}
		RegCloseKey( hSecKey );
		if ( lResult == ERROR_SUCCESS )
		{
			ASSERT( dwType == REG_BINARY );
			return TRUE;
		}
		else
		{
			delete [] *ppData;
			*ppData = NULL;
		}
		return FALSE;
	}
	else
	{
		ASSERT( mp_strProfileName != NULL );

		CString str = GetProfileString( lpszSection, lpszEntry, NULL );
		if ( str.IsEmpty())
		{
			return FALSE;
		}
		ASSERT( str.GetLength()%2 == 0 );
		int nLen = str.GetLength();
		*pBytes = nLen/2;
		*ppData = new BYTE[*pBytes];
		for ( int i=0;i<nLen;i+=2 )
		{
			( *ppData )[i/2] = ( BYTE )((( str[i+1] - 'A' ) << 4 ) + ( str[i] - 'A' ));
		}
		return TRUE;
	}
}

BOOL Registry::WriteProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	ASSERT( lpszEntry != NULL );
	if ( mp_strRegistryKey != NULL )
	{
		HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_WRITE | KEY_READ );
		if ( hSecKey == NULL )
		{
			return FALSE;
		}
		LONG lResult = RegSetValueEx( hSecKey, lpszEntry, NULL, REG_DWORD, ( LPBYTE )&nValue, sizeof( nValue ));
		RegCloseKey( hSecKey );
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT( mp_strProfileName != NULL );

		TCHAR szT[16];
		wsprintf( szT, _T( "%d" ), nValue );
		return ::WritePrivateProfileString( lpszSection, lpszEntry, szT,	mp_strProfileName );
	}
}

BOOL Registry::WriteProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	if ( mp_strRegistryKey != NULL )
	{
		LONG lResult;
		if ( lpszEntry == NULL ) //delete whole section
		{
			HKEY hAppKey = GetAppRegistryKey( l_enFlags ,  KEY_WRITE | KEY_READ );
			if ( hAppKey == NULL )
			{
				return FALSE;
			}
			lResult = ::RegDeleteKey( hAppKey, lpszSection );
			RegCloseKey( hAppKey );
		}
		else if ( lpszValue == NULL )
		{
			HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_WRITE | KEY_READ );
			if ( hSecKey == NULL )
			{
				return FALSE;
			}
			// necessary to cast away const below
			lResult = ::RegDeleteValue( hSecKey, ( LPTSTR )lpszEntry );
			RegCloseKey( hSecKey );
		}
		else
		{
			HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_WRITE | KEY_READ );
			if ( hSecKey == NULL )
			{
				return FALSE;
			}
			lResult = RegSetValueEx( hSecKey, lpszEntry, NULL, REG_SZ, ( LPBYTE )lpszValue, ( lstrlen( lpszValue )+1 )*sizeof( TCHAR ));
			RegCloseKey( hSecKey );
		}
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		ASSERT( mp_strProfileName != NULL );
		ASSERT( lstrlen( mp_strProfileName ) < 4095 ); // can't read in bigger
		return ::WritePrivateProfileString( lpszSection, lpszEntry, lpszValue,mp_strProfileName );
	}
}

BOOL Registry::WriteProfileBinary( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes, enREGISTRY_FLAGS l_enFlags )
{
	ASSERT( lpszSection != NULL );
	BOOL bResult = FALSE;
	if ( mp_strRegistryKey != NULL )
	{
		LONG lResult;
		HKEY hSecKey = GetSectionKey( lpszSection, l_enFlags, KEY_WRITE | KEY_READ );
		if ( hSecKey != NULL )
		{
			lResult = RegSetValueEx( hSecKey, lpszEntry, NULL, REG_BINARY,pData, nBytes );
			RegCloseKey( hSecKey );
			bResult = ( lResult == ERROR_SUCCESS );
		}
	}
	else
	{
		// convert to string and write out
		LPTSTR lpsz = new TCHAR[nBytes*2+1];
		UINT i = 0;
		for ( i = 0; i < nBytes; i++ )
		{
			lpsz[i*2] = ( TCHAR )(( pData[i] & 0x0F ) + 'A' ); //low nibble
			lpsz[i*2+1] = ( TCHAR )((( pData[i] >> 4 ) & 0x0F ) + 'A' ); //high nibble
		}
		lpsz[i*2] = 0;

		ASSERT( mp_strProfileName != NULL );

		bResult = WriteProfileString( lpszSection, lpszEntry, lpsz, l_enFlags );
		delete[] lpsz;
	}
	return bResult;
}
