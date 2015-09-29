/*
 * inputs_handler.cpp
 *
 *  Created on: Dec 5, 2014
 *      Author: Fan
 */

#include "header.h"
#include "inputs_handler.h"

using namespace std;

map<string, string> lut_table;

bool is_hex_space(char ch)
{
return (ch == ' ');
}

bool is_hex(char ch)
{
	return ((ch >= '0') && (ch <= '9'))       // 0 to 9
			|| ((ch >= 'a') && (ch <= 'f'))   // a to f
			|| ((ch >= 'A') && (ch <= 'F'));    // A to F
}

bool extract_inputs_from_line(string line, int line_no,
		evl_inputs &inputs)
{
	evl_input input;
	input.line_no = line_no;
	for (size_t i = 0; i < line.size();)
	{

		// spaces
		if (is_hex_space(line[i]))
		{
			++i; // skip this space character
			continue; // skip the rest of the iteration
		}

		// a hex digit
		else if (is_hex(line[i]))
		{
			size_t num_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!is_hex(line[i]))
				{
					break;
				}
			}
			string temp = string(line.substr(num_begin, i-num_begin));
			input.input_.push_back(temp);
			continue; // skip the rest of the iteration
		}

	    else
		{
			cerr << "LINE " << line_no
				<< ": invalid character" << endl;
			// cerr << "character is: " << line[i]; this is for test
			return false;
		}
	}

	inputs.push_back(input);
	return true;	// nothing left
}

bool extract_inputs_from_file(string file_name, evl_inputs &inputs)
{
    ifstream input_file(file_name.c_str());
    if (!input_file)
    {
        cerr << "I can't read " << file_name << endl;
        return false;
    }
    inputs.clear();		// be defensive, make it empty
    string line;
    for (int line_no = 1; getline(input_file, line); ++line_no)
    {
    	if (!extract_inputs_from_line(line, line_no, inputs))
    	{
    		return false;
    	}
    }
	return true;
}

void display_inputs(ostream &out, const evl_inputs &inputs)
{
	for (evl_inputs::const_iterator iter = inputs.begin();
			iter != inputs.end(); ++iter)
	{
		vector<string> input = (*iter).input_;
		for (vector<string>::iterator it = input.begin();
				it != input.end(); ++it)
		{
			out << (*it) << " ";
		}
		out << endl;
	}
}

vector<int> get_atrributes(evl_inputs &inputs)
{
	vector<string> atrributes = inputs.front().input_;
	inputs.pop_front();
	vector<int> atrributes_;
	for (vector<string>::iterator it = atrributes.begin();
			it != atrributes.end(); ++it)
	{
		int temp;
		stringstream ss(*it);
		ss >> temp;
		atrributes_.push_back(temp);
	}
	return atrributes_;
}

void input_handler(const evl_inputs &inputs, bin_inputs &bin_inputs_,
		const vector<int> &input_attr)
{
	bin_input bin_input;
	for (evl_inputs::const_iterator iter = inputs.begin();
			iter != inputs.end(); ++iter)
	{
		vector<string> input = (*iter).input_;
		list<string> b_input;
		bin_input.input_.clear();
		int no_of_tran;
		stringstream ss(input[0]);
		ss >> no_of_tran;
		for (size_t j=1 ; j < input.size() ; j++){
			stringstream ss1;
			ss1 << hex << input[j];
			unsigned n;
			ss1 >> n;
			int bit_n = input_attr[j];
			bitset<16> b(n);
			string temp = b.to_string();
			if (bit_n < 16)
				temp = temp.substr(16-bit_n,bit_n);
			b_input.push_back(temp);
		}
		bin_input.input_ = b_input;
		for (int i=0 ; i < no_of_tran ; i++){
			bin_inputs_.push_back(bin_input);
		}
	}
	if (bin_inputs_.size() < 1000){
		for (size_t k=bin_inputs_.size() ; k < 1000 ; k++){
			bin_inputs_.push_back(bin_input);
		}
	}
}

void creat_lut_table(const evl_inputs &inputs, const vector<int> &input_attr)
{
	int i = 0;
	for (evl_inputs::const_iterator iter = inputs.begin();
				iter != inputs.end(); ++iter)
	{
		vector<string> input = (*iter).input_;
		stringstream ss;
		ss << hex << i;
		unsigned addr;
		ss >> addr;
		int bit_addr = input_attr[1];
		bitset<16> a(addr);
		string tempaddr = a.to_string();
		if (bit_addr < 16)
			tempaddr = tempaddr.substr(16-bit_addr,bit_addr);
		stringstream ss1;
		ss1 << hex << input[0];
		unsigned n;
		ss1 >> n;
		int bit_n = input_attr[0];
		bitset<16> b(n);
		string temp = b.to_string();
		if (bit_n < 16)
			temp = temp.substr(16-bit_n,bit_n);
		lut_table[tempaddr] = temp;
		i++;
	}
}

void display_lut_table(ostream &out) {
	for (map<string, string>::const_iterator it = lut_table.begin();
			it != lut_table.end(); ++it) {
		out << "add " << it->first << " word " << it->second << endl;
	}
}


