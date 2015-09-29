/*
 * statements.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: Fan
 */

#include "header.h"
#include "tokens.h"
#include "statements.h"

bool move_tokens_to_statement(
		evl_tokens &statement_tokens,
		evl_tokens &tokens)
{
	for (; !tokens.empty();)
	{
		statement_tokens.push_back(tokens.front());
		tokens.erase(tokens.begin());		// consume one token per iteration
		if (statement_tokens.back().str == ";")
		{
			break;		// exit if the ending ";" is found
		}
	}
	if (statement_tokens.back().str != ";")
	{
		cerr << "Look for ';' but reach the end of file" << endl;
		return false;
	}
	return true;
}

bool group_tokens_into_statements(
		evl_statements &statements,evl_tokens &tokens)
{
	for (; !tokens.empty();)		// generate one statement per iteration
	{
		evl_token token = tokens.front();
		if (token.type != evl_token::NAME)
		{
			cerr << "Need a NAME token but found '" << token.str
					<< "' on line " << token.line_no << endl;
			return false;
		}

		if (token.str == "module")	// MODULE statement
		{
			evl_statement module;
			module.type = evl_statement::MODULE;

			if (!move_tokens_to_statement(module.tokens, tokens))
			{
				return false;
			}

			statements.push_back(module);  // store statement into list
		}

		else if (token.str == "endmodule")  // ENDMODULE statement
		{
			evl_statement endmodule;
			endmodule.type = evl_statement::ENDMODULE;
			endmodule.tokens.push_back(token);
			tokens.erase(tokens.begin());
			statements.push_back(endmodule);
		}

		else if (token.str == "wire")  // WIRE statement
		{
			evl_statement wire;
			wire.type = evl_statement::WIRE;

			if (!move_tokens_to_statement(wire.tokens, tokens))
			{
				return false;
			}

			statements.push_back(wire);
		}

		else  //  Must be COMPONENT
		{
			evl_statement component;
			component.type = evl_statement::COMPONENT;

			if (!move_tokens_to_statement(component.tokens, tokens))
			{
				return false;
			}

			statements.push_back(component);
		}
	}
	return true;
}

bool process_module(evl_module &module, evl_statement &statement)
{
	for (; !statement.tokens.empty();
			statement.tokens.pop_front())
	{
		evl_token token = statement.tokens.front();
		if (token.str == "module")
		{
			continue;
		}
		else
		{
			module.name = token.str;  // store the module name for output
			break;
		}
	}
	return true;
}

bool process_wire_statment(evl_wires &wires, evl_statement &statement)
//  each wire statement is processed in this function
{
	enum stat_type {INIT, WIRE, DONE, WIRES, WIRE_NAME};  // 5 sates are needed
	stat_type state = INIT;  // to start, set initial stat to INIT

	for (; !statement.tokens.empty() && state != DONE;
			statement.tokens.pop_front())
		//  process tokens in each statement one by one until last one
	{
		evl_token token = statement.tokens.front();   // process first token in statement
		if ((state == INIT))
		{
			if (token.str == "wire")
			{
				state = WIRE;
			}
			else
			{
				cerr << "Need 'wire' but found '" << token.str
				<< "' on line " << token.line_no << endl;
				return false;
			}
		}
		else if (state == WIRE)   // process a single WIRE
		{
			if (token.str == "[")	// if a wire has a BUS, process in this loop
			{
				evl_wire wire;
				for (; !statement.tokens.empty();statement.tokens.pop_front())
				{
					token = statement.tokens.front();
					if (token.type == evl_token::NUMBER
							&& token.str != "0")
					{
						wire.width = atoi(token.str.c_str())+1;  // wire width
					}
					else if (token.type == evl_token::NAME)
					{
						wire.name = token.str;
						wires.push_back(wire);
						state = WIRE_NAME;  // get one wire with BUS
						break;
					}
				}
			}
			else if (token.type == evl_token::NAME)  // WIRE without BUS
			{
				evl_wire wire;
				wire.name = token.str;
				wire.width = 1;
				wires.push_back(wire);
				state = WIRE_NAME;
			}
			else
			{
				cerr << "Need NAME or '[' but found '" << token.str
						<< "' on line " << token.line_no << endl;
				return false;
			}
		}
		else if (state == WIRES)  // in a statement with multiple wires
		{
			if (token.type == evl_token::NAME)
			{
				evl_wire wire;
				wire.name = token.str;
				wire.width = wires.back().width;  // for mutilple wire statement
				                                  // all wire always have same width
				wires.push_back(wire);
				state = WIRE_NAME;
			}
			else
			{
				cerr << "Need NAME but found '" << token.str
						<< "' on line " << token.line_no << endl;
				return false;
			}
		}
		else if (state == WIRE_NAME)
				{
					if (token.str == ",")
					{
						state = WIRES;
					}
					else if (token.str == ";")
					{
						state = DONE;
					}
					else
					{
						cerr << "Need ',' or ';' but found '" << token.str
								<< "' on line " << token.line_no << endl;
						return false;
					}
				}
	}
	if (!statement.tokens.empty() || state != DONE)
	{
		cerr << "Something wrong with the statement" << endl;
		return false;
	}
	return true;
}

