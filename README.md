# A Logic simulator for EasyVL Language
This is a digital logic simulator using the C++ that is able to simulate nontrivial digital systems including central processing units (CPU).

# Program Design
The logic simulation of “.evl” files is performed by this simulator. And based on the netlist analysis result, the simulation is realized. 

all needed information is saved in two lists (for wires and components) by parsing the input evl file, and then these two lists are used to construct the structure netlist. At last the netlist is used to do the simulations and saved the results into an `“.output”` file as required in a proper format. 

The gate class is inherited to all used gate types with two virtual constructor, `validate_structural_semantics()` and `compute()`. The former one is used to verify the inputs of each gate and setup input pins, output pins and drivers for each gate. And the latter one could compute the logic results. `“inputs_handler.cpp”` is created to support the data input and also the look-up-table. 

All method details are still hidden from user. For the `sim.cpp`, it is still easy to understand and all parsing and simulating processes are hidden from user.
