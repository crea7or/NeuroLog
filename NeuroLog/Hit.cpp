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

size_t Hit::ParseLine( pbyte byteBuffer, size_t remainBytes )
{
	pbyte workingBuffer = byteBuffer;
	pbyte current;

	IN_ADDR ipAddr;
	size_t cnt = 0; // Counter of travesed bytes

	// 1.2.3.4 - - [11/Feb/2017:00:00:07 -0400] "GET /omm/ HTTP/1.1" 200 7520 "https://www.google.co.nz/" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36" "3.41"
	// 0 - ip, 1 - date&time, 2 - reuest+uri, 3 return code, 4 size, 5 referrer, 6 user agent
	uint32 valueType = 0;
	size_t counter;
	byte stopbyte = 0x20; // space after IP as stopbyte
	bool validEntry = true;
	for ( cnt = 0; cnt < remainBytes; cnt++ )
	{
		// traverse between separators
		current = &byteBuffer[ cnt ];
		if ( byteBuffer[ cnt ] == stopbyte )
		{
			switch( valueType )
			{
				case 0: // IP
				{
					byteBuffer[ cnt ] = 0x00;
					// workingByteBuffer to byteBuffer[cnt] possilbe ipv4 address
					if ( inet_pton( AF_INET, (pchar)workingBuffer, &ipAddr ) == 1 )
					{
						// ok ipv4 here
						ipv4 = ntohl( ipAddr.S_un.S_addr );
						//
						stopbyte = 0x5B; // data&time start
						valueType = 1; // data&time [
					}
					else
					{
						// no, not valid
						// do not even check for the final value
						validEntry = false;
						stopbyte = 0xFF; // skip to end
					}
				}
				break;

				case 1: // Date&Time
				{
					if ( stopbyte == 0x5B )
					{
						// before date&time
						workingBuffer = byteBuffer + cnt + 1;
						stopbyte = 0x5D; // after data&time ]
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

							stopbyte = 0x22; // " before request type
							valueType = 2; // request
						}
						else
						{
							validEntry = false;
							stopbyte = 0xFF; // skip to end
						}
					}
				}
				break;

				case 2: // Request type
				{
					if ( stopbyte == 0x22 )
					{
						// before request type
						workingBuffer = byteBuffer + cnt + 1;
						stopbyte = 0x20; // after request type ]
					}
					else
					{
						if ( ( workingBuffer - byteBuffer ) + cnt > 2 )
						{
							requestTypeIndex = ParseRequest( workingBuffer );

							stopbyte = 0x20; // " " before request URI
							workingBuffer = byteBuffer + cnt + 1;
							valueType = 4; // request URI end
//							valueType = 3; // request URI start
						}
						else
						{
							validEntry = false;
							stopbyte = 0xFF; // skip to end
						}
					}
				}
				break;

				case 3: // URI start stopbyte = 0x20;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 4; // request URI end
				}
				break;

				case 4: // URI end
				{
					byteBuffer[ cnt ] = 0x00;
#ifndef DO_NOT_COLLECT_REQUEST_URI
					std::string uri( ( pchar )workingBuffer );
					std::unordered_map< std::string, uint32 >::const_iterator got = GetCore()->uriMap.find( uri );
					if ( got == GetCore()->uriMap.end() )
					{
						// add new URI
						requestUriIndex = uint32( GetCore()->uriMap.size());
						std::pair< std::string, uint32 > np( uri, requestUriIndex );
						GetCore()->uriMap.insert( np );
					}
					else
					{
						requestUriIndex = got->second;
					}
#endif
					valueType = 5; // return code,  stopbyte = 0x20 - before return code
				}
				break;

				case 5: // before return code, stopbyte = 0x20;
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
						stopbyte = 0xFF; // skip to end
					}
					valueType = 6; // request size
				}
				break;

				case 6: // before request size, stopbyte = 0x20;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 7; // request size end
				}
				break;

				case 7: // after request size, stopbyte = 0x20;
				{
					byteBuffer[ cnt ] = 0x00;

					int number = atoi( ( pchar )workingBuffer );
					if ( number < INT_MAX )
					{
						// cool, something valid
						requestSize = ( uint32 )number;
						stopbyte = 0x22; // " before referrer
						valueType = 8; // referrer
					}
					else
					{
						validEntry = false;
						stopbyte = 0xFF; // skip to end
					}
				}
				break;

				case 8: // " before referrer stopbyte = 0x22;
				{
					workingBuffer = byteBuffer + cnt + 1;
					valueType = 9; // referrer end
				}
				break;

				case 9: // referrer end
				{
					byteBuffer[ cnt ] = 0x00;
#ifndef DO_NOT_COLLECT_REFFERER
					std::string ref( ( pchar )workingBuffer );
					std::unordered_map< std::string, uint32 >::const_iterator got = GetCore()->refMap.find( ref );
					if ( got == GetCore()->refMap.end() )
					{
						// add new URI
						referrerIndex = uint32( GetCore()->refMap.size());
						std::pair< std::string, uint32 > np( ref, referrerIndex );
						GetCore()->refMap.insert( np );
					}
					else
					{
						referrerIndex = got->second;
					}
#endif
					// Everything good
					stopbyte = 0xFE; // skip to end of line
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

	if ( stopbyte != 0xFE )
	{
		ipv4 = 0;
	}

	return cnt;
}

byte Hit::ParseRequest( pbyte buffer )
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
