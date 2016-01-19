#pragma once
#include <string>

#ifndef SYSOUT_F
#define SYSOUT_F(f, ...)      _RPT1( 0, f, __VA_ARGS__ ) // For Visual studio
#endif

#ifndef speedtest__             
#define speedtest__(data)   for (long blockTime = NULL; (blockTime == NULL ? (blockTime = clock()) != NULL : false); SYSOUT_F(data "%.9fs", (double) (clock() - blockTime) / CLOCKS_PER_SEC))
#endif

class Util
{
public:
	static std::wstring ToWide(const std::string&, const std::locale& p_loc = std::locale(""));
	static std::string ToChar(const std::wstring& p_in, const std::locale& p_loc = std::locale(""));
};