bool process_component_statment(evl_components &components, evl_statement &statement)
{
	// process component statement
	enum stat_type {INIT, DONE, PINS, PIN_NAME, BUS};  // total five states are needed
	stat_type state = INIT;
	evl_component component;
	component_pins pins;
	pins.clear();  // initiate the pins list

	for (; !statement.tokens.empty() && state != DONE;
			statement.tokens.pop_front())
		//compute state transitions
	{
		evl_token token = statement.tokens.front();
		if ((state == INIT))
		{
			if (token.type == evl_token::NAME)
			{
				component.type = token.str;
				statement.tokens.pop_front();
				token = statement.tokens.front();
				if (token.type == evl_token::NAME)
				{
					component.name = token.str;
				}
				else
				{
					component.name = "";
				}
			}
			else
			{
				cerr << "Need NAME but found '" << token.str
				<< "' on line " << token.line_no << endl;
				return false;
			}
			state = PINS;
		}

		else if (state == PINS)
		{
			component_pin pin;
			if (token.str == "(")
			{
				continue;
			}
			else if (token.type == evl_token::NAME)
			{
				pin.pin_name = token.str;
				pin.bus_msb = -1;
				pin.bus_lsb = -1;
				pins.push_back(pin);  // push one pin to pin list
				state = PIN_NAME;
			}
			else
			{
				cerr << "Need NAME or '(' but found '" << token.str
						<< "' on line " << token.line_no << endl;
				return false;
			}
		}

		else if (state == BUS)
		{
			component_pin pin = pins.back();  // if BUS exits, modify last pin
			for (; !statement.tokens.empty();statement.tokens.pop_front())
			{
				token = statement.tokens.front();
				if (token.type == evl_token::NUMBER)
				{
					pin.bus_msb = atoi(token.str.c_str());  // modify msb if exits
				}
				else if (token.str == ":")
				{
					statement.tokens.pop_front();
					evl_token token = statement.tokens.front();
					pin.bus_lsb = atoi(token.str.c_str());  // modify lsb if exits
				}
				else if (token.str == "]")
				{
					pins.pop_back();  // delete the original one
					pins.push_back(pin);  // push new one to list
					state = PIN_NAME;
					break;
				}
			}
		}

		else if (state == PIN_NAME)
				{
					if (token.str == ",")
					{
						state = PINS;  // next pin
					}
					else if (token.str == ")")
					{
						continue;  // one statement is over, skip to ';'
					}
					else if (token.str == ";")
					{
						state = DONE;
					}
					else if (token.str == "[")
					{
						state = BUS;
					}
					else
					{
						cerr << "Need ',' or ')' or '[' or ';' but found '"
								<< token.str
								<< "' on line " << token.line_no << endl;
						return false;
					}
				}
	}

	component.pins = pins;	// attach this pins list to component
	components.push_back(component);

	if (!statement.tokens.empty() || state != DONE)
	{
		cerr << "Something wrong with the statement" << endl;
		return false;
	}
	return true;
}

