#include "header.h"
#include "netlist.h"

using namespace std;
map<string, net *> nets_table;
map<string, int> dff_table;
int clk_flag = 0;
string global_clk;

string make_net_name(string wire_name, int i) {
		ostringstream oss;
		oss << wire_name << "[" << i << "]";
		return oss.str();
}

bool netlist::create(const evl_wires &wires,
		const evl_components &comps,
		const evl_wires_table &wires_table)
{
	bool result = create_nets(wires)
			&& create_gates(comps, wires_table);
	nets_table = nets_table_;
	save_nets_table(dff_table);
	return result;
}

bool netlist::create_nets(const evl_wires &wires) {
	for (evl_wires::const_iterator iter = wires.begin();
			iter != wires.end(); ++iter) {
		if ((*iter).width == 1) {
			create_net((*iter).name);
		}
		else {
			for (int i = 0; i < (*iter).width; ++i) {
				create_net(make_net_name((*iter).name, i));
			}
		}
	}
	return true;
}

void netlist::create_net(string net_name) {
	assert(nets_table_.find(net_name) == nets_table_.end());
	net *n = new net;
	n->name = net_name;
	nets_table_[net_name] = n;
	nets_.push_back(n);
}

bool netlist::create_gates(const evl_components &comps,
		const evl_wires_table &wires_table) {
	for (evl_components::const_iterator iter = comps.begin(); iter != comps.end();
			++iter) {
		create_gate((*iter), wires_table);
	}
	return true;
}

bool netlist::create_gate(const evl_component &comp,
		const evl_wires_table &wires_table) {
	if (comp.type == "evl_dff"){
		gates_.push_back(new flip_flop());
	} else if (comp.type == "and"){
		gates_.push_back(new and_gate());
	} else if (comp.type == "evl_input"){
		gates_.push_back(new evl_inputs_());
	} else if (comp.type == "evl_one"){
		gates_.push_back(new evl_one());
	} else if (comp.type == "evl_zero"){
		gates_.push_back(new evl_zero());
	} else if (comp.type == "evl_output"){
		gates_.push_back(new evl_output());
	} else if (comp.type == "evl_clock"){
		gates_.push_back(new evl_clock());
	} else if (comp.type == "not"){
		gates_.push_back(new not_gate());
	} else if (comp.type == "xor"){
		gates_.push_back(new xor_gate());
	} else if (comp.type == "or"){
		gates_.push_back(new or_gate());
	} else if (comp.type == "nor"){
		gates_.push_back(new nor_gate());
	} else if (comp.type == "buf"){
		gates_.push_back(new buf_gate());
	} else if (comp.type == "evl_lut"){
		gates_.push_back(new evl_lut());
	} else if (comp.type == "tris"){
		gates_.push_back(new evl_tris());
	} else {
		cerr << comp.type << " No Such Gate!!!" << endl;
	};
	return gates_.back()->create(comp, nets_table_, wires_table);
}

void netlist::display_nets_table(ostream &out) {
	for (map<string, net *>::const_iterator it = nets_table.begin();
			it != nets_table.end(); ++it) {
		out << "net " << it->first << " " << it->second->netState << endl;
	}
}

void netlist::reset_nets() {
	for (map<string, net *>::const_iterator it = nets_table.begin();
			it != nets_table.end(); ++it) {
		it->second->netState = -1;
	}
}

void netlist::save_nets_table(map<string, int> &table) {
	for (map<string, net *>::const_iterator it = nets_table.begin();
			it != nets_table.end(); ++it) {
		table[it->first] = it->second->netState;
	}
}

bool gate::create(const evl_component &comp,
		const map<string, net *> &nets_table,
		const evl_wires_table &wires_table){
	name = comp.name;
	type = comp.type;
	size_t pin_index = 0;
	for (component_pins::const_iterator iter = comp.pins.begin();
			iter != comp.pins.end(); ++iter) {
		create_pin((*iter), pin_index, nets_table, wires_table);
		++pin_index;
	}
	return validate_structural_semantics();
}

bool gate::create_pin(const component_pin &c_pin,
		size_t pin_index, const map<string, net *> &nets_table,
		const evl_wires_table &wires_table) {
	pin *p = new pin;
	pins_.push_back(p);
	return p->create(this, pin_index, c_pin, nets_table);
}

