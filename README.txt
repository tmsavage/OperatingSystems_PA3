Assignment: Programming Assignment 3
Name: Toby Maxime Savage
Email: tosa5156@colorado.edu

###### FILES ######
1) multi-lookup.c
This file contains the main function as well as the requester and resolver threads. All declarations of variables and structs are declared here.

2) multi-lookup.c
This file contains the buffer struct that is passed to the requester and resolver threads. All variables that are necessary for this multithreaded program are declared dynamically here.

3) Makefile
This file is a given file by the course Professor/TAs. This allows the user to compile, or 'make' the program within the same directory.

4) performance.txt
This file contains the performance results of the following scenarios:
	- 1 requester thread and 1 resolver thread
	- 1 requester thread and 3 resolver threads
	- 3 requester threads and 1 resolver thread
	- 3 requester threads and 3 resolver threads
	- 5 requester threads and 5 resolver threads
This file is created each time you run the program. However, each time you run the program, the file will be overwritten with the specified number of requester and resolver threads.

###### SETUP and Testing ######
1) Place all files within the same directory.

2) Run 'make' to compile the program.

3) Execute the command by typing './multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ... ]' .