/*
 * tokens.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: Fan
 */

#include "header.h"
#include "tokens.h"

using namespace std;

bool is_space(char ch)
{
return (ch == ' ') || (ch == '\t')
		|| (ch == '\r') || (ch == '\n');
}

bool is_punctuation(char ch)
{
	return (ch == '(') || (ch == ')')
			|| (ch == '[') || (ch == ']')
			|| (ch == ':') || (ch == ';')
			|| (ch == ',');
}

bool is_digit(char ch)
{
	return (ch >= '0') && (ch <= '9');		// 0 to 9
}

bool is_letter(char ch)
{
	return ((ch >= 'a') && (ch <= 'z'))       // a to z
			|| ((ch >= 'A') && (ch <= 'Z'))    // A to Z
			|| (ch == '_');
}

bool extract_tokens_from_line(string line, int line_no,
		evl_tokens &tokens)		// use reference to modify an input line
{
	for (size_t i = 0; i < line.size();)
	{
		// comments
		if (line[i] == '/')
		{
			++i;
			if ((i == line.size()) || (line[i] != '/'))
			{
				cerr << "LINE " << line_no
					<< ": a single / is not allowed" << endl;
				return -1;
			}
			break; // skip the rest of the line by exiting the loop
		}

		// spaces
		else if (is_space(line[i]))
		{
			++i; // skip this space character
			continue; // skip the rest of the iteration
		}

		// SINGLE
		else if (is_punctuation(line[i]))
		{
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::SINGLE;
			token.str = string(1, line[i]);
			tokens.push_back(token);
			++i; // we consumed this character
			continue; // skip the rest of the iteration
		}

		// NUMBER
		else if (is_digit(line[i]))
		{
			size_t num_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!is_digit(line[i]))
				{
					break; // [num_begin, i-num_begin) is the number token
				}
			}
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::NUMBER;
			token.str = string(line.substr(num_begin, i-num_begin));
			tokens.push_back(token);
			// ++i; // we consumed this character
			continue; // skip the rest of the iteration
		}

		// NAME
		else if (is_letter(line[i]))
		{
			size_t name_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!((is_letter(line[i]))
						|| is_digit(line[i])
						|| (line[i] == '$')))
				{
					break; // [name_begin, i) is the range for the token
				}
			}
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::NAME;
			token.str = string(line.substr(name_begin, i-name_begin));
			tokens.push_back(token);
			// ++i; // we consumed this character
			continue; // skip the rest of the iteration
		}

	    else
		{
			cerr << "LINE " << line_no // << " no." << i
				<< ": invalid character" << endl;
			// cerr << "character is: " << line[i]; this is for test
			return false;
		}
	}
	return true;	// nothing left
}

bool extract_tokens_from_file(string file_name, evl_tokens &tokens)
// use reference to modify an input file
{
    ifstream input_file(file_name.c_str());
    if (!input_file)
    {
        cerr << "I can't read " << file_name << "." << endl;
        return false;
    }
    tokens.clear();		// be defensive, make it empty
    string line;
    for (int line_no = 1; getline(input_file, line); ++line_no)
    {
    	if (!extract_tokens_from_line(line, line_no, tokens))
    	{
    		return false;
    	}
    }
	return true;
}

void display_tokens(ostream &out, const evl_tokens &tokens)
		// display_tokens now is in list form
{
	for (evl_tokens::const_iterator iter = tokens.begin();
			iter != tokens.end(); ++iter)
	{
		if ((*iter).type == evl_token::SINGLE)
		{
			out << "SINGLE " << (*iter).str << endl;
		}
		else if ((*iter).type == evl_token::NAME)
		{
			out << "NAME " << (*iter).str << endl;
		}
		else		// Must be NUMBER
		{
			out << "NUMBER " << (*iter).str << endl;
		}
	}
}

bool store_tokens_to_file(string file_name,
		const evl_tokens &tokens) {
	ofstream output_file(file_name.c_str());
	if (!output_file)
    {
        cerr << "I can't write " << file_name << ".tokens ." << endl;
        return -1;
    }		// verify output_file is ready
	display_tokens(output_file, tokens);
	return true;
}