bool pin::create(gate *g, size_t pin_index, const component_pin &pin,
		const map<string, net *> &nets_table) {
	string net_name;
	pin_index_ = pin_index;
	gate_ = g;
	net *n_;
	if (pin.bus_msb == -1) { // a 1-bit wire
		net_name = pin.pin_name;
		n_ = new net;
		if (nets_table.find(net_name) != nets_table.end()){
			n_ = nets_table.find(net_name)->second;
			n_->append_pin(this);
			if (g->type != "evl_output" && pin_index_ == 0)
				n_->driver = g;
			nets_.push_back(n_);
		}
		else{
			int count = 0;
			string net_n;
			for (count = 0; nets_table.find(make_net_name(net_name,count)) != nets_table.end(); count++){
				 net_n= make_net_name(net_name, count);
				 n_ = nets_table.find(net_n)->second;
				 n_->append_pin(this);
				 if (g->type != "evl_output" && pin_index_ == 0)
					 n_->driver = g;
				 nets_.push_back(n_);
			}
		}
	}
	else {
		if (pin.bus_lsb == -1){//like in[0]
			net_name = make_net_name(pin.pin_name, pin.bus_msb);
			n_ = new net;
			if (nets_table.find(net_name) == nets_table.end()){
				cout << net_name << "netname error!" << endl;
				return false;
			}
			n_ = nets_table.find(net_name)->second;
			n_->append_pin(this);
			if (g->type != "evl_output" && pin_index_ == 0)
				n_->driver = g;
			nets_.push_back(n_);
		}
		else
		{
			int count;
			for (count = pin.bus_lsb; count<=pin.bus_msb; count++){
				net_name = make_net_name(pin.pin_name, count);
				n_ = new net;
				if (nets_table.find(net_name) == nets_table.end())
				{
					cout << net_name << "netname error!" << endl;
					return false;
				}
				n_ = nets_table.find(net_name)->second;
				n_->append_pin(this);
				if (g->type != "evl_output" && pin_index_ == 0)
					n_->driver = g;
				nets_.push_back(n_);
			}
		}
	}
	return true;
}

void net::append_pin(pin *p) {
	connections_.push_back(p);
}

void netlist::display(ostream &out,
		const string module_name, const netlist &nl){
	net *n;
	gate *g;
	pin *p;

	out << "module " << module_name << endl;
	out << "nets " << nets_.size() << endl;
	for (list<net *>::const_iterator nIter = nets_.begin();
			nIter != nets_.end(); ++nIter) {
		n = *(nIter);
		out << "  net " << n->name << " " << n->connections_.size() << endl;
		for (list<pin *>::const_iterator pIter = n->connections_.begin();
				pIter != n->connections_.end(); ++pIter){
			p = *(pIter);
			out << "    " << p->gate_->type;
			if (p->gate_->name != "")
			{

				out << " " << p->gate_->name << " " << p->pin_index_ << endl;
				// out << "1" << endl;
			}else{
				out << " " << p->pin_index_ << endl;
				// out << "2" << endl;
			}
		}
	}

	out << "components " << nl.gates_.size() << endl;
	for (list<gate *>::const_iterator gIter = gates_.begin();
			gIter != gates_.end(); ++gIter) {
		g = *(gIter);
		out << "  component " << g->type << " ";
		if (g->name != ""){
			out << g->name << " " << g->pins_.size() << endl;;
		}else{
			out << g->pins_.size() << endl;
		}

		for (vector<pin *>::const_iterator pIter = g->pins_.begin();
				pIter != g->pins_.end(); ++pIter){
			p = *(pIter);
			out << "    pin " << p->nets_.size() << " ";

			for (vector<net *>::const_iterator nIter = p->nets_.begin();
					nIter != p->nets_.end(); ++nIter){
				n = *(nIter);
				out << n->name <<" ";
			}
			out << endl;
		}
	}
}

bool netlist::save(const string &nl_file, const string module_name, const netlist &nl){
	ofstream output_file(nl_file.c_str());
	if (!output_file){
		cerr << "I can't write " << nl_file << ".netlist ." << endl;
		return false;
	}
	display(output_file, module_name, nl);
	return true;
}

int return_state (string net_name){
	return nets_table.find(net_name)->second->netState;
}

bool flip_flop::validate_structural_semantics() {
	if (pins_.size() != 3) return false;
	output_.clear();
	input_.clear();
	output_.push_back(pins_[0]->nets_[0]->name);
	input_.push_back(pins_[1]->nets_[0]->name);
	input_.push_back(pins_[2]->nets_[0]->name);
	return true;
}

