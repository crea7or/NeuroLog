// Copyright (c) CEZEO software Ltd. http://www.cezeo.com

#pragma once

#include <sstream>

class BitDateTime
{
public:

	BitDateTime() : dateTime{ 0 } {};
	~BitDateTime() {};

	#pragma region Enums
	enum BitMask : uint32
	{
		mSeconds = 0x03F,
		mMinutes = 0xFC0,
		mHours = 0x1F000,
		mDay = 0x3E0000,
		mMonth = 0x3C00000,
		mYear = 0xFC000000,
	};

	enum BitShift : uint32
	{
		sSeconds = 0x00, //00
		sMinutes = 0x06, //06
		sHours = 0x0C, //12
		sDay = 0x11, //17
		sMonth = 0x16, //22
		sYear = 0x1A, //26
	};
	#pragma endregion

	#pragma region Data Time Access
	uint32 GetSeconds()
	{
		return dateTime & mSeconds;
	}
	void SetSeconds( uint32 value )
	{
		dateTime = ( dateTime & ~mSeconds ) | ( value & mSeconds );
	}

	uint32 GetMinutes()
	{
		return ( dateTime & mMinutes ) >> sMinutes;
	}
	void SetMinutes( uint32 value )
	{
		dateTime = ( dateTime & ~mMinutes ) | ( ( value << sMinutes ) & mMinutes );
	}

	uint32 GetHours()
	{
		return ( dateTime & mHours ) >> sHours;
	}
	void SetHours( uint32 value )
	{
		dateTime = ( dateTime & ~mHours ) | ( ( value << sHours ) & mHours );
	}

	uint32 GetDay()
	{
		return ( dateTime & mDay ) >> sDay;
	}
	void SetDay( uint32 value )
	{
		dateTime = ( dateTime & ~mDay ) | ( ( value << sDay ) & mDay );
	}

	uint32 GetMonth()
	{
		return ( dateTime & mMonth ) >> sMonth;
	}
	void SetMonth( uint32 value )
	{
		dateTime = ( dateTime & ~mMonth ) | ( ( value << sMonth ) & mMonth );
	}

	uint32 GetYear()
	{
		return ( ( dateTime & mYear ) >> sYear ) + 1970;
	}
	void SetYear( uint32 value )
	{
		ASSERT( value >= 1970 ); // nshit dude
		dateTime = ( dateTime & ~mYear ) | ( ( ( value - 1970 ) << sYear ) & mYear );
	}

	uint32 GetRaw()
	{
		return dateTime;
	}
	void SetRaw( uint32 dt )
	{
		dateTime = dt;
	}
	#pragma endregion

	#pragma region Parse Helpers

	bool SetFrom2C( pbyte buffer, BitMask bm ) // set value from two ascii chars 00: seconds, minutes, hours, month, day, year
	{
		bool result = true;
		uint32 value = ( ( buffer[ 0 ] - 48 ) * 10 ) + ( buffer[ 1 ] - 48 );
		switch ( bm )
		{
		case mSeconds:
			SetSeconds( value );
			break;
		case mMinutes:
			SetMinutes( value );
			break;
		case mHours:
			SetHours( value );
			break;
		case mDay:
			SetDay( value );
			break;
		case mMonth:
			SetMonth( value );
			break;
		case mYear: // Starting from 2000
			SetYear( 2000 + value );
			break;
		default:
			result = false;
			break;
		}
		return result;
	}

	bool SetYearFrom4C( pbyte buffer ) // set value from four ascii chars 0000
	{
		bool result = true;
		// 1970 - 2034
		uint32 value = (( buffer[ 0 ] - 48 ) * 1000 ) + ( ( buffer[ 1 ] - 48 ) * 100 ) + ( ( buffer[ 2 ] - 48 ) * 10 ) + ( buffer[ 3 ] - 48 );
		if ( value >= 1970 && value < 2034 )
		{
			SetYear( value );
		}
		else
		{
			result = false;
		}
		return result;
	}

	bool SetMonthFrom3C( pbyte buffer ) // set month value from three ascii chars Jan/Feb/Mar/Apr/May/Jun/Jul/Aug/Sep/Oct/Nov/Dec...
	{
		bool result = true;
		switch ( buffer[0] )
		{
			case 0x4A: // J - Jan/Jun/Jul
				if ( buffer[ 2 ] == 0x6C ) // J*l
				{
					SetMonth( 6 ); // Jul
				}
				else
				{
					if ( buffer[ 1 ] == 0x61 ) // Ja*
					{
						SetMonth( 0 ); // Jan
					}
					else
					{
						SetMonth( 5 ); // Jun
					}
				}
				break;

			case 0x4D: // M - Mar/May
				if ( buffer[ 2 ] == 0x72 ) // M*r
				{
					SetMonth( 2 ); // Mar
				}
				else
				{
					SetMonth( 4 ); // May
				}
				break;

			case 0x41: // A - Apr/Aug
				if ( buffer[ 2 ] == 0x72 ) // A*r
				{
					SetMonth( 3 ); // Apr
				}
				else
				{
					SetMonth( 7 ); // Aug
				}
				break;

			case 0x46: // F - Feb
				SetMonth( 1 ); // Feb
				break;

			case 0x53: // S - Sep
				SetMonth( 8 ); // Sep
				break;

			case 0x4F: // O - Oct
				SetMonth( 9 ); // Oct
				break;

			case 0x4E: // N - Nov
				SetMonth( 9 ); // Nov
				break;

			case 0x44: // D - Dec
				SetMonth( 11 ); // Dec
				break;

			default:
				result = false;
				break;
		}
		return result;
	}

	#pragma endregion

	std::string GetDateTime()
	{
		std::ostringstream stringStream;
		stringStream << GetYear() << "." << GetMonth() << "." << GetDay() << " " << GetHours() << ":" << GetMinutes() << ":" << GetSeconds();
		return std::string(stringStream.str());
	}

private:

	// 6 bits = 60(64) * 2 /mins - secs
	// 5 bits = 24(32) /hours
	// time = 17 bits
	//
	// 5 bits = 31(32) /day
	// 4 bits = 12(16) /month
	// 6 bits = 64 /year (starting from 1970)
	uint32 dateTime;
};

