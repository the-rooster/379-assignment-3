/*
	C ECHO client example using sockets
*/
#include<stdio.h>	//printf
#include<string.h>	//strlen
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>
#include<bits/stdc++.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<string>
#include<iostream>
#include<ctime>
#include<sstream>
#include"tands.h"

using namespace std;

string output_file_name;
int out_file;
int transaction_count = 0;

void handle_exit(int SIG_EXIT){
	string out_msg = "Sent " + to_string(transaction_count) + " transactions.\n";
	write(out_file,out_msg.c_str(),out_msg.size());
	exit(0);
}

int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char message[1000] = {0};

	char server_reply[2000] = {0};
	
	char hostname[1000] = {0};

	gethostname(hostname,1000);

	output_file_name = string(hostname) + "." + to_string(getpid());
	string out_msg = "";

	out_file = open(output_file_name.c_str(),O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	

	int arg_size = 0;
	while(*(argv + arg_size) != NULL){arg_size++;}

	if(arg_size < 3){
		perror("Not enough arguments.\n");
		exit(0);
	}

	signal(SIGINT,handle_exit);

	string ip = *(argv + 2);

	int port = atoi(*(argv + 1));

	string print_port = "Using port " + to_string(port) + "\n";
	string print_ip = "Using server address " + ip + "\n";
	string print_host = "Host " + string(hostname) + "." + to_string(getpid()) + "\n";

	write(out_file,print_port.c_str(),print_port.size());
	write(out_file,print_ip.c_str(),print_ip.size());
	write(out_file,print_host.c_str(),print_host.size());
	
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		cout << "Could not create socket" << endl;
	}
	cout << "Socket created" << endl;
	
	server.sin_addr.s_addr = inet_addr(ip.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	cout << "Connected." << endl;


	if( send(sock , output_file_name.c_str() , output_file_name.size() , 0) < 0)
	{
		cout << "Send failed." << endl;
		return 1;
	}
	
	//keep communicating with server
	while(1)
	{
		cout << "Enter Message: ";
		cin >> message;

		if(message[0] == 'S' || message[0] == 's'){
			out_msg = "Sleep " + string(message + 1) + " units\n";
			write(out_file,out_msg.c_str(),out_msg.size());
			Sleep(atoi(message + 1));
			continue;
		}

		transaction_count++;

		stringstream time_precision;
		time_precision << fixed << setprecision(2) << (float) time(NULL) / 1000.0;
		out_msg = time_precision.str() + ": Send (" +string(message) + ")\n";
		write(out_file,out_msg.c_str(),out_msg.size());

		time_precision.flush();

		//Send some data
		if( send(sock , message , strlen(message) , 0) < 0)
		{
			cout << "Send failed." << endl;
			return 1;
		}
		
		//Receive a reply from the server
		if( recv(sock , server_reply , 2000 , 0) < 0)
		{
			cout << "recv failed." << endl;
			break;
		}
		
		cout << "Server reply: " << server_reply << endl;

		stringstream time_precision_2;

		time_precision_2 << fixed << setprecision(2) << (float) time(NULL) / 1000.0;

		out_msg = time_precision_2.str() + ": Recv (" + string(server_reply) + ")\n";
		write(out_file,out_msg.c_str(),out_msg.size());

		memset(message,0,1000);
		memset(server_reply,0,1000);
	}
	
	close(sock);
	return 0;
}