void flip_flop::compute() {
	state_ = dff_table.find(input_.front())->second;
	if (state_ == -1)
		state_ = 0;
	nets_table.find(output_.front())->second->netState = state_;
}

bool and_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void and_gate::compute() {
	int update_flag = 0;
	for (list<string>::iterator it = input_.begin();
			it != input_.end(); ++it){
		if (return_state(*it)== 0){
			nets_table.find(output_.front())->second->netState = 0;
			break;
		} else if (return_state(*it)== -1){
			net *n = nets_table.find(*it)->second;
			if (n->driver != NULL){
				n->driver->compute();
				update_flag = 1;
				continue;
			}
			else {
				cerr << "no dirver for " << *it << endl;
				break;
			}
		} else {
			nets_table.find(output_.front())->second->netState = 1;
			continue;
		}
	}
	if (update_flag == 1)
		this->compute();
}

bool evl_one::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 1) return false;
	for (size_t i = 0; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			output_.push_back(n->name);
		}
	return true;
}

void evl_one::compute() {
	for (list<string>::iterator it = output_.begin();
			it != output_.end(); ++it){
		nets_table.find(*it)->second->netState = 1;
	}
}

bool evl_zero::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 1) return false;
	for (size_t i = 0; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			output_.push_back(n->name);
		}
	return true;
}

void evl_zero::compute() {
	for (list<string>::iterator it = output_.begin();
			it != output_.end(); ++it){
		nets_table.find(*it)->second->netState = 0;
	}
}

bool evl_output::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 1) return false;
	for (size_t i = 0; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			output_.push_back(n->name);
		}
	return true;
}

void evl_output::compute() {
}

bool evl_inputs_::validate_structural_semantics() {
	return true;
}

void evl_inputs_::compute() {
}

bool evl_clock::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() != 1) return false;
	global_clk = pins_[0]->nets_[0]->name;
	output_.push_back(global_clk);
	return true;
}

void evl_clock::compute() {
	clk_flag = 1;
}

bool not_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() != 2) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	input_.push_back(pins_[1]->nets_[0]->name);
	return true;
}

void not_gate::compute() {
	string temp = input_.front();
	if (return_state(temp) == 1){
		nets_table.find(output_.front())->second->netState = 0;
	} else if (return_state(temp) == 0){
		nets_table.find(output_.front())->second->netState = 1;
	} else {
		net *n = nets_table.find(temp)->second;
		if (n->driver != NULL){
			n->driver->compute();
			this->compute();
		}
		else {
			cerr << "no dirver for " << temp << endl;
		}
	}
}

bool xor_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void xor_gate::compute() {
	int count = 0;
	int update_flag = 0;
	for (list<string>::iterator it = input_.begin();
			it != input_.end(); ++it){
		if (return_state(*it)== 1){
			count ++;
			continue;
		} else if (return_state(*it)== -1){
			net *n = nets_table.find(*it)->second;
			if (n->driver != NULL){
				n->driver->compute();
				update_flag = 1;
				continue;
			}
			else {
				cerr << "no dirver for " << *it << endl;
				break;
			}
		}
	}
	if (count%2 == 0){
		nets_table.find(output_.front())->second->netState = 0;
	} else {
		nets_table.find(output_.front())->second->netState = 1;
	}
	if (update_flag == 1)
		this->compute();
}

bool or_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void or_gate::compute() {
	int update_flag = 0;
	for (list<string>::iterator it = input_.begin();
			it != input_.end(); ++it){
		if (return_state(*it)== 1){
			nets_table.find(output_.front())->second->netState = 1;
			break;
		} else if (return_state(*it)== -1){
			net *n = nets_table.find(*it)->second;
			if (n->driver != NULL){
				n->driver->compute();
				update_flag = 1;
				continue;
			}
			else {
				cerr << "no dirver for " << *it << endl;
				break;
			}
		} else {
			nets_table.find(output_.front())->second->netState = 0;
			continue;
		}
	}
	if (update_flag == 1)
		this->compute();
}

bool nor_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void nor_gate::compute() {
	int update_flag = 0;
	for (list<string>::iterator it = input_.begin();
			it != input_.end(); ++it){
		if (return_state(*it)== 1){
			nets_table.find(output_.front())->second->netState = 0;
			break;
		} else if (return_state(*it)== -1){
			net *n = nets_table.find(*it)->second;
			if (n->driver != NULL){
				n->driver->compute();
				update_flag = 1;
				continue;
			}
			else {
				cerr << "no dirver for " << *it << endl;
				break;
			}
		} else {
			nets_table.find(output_.front())->second->netState = 1;
			continue;
		}
	}
	if (update_flag == 1)
		this->compute();
}

