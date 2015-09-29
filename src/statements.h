#ifndef STATEMENTS_H
#define STATEMENTS_H

#include "header.h"
#include "tokens.h"

using namespace std;

// use list to store all structures

struct evl_statement
{
	enum statement_type {MODULE, WIRE, COMPONENT, ENDMODULE};
	statement_type type;
	evl_tokens tokens;
};		// structure evl_statement

typedef list<evl_statement> evl_statements;

struct evl_module
{
	string name;
};		// structure evl_module

struct evl_wire
{
	string name;
	int width;
};		// structure evl_wire

typedef list<evl_wire> evl_wires;

struct component_pin
{
	string pin_name;
	int bus_msb;
	int bus_lsb;
};		// structure component_pin

typedef list<component_pin> component_pins;

struct evl_component
{
	string type;
	string name;
	component_pins pins;
};		// structure evl_component, store pin list

typedef list<evl_component> evl_components;

typedef map<string, int> evl_wires_table;

bool move_tokens_to_statement(evl_tokens &statement_tokens, evl_tokens &tokens);
bool group_tokens_into_statements(evl_statements &statements, evl_tokens &tokens);
bool process_module(evl_module &module, evl_statement &statement);
bool process_wire_statment(evl_wires &wires, evl_statement &statement);
bool process_component_statment(evl_components &components, evl_statement &statement);
void print_pins(ostream &out, component_pins &pins);
string process_statements(ostream &out,
		const evl_statements &statements, evl_wires &wires, evl_components &components);
bool store_statements_to_file(string file_name, const evl_statements &statements);
evl_wires_table make_wires_table(const evl_wires &wires);
void display_wires_table(ostream &out, const evl_wires_table &wires_table);

#endif