void print_pins(ostream &out, component_pins &pins)
	// print out all pins for a component in a requested manner
{
	for (component_pins::iterator iter = pins.begin();
			iter != pins.end(); ++iter)
		if ((*iter).bus_msb != -1
				&& (*iter).bus_lsb != -1)
		{
			out << "    pin " << (*iter).pin_name
					<< " " << (*iter).bus_msb << " "
					<< (*iter).bus_lsb << endl;
		}
		else if ((*iter).bus_msb != -1)
		{
			out << "    pin " << (*iter).pin_name
					<< " " << (*iter).bus_msb << endl;
		}
		else if ((*iter).bus_lsb != -1)
		{
			out << "    pin " << (*iter).pin_name
					<< " " << (*iter).bus_lsb << endl;
		}
		else
		{
			out << "    pin " << "1 " << (*iter).pin_name << endl;
		}
}

string process_statements(ostream &out,
		const evl_statements &statements,
		evl_wires &wires,
		evl_components &components)
{
	// display all statements
	//evl_wires wires;
	//wires.clear();
	//evl_components components;
	//components.clear();
	string module_name;

	int count = 1;
	for (evl_statements::const_iterator iter = statements.begin();
			iter != statements.end(); ++iter, ++count)
		{
		//  handle different type of statements one by one
		    if ((*iter).type == evl_statement::ENDMODULE)
			{
				//cout << "Statement " << count;
				//cout << ": ENDMODULE" << endl;
				//out << "endmodule" << endl;
			}
			else if ((*iter).type == evl_statement::MODULE)
			{
				evl_module module;
				evl_statement statement = *iter;
				process_module(module, statement);
				//cout << "Statement " << count;
				//cout << ": MODULE" << endl;
				module_name = module.name;
				//out << "module " << module.name << endl;
			}

			else if ((*iter).type == evl_statement::WIRE)
			{
				evl_statement statement = *iter;
				process_wire_statment(wires, statement);
				//cout << "Statement " << count;
				//cout << ": WIRE" << endl;
			}
			else if ((*iter).type == evl_statement::COMPONENT)
			{
				evl_statement statement = *iter;
				process_component_statment(components, statement);
				//cout << "Statement " << count;
				//cout << ": COMPONENT" << endl;
			}

		}
	return module_name;
}

bool store_statements_to_file(string file_name,
		const evl_statements &statements) {
	ofstream output_file(file_name.c_str());
	if (!output_file)
    {
        cerr << "I can't write " << file_name << ".syntax ." << endl;
        return -1;
    }		// verify output_file is ready
	//display_statements(output_file, statements);
	return true;
}

evl_wires_table make_wires_table(const evl_wires &wires) {
	evl_wires_table wires_table;
	for (evl_wires::const_iterator it = wires.begin();
			it != wires.end(); ++it) {
		evl_wires_table::iterator same_name = wires_table.find(it->name);
		if (same_name != wires_table.end()) {
			cerr << "Wire ¡¯" << it->name
					<< "¡¯is already defined" << endl;
			throw runtime_error("multiple wire definitions");
		}
		wires_table[it->name] = it->width;
	}
	return wires_table;
}

void display_wires_table(ostream &out,
		const evl_wires_table &wires_table) {
	for (evl_wires_table::const_iterator it = wires_table.begin();
			it != wires_table.end(); ++it) {
		out << "wire " << it->first
				<< " " << it->second << endl;
	}
}

