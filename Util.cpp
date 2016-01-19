#include "Util.h"

std::wstring Util::ToWide(const std::string& p_in, const std::locale& p_loc)
{
	try{
		std::wstring out(p_in.length(), 0);
		std::string::const_iterator i = p_in.begin(), ie = p_in.end();
		std::wstring::iterator j = out.begin();
		for (; i != ie; ++i, ++j)
			*j = std::use_facet<std::ctype< wchar_t > >(p_loc).widen(*i);
		return out;
	}
	catch (...)
	{
		return std::wstring();
	}
}

std::string Util::ToChar(const std::wstring & p_in, const std::locale & p_loc)
{
	typedef std::wstring::traits_type::state_type       TsType;
	typedef std::codecvt<wchar_t, char, TsType>         Tcvt;

	try{
		std::string     l_out(p_in.length(), char());
		const Tcvt &    l_cvt = std::use_facet<Tcvt>(p_loc);
		const wchar_t*  l_from_nxt;
		char*           l_to_nxt;
		TsType          l_state = TsType();
		l_cvt.out(l_state, &p_in[0], &p_in[0] + p_in.size(), l_from_nxt, &l_out[0], &l_out[0] + l_out.size(), l_to_nxt);
		return l_out;
	}
	catch (...)
	{
		return std::string();
	}
};