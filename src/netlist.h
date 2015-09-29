#ifndef NETLIST_H_
#define NETLIST_H_

#include "header.h"
#include "statements.h"
#include "inputs_handler.h"

using namespace std;

class netlist;
class gate;
class net;
class pin;

string make_net_name(string wire_name, int i);
int return_state (string net_name);

class net
{
public:
	string name;
	list<pin *> connections_;
	int netState;
	gate *driver;
	net() : netState(-1), driver(NULL){};
	void append_pin(pin *p);
};

class pin
{
public:
	gate *gate_;
	size_t pin_index_;
	vector<net *> nets_;
	bool create(gate *g, size_t pin_index, const component_pin &pin, const map<string, net *> &nets_table);
};

class gate
{
public:
	string name;
	string type;
	vector<pin *> pins_;
	list<string> input_;
	list<string> output_;
	virtual bool validate_structural_semantics() = 0;
	virtual void compute() = 0;
	bool create(const evl_component &comp, const map<string, net *> &nets_table, const evl_wires_table &wires_table);
	bool create_pin(const component_pin &c_pin, size_t pin_index, const map<string, net *> &nets_table, const evl_wires_table &wires_table);
	virtual ~gate(){};
};

class flip_flop : public gate {
	int state_, next_state_;
public:
	flip_flop() : state_(0), next_state_(0){};
	bool validate_structural_semantics();
	void compute();
	~flip_flop(){};
}; // class flip_flop

class and_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~and_gate(){};
}; // class and_gate

class evl_one : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_one(){};
}; // class evl_one

class evl_zero : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_zero(){};
}; // class evl_zero

class evl_output : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_output(){};
}; // class evl_output

class evl_inputs_ : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_inputs_(){};
}; // class evl_inputs_

class evl_clock : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_clock(){};
}; // class evl_clock

class not_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~not_gate(){};
}; // class not_gate

class xor_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~xor_gate(){};
}; // class xor_gate

class or_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	virtual ~or_gate(){};
}; // class or_gate

class nor_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	virtual ~nor_gate(){};
}; // class nor_gate

class buf_gate : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~buf_gate(){};
}; // class buf_gate

class evl_lut : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_lut(){};
}; // class evl_lut

class evl_tris : public gate {
public:
	bool validate_structural_semantics();
	void compute();
	~evl_tris(){};
}; // class evl_tris

class netlist
{
	list<gate *> gates_;
	list<gate *> flip_flops_;
	list<gate *> one_zero_;
	list<gate *> evl_luts_;
	list<gate *> tris_;
	gate *inputs_;
	list<net *> nets_;
	map<string, net *> nets_table_;
public:
	bool create(const evl_wires &wires, const evl_components &comps, const evl_wires_table &wires_table);
	bool create_nets(const evl_wires &wires);
	void create_net(string net_name);
	bool create_gates(const evl_components &comps, const evl_wires_table &wires_table);
	bool create_gate(const evl_component &comp, const evl_wires_table &wires_table);
	bool save(const string &nl_file, const string module_name, const netlist &nl);
	void display(ostream &out, const string module_name, const netlist &nl);
	void simulate(const int cycles, const string out_put);
	void display_nets_table(ostream &out);
	void save_nets_table(map<string, int> &table);
	void flip_flop_handler();
	void zero_one_handler();
	void reset_nets();
	bool evl_inputs_checker(vector<int> &input_attr);
	void bin_inputer(bin_inputs &inputs);
};

#endif /* CLASS_H_ */
