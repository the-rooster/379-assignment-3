Andrew Culberson - CMPUT 379 - Assignment #3

Name of each file and summary of their contents:

    client.cc: source code for client executable.
    server.cc: source code for server executable.
    tands.cc: provided code for Sleep and Trans method.

Assumptions made in doing this assignment:

    I assumed that the program will be stopped by using CTRL + C. This calculated the end statistics and closes the program.

Summary of approach:
    The server uses one thread for listening for new clients, any number of threads assigned to each individual client,
    and another thread for processing transactions sent by all of the clients.

    The client is a single threaded program that simply sends transactions and waits for the finished transaction message (D<n>). It can also take in sleep
    commands as specified in the assignment description.

Instructions to compile and run:
    Simply run the command "make" in the src directory to compile both the server and client.
    The compiled executables have attached man pages (client.man and server.man) which can be viewed with
    the command "groff -Tascii -man <man page>".
