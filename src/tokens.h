#ifndef TOKENS_H_
#define TOKENS_H_

#include "header.h"

using namespace std;

struct evl_token {
	enum token_type {NAME, NUMBER, SINGLE};
	token_type type;
	string str;
	int line_no;
};		// structure evl_token

typedef list<evl_token> evl_tokens;

bool is_space(char ch);
bool is_punctuation(char ch);
bool is_digit(char ch);
bool is_letter(char ch);
bool extract_tokens_from_line(string line, int line_no, evl_tokens &tokens);
bool extract_tokens_from_file(string file_name, evl_tokens &tokens);
void display_tokens(ostream &out, const evl_tokens &tokens);
bool store_tokens_to_file(string file_name, const evl_tokens &tokens);

#endif /* TOKENS_H_ */