bool buf_gate::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() != 2) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	input_.push_back(pins_[1]->nets_[0]->name);
	return true;
}

void buf_gate::compute() {
	string temp = input_.front();
	if (return_state(temp) == 1){
		nets_table.find(output_.front())->second->netState = 1;
	} else if (return_state(temp) == 0){
		nets_table.find(output_.front())->second->netState = 0;
	} else {
		net *n = nets_table.find(temp)->second;
		if (n->driver != NULL){
			n->driver->compute();
			this->compute();
		}
		else {
			cerr << "no dirver for " << temp << endl;
		}
	}
}

bool evl_lut::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() != 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void evl_lut::compute() {
	stringstream ss;
	for (list<string>::const_iterator liter=input_.begin();
			liter != input_.end(); liter++){
		int temp = return_state(*liter);
		ss << temp;
	}
	string addr(ss.str());
	//string str(s.rbegin(),s.rend());
	string word = addr;

	int i = 1;
	for (vector<net *>::const_iterator nIter = pins_[0]->nets_.begin();
				nIter != pins_[0]->nets_.end(); ++nIter){
		net *n = *(nIter);
		string temp = "0";
		stringstream ss1;
		ss1 << temp;
		int tempN;
		ss1 >> tempN;
		n->netState = tempN;
		i++;
	}
}

bool evl_tris::validate_structural_semantics() {
	output_.clear();
	input_.clear();
	if (pins_.size() < 3) return false;
	output_.push_back(pins_[0]->nets_[0]->name);
	for (size_t i = 1; i < pins_.size(); ++i)
		for (vector<net *>::const_iterator nIter = pins_[i]->nets_.begin();
				nIter != pins_[i]->nets_.end(); ++nIter){
			net *n = *(nIter);
			input_.push_back(n->name);
		}
	return true;
}

void evl_tris::compute() {
	int update_flag = 0;
	for (list<string>::iterator it = input_.begin();
			it != input_.end(); ++it){
		if (return_state(*it)== 1){
			nets_table.find(output_.front())->second->netState = 1;
			break;
		} else if (return_state(*it)== -1){
			net *n = nets_table.find(*it)->second;
			if (n->driver != NULL){
				n->driver->compute();
				update_flag = 1;
				continue;
			}
			else {
				cerr << "no dirver for " << *it << endl;
				break;
			}
		} else {
			nets_table.find(output_.front())->second->netState = 0;
			continue;
		}
	}
	if (update_flag == 1)
		this->compute();
}


void netlist::flip_flop_handler(){
	gate *g;
		for (list<gate *>::const_iterator gIter = flip_flops_.begin();
				gIter != flip_flops_.end(); ++gIter) {
			g = *(gIter);
			g->compute();
	}
}

void netlist::zero_one_handler(){
	gate *g;
		for (list<gate *>::const_iterator gIter = one_zero_.begin();
				gIter != one_zero_.end(); ++gIter) {
			g = *(gIter);
			g->compute();
	}
}

bool netlist::evl_inputs_checker(vector<int> &input_attr){
	if (int(inputs_->pins_.size()) != input_attr[0])
		return false;
	else{
		for (vector<pin *>::const_iterator pIter = inputs_->pins_.begin();
					pIter != inputs_->pins_.end(); ++pIter){
			int i = 1;
			pin *p = *(pIter);
			if (int(p->nets_.size()) != input_attr[i]){
					return false;
					break;
				}
			i++;
		}
	}
	return true;
}

void netlist::bin_inputer(bin_inputs &inputs){
	bin_input tempbin = inputs.front();
	for (vector<pin *>::const_iterator pIter = inputs_->pins_.begin();
				pIter != inputs_->pins_.end(); ++pIter){
		pin *p = *pIter;
		string pin_value = tempbin.input_.front();
		int i = 1;
		for (vector<net *>::const_iterator nIter = p->nets_.begin();
					nIter != p->nets_.end(); ++nIter){
			net *tempn = *(nIter);
			string temp = pin_value.substr(pin_value.size()-i,1);
			stringstream ss1;
			ss1 << temp;
			int tempN;
			ss1 >> tempN;
			tempn->netState = tempN;
			i++;
		}
		tempbin.input_.pop_front();
	}
	inputs.pop_front();
}

