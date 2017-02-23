// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#include "stdafx.h"
#include "Hit.h"
#include "Ws2tcpip.h"

Hit::Hit()
{
}


Hit::~Hit()
{
}

DWORD Hit::ParseLine( LPBYTE byteBuffer, DWORD remainBytes )
{
	LPBYTE workingBuffer = byteBuffer, current;
	IN_ADDR ipAddr;
	DWORD cnt = 0; // Counter of travesed bytes

	// 1.2.3.4 - - [11/Feb/2017:00:00:07 -0400] "GET /omm/ HTTP/1.1" 200 7520 "https://www.google.co.nz/" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36" "3.41"
	// 0 - ip, 1 - date&time, 2 - reuest+uri, 3 return code, 4 size, 5 referrer, 6 user agent
	DWORD valueType = 0;
	INT_PTR counter;
	BYTE stopChar = 0x20; // space after IP as stopchar
	BOOL validEntry = true;
	for ( cnt = 0; cnt < remainBytes; cnt++ )
	{
		// traverse between separators
		current = &byteBuffer[ cnt ];
		if ( byteBuffer[ cnt ] == stopChar )
		{
			switch( valueType )
			{
				case 0: // IP
				{
					byteBuffer[ cnt ] = 0x00;
					// workingByteBuffer to byteBuffer[cnt] possilbe ipv4 address
					if ( inet_pton( AF_INET, ( PCSTR )workingBuffer, &ipAddr ) == 1 )
					{
						// ok ipv4 here
						ipv4 = ntohl( ipAddr.S_un.S_addr );
						//
						stopChar = 0x5B; // data&time start
						valueType = 1; // data&time [
					}
					else
					{
						// no, not valid
						// do not even check for the final value
						validEntry = false;
						stopChar = 0xFF; // skip to end
					}
				}
				break;

				case 1: // Date&Time
				{
					if ( stopChar == 0x5B )
					{
						// before date&time
						workingBuffer = byteBuffer + cnt + 1;
						stopChar = 0x5D; // after data&time ]
					}
					else
					{
						counter = ( byteBuffer + cnt ) - workingBuffer;
						if ( counter > 22 && counter < 40 )
						{
							// [11/Feb/2017:00:00:07 -0400]
							dateTime.SetFrom2C( workingBuffer, BitDateTime::mDay );
							dateTime.SetMonthFrom3C( workingBuffer + 3 );
							dateTime.SetYearFrom4C( workingBuffer + 7 );
							dateTime.SetFrom2C( workingBuffer + 12, BitDateTime::mHours );
							dateTime.SetFrom2C( workingBuffer + 15, BitDateTime::mMinutes );
							dateTime.SetFrom2C( workingBuffer + 18, BitDateTime::mSeconds );


							//GetCore()->appLog.Add( dateTime.GetDateTime() );

							stopChar = 0x22; // " before request type
							valueType = 2; // request
						}
						else
						{
							validEntry = false;
							stopChar = 0xFF; // skip to end
						}
					}
				}
				break;

				case 2: // Request type
				{
					if ( stopChar == 0x22 )
					{
						// before request type
						workingBuffer = byteBuffer + cnt + 1;
						stopChar = 0x20; // after request type ]
					}
					else
					{
						if ( ( workingBuffer - byteBuffer ) + cnt > 2 )
						{
							requestTypeIndex = ParseRequest( workingBuffer );

							stopChar = 0x20; // " " before request URI
							workingBuffer = byteBuffer + cnt + 1;
							valueType = 4; // request URI end
//							valueType = 3; // request URI start
						}
						else
						{
							validEntry = false;
							stopChar = 0xFF; // skip to end
						}
					}
				}
				break;

				case 3: // URI start stopChar = 0x20;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 4; // request URI end
				}
				break;

				case 4: // URI end
				{
					byteBuffer[ cnt ] = 0x00;
					std::string uri( ( const char* )( workingBuffer ) );
					std::unordered_map< std::string, DWORD >::const_iterator got = GetCore()->uriMap.find( uri );
					if ( got == GetCore()->uriMap.end() )
					{
						// add new URI
						requestUriIndex = DWORD( GetCore()->uriMap.size());
						std::pair< std::string, DWORD > np( uri, requestUriIndex );
						GetCore()->uriMap.insert( np );
					}
					else
					{
						requestUriIndex = got->second;
					}
					valueType = 5; // return code,  stopChar = 0x20 - before return code
				}
				break;

				case 5: // before return code, stopChar = 0x20;
				{
					// 3 chars codes only
					workingBuffer = byteBuffer + cnt + 1;
					if ( ( remainBytes - cnt ) > 4 )
					{
						statusCode = ( ( workingBuffer[ 0 ] - 48 ) * 100 ) + ( ( workingBuffer[ 1 ] - 48 ) * 10 ) + ( workingBuffer[ 2 ] - 48 );
					}
					else
					{
						validEntry = false;
						stopChar = 0xFF; // skip to end
					}
					valueType = 6; // request size
				}
				break;

				case 6: // before request size, stopChar = 0x20;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 7; // request size end
				}
				break;

				case 7: // after request size, stopChar = 0x20;
				{
					byteBuffer[ cnt ] = 0x00;

					int number = atoi( ( PCSTR )workingBuffer );
					if ( number < INT_MAX )
					{
						// cool, something valid
						requestSize = ( DWORD )number;
						stopChar = 0x22; // " before referrer
						valueType = 8; // referrer
					}
					else
					{
						validEntry = false;
						stopChar = 0xFF; // skip to end
					}
				}
				break;

				case 8: // " before referrer stopChar = 0x22;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 9; // referrer end
				}
				break;

				case 9: // referrer end
				{
					byteBuffer[ cnt ] = 0x00;
					std::string ref( ( const char* )( workingBuffer ) );
					std::unordered_map< std::string, DWORD >::const_iterator got = GetCore()->refMap.find( ref );
					if ( got == GetCore()->refMap.end() )
					{
						// add new URI
						referrerIndex = DWORD( GetCore()->refMap.size());
						std::pair< std::string, DWORD > np( ref, referrerIndex );
						GetCore()->refMap.insert( np );
					}
					else
					{
						referrerIndex = got->second;
					}

					// Everything good
					stopChar = 0xFE; // skip to end of line
				}
				break;
			};
		}
		else if ( byteBuffer[ cnt ] < 0x20 ) // end of line
		{
			//cnt++;
			break;
		}
	}

	if ( stopChar != 0xFE )
	{
		ipv4 = 0;
	}

	return cnt;
}

BYTE Hit::ParseRequest( LPBYTE buffer )
{
	switch ( MAKEWORD( buffer[ 1 ], buffer[ 0 ] ) )
	{
		case 0x4745: //GE(T)
			return RequestType::rGet;
		case 0x504F: //PO(ST)
			return RequestType::rPost;
		case 0x4845: //HE(AD)
			return RequestType::rHead;
		default:
			return RequestType::rUnknown;
	};
}
