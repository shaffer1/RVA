//##########################################################
// Author: Nehme Bilal
// Date: Nov 2011
// Contact: nehmebilal@gmail.com
//##########################################################

/**
This class group all string utilities methods in one place.
*/

#ifndef _STRING_UTILITIES_H 
#define _STRING_UTILITIES_H

#include <string>
#include <vector>


using std::string;
using std::vector;

class StringUtilities
{
public:

	// Description:
	// splits a string with a given separator and add the 
	// tokens to a container that supports push_back and clear operations.
	template <class container>
	static void split(const string& str, container& tokens,
		const string& delimiters)
	{
		tokens.clear();
		if( !str.length() )
			return;

		// Skip delimiters at beginning.
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		string::size_type pos     = str.find_first_of(delimiters, lastPos);

		while (string::npos != pos || string::npos != lastPos)
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

	// Description:
	// removes white spaces from a string
	inline static void trim(string& input);

	// Description:
	// return true if the string input ends with the string value.
	// false otherwise
	inline static bool endsWith(const string& input, const string& value);

};

//____________________________________________________________________
inline void StringUtilities::trim(std::string &input)
{
	throw("not implemented yet");
}

//____________________________________________________________________
inline bool StringUtilities::endsWith(
			const std::string &input, const std::string &value)
{
	throw("not implemented yet");
}

#endif
