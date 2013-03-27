// String utilities.
//
// Written by Bernie Bright, started 1998
//
// Copyright (C) 1998  Bernie Bright - bbright@bigpond.net.au
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// $Id$

#include <ctype.h>
#include <cstring>
#include <sstream>

#include "strutils.hxx"

using std::string;
using std::vector;
using std::stringstream;

namespace simgear {
    namespace strutils {

	/**
	 * 
	 */
	static vector<string>
	split_whitespace( const string& str, int maxsplit )
	{
	    vector<string> result;
	    string::size_type len = str.length();
	    string::size_type i = 0;
	    string::size_type j;
	    int countsplit = 0;

	    while (i < len)
	    {
		while (i < len && isspace((unsigned char)str[i]))
		{
		    ++i;
		}

		j = i;

		while (i < len && !isspace((unsigned char)str[i]))
		{
		    ++i;
		}

		if (j < i)
		{
		    result.push_back( str.substr(j, i-j) );
		    ++countsplit;
		    while (i < len && isspace((unsigned char)str[i]))
		    {
			++i;
		    }

		    if (maxsplit && (countsplit >= maxsplit) && i < len)
		    {
			result.push_back( str.substr( i, len-i ) );
			i = len;
		    }
		}
	    }

	    return result;
	}

	/**
	 * 
	 */
	vector<string>
	split( const string& str, const char* sep, int maxsplit )
	{
	    if (sep == 0)
		return split_whitespace( str, maxsplit );

	    vector<string> result;
	    int n = std::strlen( sep );
	    if (n == 0)
	    {
		// Error: empty separator string
		return result;
	    }
	    const char* s = str.c_str();
	    string::size_type len = str.length();
	    string::size_type i = 0;
	    string::size_type j = 0;
	    int splitcount = 0;

	    while (i+n <= len)
	    {
		if (s[i] == sep[0] && (n == 1 || std::memcmp(s+i, sep, n) == 0))
		{
		    result.push_back( str.substr(j,i-j) );
		    i = j = i + n;
		    ++splitcount;
		    if (maxsplit && (splitcount >= maxsplit))
			break;
		}
		else
		{
		    ++i;
		}
	    }

	    result.push_back( str.substr(j,len-j) );
	    return result;
	}

	/**
	 * The lstrip(), rstrip() and strip() functions are implemented
	 * in do_strip() which uses an additional parameter to indicate what
	 * type of strip should occur.
	 */
	const int LEFTSTRIP = 0;
	const int RIGHTSTRIP = 1;
	const int BOTHSTRIP = 2;

	static string
	do_strip( const string& s, int striptype )
	{
	    string::size_type len = s.length();
	    if( len == 0 ) // empty string is trivial
		return s;
	    string::size_type i = 0;
	    if (striptype != RIGHTSTRIP)
	    {
		while (i < len && isspace(s[i]))
		{
		    ++i;
		}
	    }

	    string::size_type j = len;
	    if (striptype != LEFTSTRIP)
	    {
		do
		{
		    --j;
		}
		while (j >= 1 && isspace(s[j]));
		++j;
	    }

	    if (i == 0 && j == len)
	    {
		return s;
	    }
	    else
	    {
		return s.substr( i, j - i );
	    }
	}

	string
	lstrip( const string& s )
	{
	    return do_strip( s, LEFTSTRIP );
	}

	string
	rstrip( const string& s )
	{
	    return do_strip( s, RIGHTSTRIP );
	}

	string
	strip( const string& s )
	{
	    return do_strip( s, BOTHSTRIP );
	}

	string 
	rpad( const string & s, string::size_type length, char c )
	{
	    string::size_type l = s.length();
	    if( l >= length ) return s;
	    string reply = s;
	    return reply.append( length-l, c );
	}

	string 
	lpad( const string & s, size_t length, char c )
	{
	    string::size_type l = s.length();
	    if( l >= length ) return s;
	    string reply = s;
	    return reply.insert( 0, length-l, c );
	}

	bool
	starts_with( const string & s, const string & substr )
	{	
		return s.find( substr ) == 0;
	}

	bool
	ends_with( const string & s, const string & substr )
	{	
		size_t n = s.rfind( substr );
		return (n != string::npos) && (n == s.length() - substr.length());
	}

    string simplify(const string& s)
    {
        string result; // reserve size of 's'?
        string::const_iterator it = s.begin(),
            end = s.end();
    
    // advance to first non-space char - simplifes logic in main loop,
    // since we can always prepend a single space when we see a 
    // space -> non-space transition
        for (; (it != end) && isspace(*it); ++it) { /* nothing */ }
        
        bool lastWasSpace = false;
        for (; it != end; ++it) {
            char c = *it;
            if (isspace(c)) {
                lastWasSpace = true;
                continue;
            }
            
            if (lastWasSpace) {
                result.push_back(' ');
            }
            
            lastWasSpace = false;
            result.push_back(c);
        }
        
        return result;
    }
    
    int to_int(const std::string& s, int base)
    {
        stringstream ss(s);
        switch (base) {
        case 8:      ss >> std::oct; break;
        case 16:     ss >> std::hex; break;
        default: break;
        }
        
        int result;
        ss >> result;
        return result;
    }
    
    int compare_versions(const string& v1, const string& v2)
    {
        vector<string> v1parts(split(v1, "."));
        vector<string> v2parts(split(v2, "."));

        int lastPart = std::min(v1parts.size(), v2parts.size());
        for (int part=0; part < lastPart; ++part) {
            int part1 = to_int(v1parts[part]);
            int part2 = to_int(v2parts[part]);

            if (part1 != part2) {
                return part1 - part2;
            }
        } // of parts iteration

        // reached end - longer wins
        return v1parts.size() - v2parts.size();
    }
    
    string join(const string_list& l, const string& joinWith)
    {
        string result;
        unsigned int count = l.size();
        for (unsigned int i=0; i < count; ++i) {
            result += l[i];
            if (i < (count - 1)) {
                result += joinWith;
            }
        }
        
        return result;
    }
    
    string uppercase(const string &s) {
      string rslt(s);
      for(string::iterator p = rslt.begin(); p != rslt.end(); p++){
        *p = toupper(*p);
      }
      return rslt;
    }


#ifdef SG_WINDOWS
    #include <windows.h>
#endif
        
std::string convertWindowsLocal8BitToUtf8(const std::string& a)
{
#ifdef SG_WINDOWS
    DWORD flags = 0;
    std::vector<wchar_t> wideString;

    // call to query transform size
    int requiredWideChars = MultiByteToWideChar(CP_ACP, flags, a.c_str(), a.size(),
                        NULL, 0);
    // allocate storage and call for real
    wideString.resize(requiredWideChars);
    MultiByteToWideChar(CP_ACP, flags, a.c_str(), a.size(),
                        wideString.data(), wideString.size());
    
    // now convert back down to UTF-8
    std::vector<char> result;
    int requiredUTF8Chars = WideCharToMultiByte(CP_UTF8, flags,
                                                wideString.data(), wideString.size(),
                                                NULL, 0, NULL, NULL);
    result.resize(requiredUTF8Chars);
    WideCharToMultiByte(CP_UTF8, flags,
                        wideString.data(), wideString.size(),
                        result.data(), result.size(), NULL, NULL);
    return std::string(result.data(), result.size());
#else
    return a;
#endif
}

    } // end namespace strutils
    
} // end namespace simgear