void netlist::simulate(const int cycles, const string out_put){
	gate *g;
	gate *outputG = NULL;
	pin *p;
	net *n;
	string outputN = "";
	string inputN = "";
	list<gate *> tempGList;
	inputs_ = NULL;
	int evl_clock = 0;

	//out << "transitions " << cycles << endl;

	for (list<gate *>::const_iterator gIter = gates_.begin();
				gIter != gates_.end(); ++gIter) {
			g = *(gIter);
			if (g->type == "evl_output"){
				outputN = g->name;
			}
			if (g->type == "evl_input"){
				inputN = g->name;
			}
			if (g->type == "evl_clock"){
				g->compute();
			}

			if (g->type == "evl_dff"){
				flip_flops_.push_back(g);
			} else if (g->type == "evl_one" || g->type == "evl_zero"){
				one_zero_.push_back(g);
			} else if (g->type == "evl_input"){
				inputs_ = g;
			} else if (g->type == "evl_lut"){
				evl_luts_.push_back(g);
			} else if (g->type == "tris"){
				tris_.push_back(g);
			} else {
				tempGList.push_back(g);
			}
	}

	string sim_file = string(out_put) + "." + outputN + ".evl_output";
	ofstream out(sim_file.c_str());

	evl_inputs inputs;
	bin_inputs bin_inputs;
	vector<int> input_attr;
    list<string> list6;
    list<string> list7;
	int input_flag = 0;

	if (inputs_ != NULL){
		string in_file = string(out_put) + "." + inputN + ".evl_input";
		extract_inputs_from_file(in_file.c_str(), inputs);
		input_attr = get_atrributes(inputs);
		evl_inputs_checker(input_attr);
		input_handler(inputs, bin_inputs, input_attr);
		input_flag = 1;
	}


	for (list<gate *>::const_iterator gIter = gates_.begin();
				gIter != gates_.end(); ++gIter) {
		g = *(gIter);
		if (g->type == "evl_output"){
			outputG = *(gIter);
			out << g->pins_.size() << endl;
			for (vector<pin *>::const_iterator pIter = g->pins_.begin();
					pIter != g->pins_.end(); ++pIter){
				p = *(pIter);
				out <<  p->nets_.size() << endl;
			}
		}
	}

	do{
    if (clk_flag == 1)
    	nets_table.find(global_clk)->second->netState = evl_clock;

    if (input_flag == 1)
    	//cout << bin_inputs.size() << endl;
    	bin_inputer(bin_inputs);

    if (flip_flops_.size() != 0)
    	flip_flop_handler();

    if (one_zero_.size() != 0)
        zero_one_handler();

    if (evl_luts_.size() != 0){
    	for (list<gate *>::const_iterator gIter = evl_luts_.begin();
    				gIter != evl_luts_.end(); ++gIter) {
    			g = *(gIter);
    			g->compute();
    	}
    }

    if (tris_.size() != 0){
        	for (list<gate *>::const_iterator gIter = tris_.begin();
        				gIter != tris_.end(); ++gIter) {
        		g = *(gIter);
        		g->compute();
        	}
        }

	for (list<gate *>::const_iterator gIter = tempGList.begin();
				gIter != tempGList.end(); ++gIter) {
			g = *(gIter);
			if (g->type != "evl_output"){
				g->compute();
			}
	}

	vector<string> hexOut;
	for (vector<pin *>::const_iterator pIter = outputG->pins_.begin();
						pIter != outputG->pins_.end(); ++pIter){
		p = *(pIter);
		int tempN = 0;
		int j = 0;
		stringstream ss;
		for (vector<net *>::const_iterator nIter = p->nets_.begin();
							nIter != p->nets_.end(); ++nIter){
			if (j == 4){
				ss << hex << tempN;
				tempN = 0;
				j = 0;
			}
			n = *(nIter);
			tempN = int((n->netState)*pow(2,j) + tempN);
			j++;
		}
		ss << hex << tempN;
		string s(ss.str());
		string str(s.rbegin(),s.rend());
		transform(str.begin(), str.end(), str.begin(), ::toupper);
		hexOut.push_back(str);
	}


	for (vector<string>::iterator it = hexOut.begin();
			it != hexOut.end(); ++it){
		out << (*it) << " ";
	}
	out << endl;
	//display_nets_table(cout);
	save_nets_table(dff_table);
	reset_nets();
	evl_clock++;
	} while (evl_clock<cycles);
}
