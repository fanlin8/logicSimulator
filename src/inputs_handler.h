/*
 * inputs_handler.h
 *
 *  Created on: Dec 5, 2014
 *      Author: Fan
 */

#ifndef INPUTS_HANDLER_H_
#define INPUTS_HANDLER_H_

#include "header.h"

using namespace std;

struct evl_input {
	vector<string> input_;
	int line_no;
};		// structure evl_input

typedef list<evl_input> evl_inputs;

struct bin_input {
	list<string> input_;
};		// structure bin_input

typedef list<bin_input> bin_inputs;

bool extract_inputs_from_line(string line, int line_no, evl_inputs &inputs);
bool is_space(char ch);
bool is_hex_space(char ch);
bool extract_inputs_from_file(string file_name, evl_inputs &inputs);
void display_inputs(ostream &out, const evl_inputs &inputs);
vector<int> get_atrributes(evl_inputs &inputs);
void input_handler(const evl_inputs &inputs, bin_inputs &bin_inputs_, const vector<int> &input_attr);
void creat_lut_table(const evl_inputs &inputs, const vector<int> &input_attr);
void display_lut_table(ostream &out);

#endif /* INPUTS_HANDLER_H_ */
