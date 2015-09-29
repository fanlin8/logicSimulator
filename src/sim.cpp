#include "header.h"
#include "netlist.h"

using namespace std;

void display_components(ostream &out, const evl_components components)
{
	out << "components " << components.size() << endl;	 //print components list
	for (evl_components::const_iterator iter = components.begin();
			iter != components.end(); ++iter)
	{
		component_pins pins;
		pins = (*iter).pins;	// in order to display pins' information

		if ((*iter).name == "")		// display pins with or without BUS in different formats
		{
			out << "  component " << (*iter).type << " "
					<<(*iter).name << pins.size() << endl;
			print_pins(out, pins);
		}
		else
		{
			out << "  component " << (*iter).type << " "
					<<(*iter).name << " " << pins.size() << endl;
			print_pins(out, pins);
		}
	}
}

// parse the input evl file
// store related information in two lists
bool parse_evl_file(string file_name, string &module_name,
		evl_wires &wires, evl_components &components)
{
	evl_tokens tokens;
	evl_statements statements;
    if (!extract_tokens_from_file(file_name, tokens))
    {
    	return -1;
    }
    if (!group_tokens_into_statements(statements, tokens))
    {
    	return -1;
    }
    module_name = process_statements(cout, statements, wires, components);
    return true;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
    {
       cerr << "You should provide a file name." << endl;
        return -1;
    }

    string evl_file = argv[1];//"D:\\Documents\\GitHome\\git\\flin\\mytests\\test_4.evl";
    string module_name;
    evl_wires wires;
    evl_components components;
    evl_wires_table wires_table;

    if (!parse_evl_file(evl_file, module_name, wires, components))
       {
       	return -1;
       }

    wires_table = make_wires_table(wires);

    netlist nl;
    if (!nl.create(wires, components, wires_table))
    {
    	return -1;
    }

    //display_wires_table(cout, wires_table);
    //nl.display(cout, module_name, nl);

    //string nl_file = string(argv[1]) + ".netlist";
    //nl.save(nl_file, module_name, nl);
    string sim_file = string(argv[1]);
    nl.simulate(1000, sim_file);
    //nl.display_nets_table(cout);

    return 0;
}
