# Programming Listing
## `header.h` 
Header information for all other programs. Introduced for simplicity.
## `tokens.cpp` & `tokens.h` 
Extract tokens from input file and store them in a list.
## `statements.cpp` & `statements.h`
Process the tokens and group them into four different structure.
## `netlist.cpp` & `netlist.h`
All classes are constructed here. And then the codes will build the netlist and do the simulations, all computed results will be save into a specific file.
## `inputs_handler.cpp` & `inputs_handler.h`
The program to read a pre-defined file in required format. The data in the file will be read and converted to binary values, and pass these values to main function. And the look-up-table is also created by these codes.
## `sim.cpp`
Main function, which provides simple interface for user, and could hide all processing detail from user.
