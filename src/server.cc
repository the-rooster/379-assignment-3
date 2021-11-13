/*
	C socket server example
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<bits/stdc++.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<fcntl.h>
#include<iostream>
#include<thread>
#include<mutex>
#include<vector>
#include<queue>
#include<sstream>
#include<chrono>
#include"tands.h"

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

vector<thread> clients;

int curr_work_id = 1;

struct work_item{
	public:

	int client_sock;
	int work_id;
	int t_n;
	int client_id;
	string client_name;

	work_item(int a,int b,int c,string d,int e){
		client_sock = a;
		work_id = b;
		t_n = c;
		client_name = d;
		client_id = e;
	}
};

queue<struct work_item*> work;

mutex work_lock;

std::mutex write_lock;
string output_file_name = "";
int out_file = -1;

clock_t start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

clock_t last_instruction = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

vector<pair<string,int>> work_counter;


void detect_timeout(){
	clock_t timeout = 30 * 1000;
	while(true){
		//cout << (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - last_instruction) << endl;
		if((duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - last_instruction) >= timeout){
			kill(getpid(),SIGINT);
		}
		sleep(0);
	}
}

void exit_handler(int SIG){
	write_lock.lock();

	string out_msg = "SUMMARY\n";
	write(out_file,out_msg.c_str(),out_msg.size());

	int total_transactions = 0;

	for(int i=0;i<work_counter.size();i++){
		out_msg = "\t" + to_string(work_counter.at(i).second) + " transactions from " + work_counter.at(i).first + "\n";
		write(out_file,out_msg.c_str(),out_msg.size());
		total_transactions += work_counter.at(i).second;
	}

	out_msg = to_string(((float) total_transactions / (last_instruction - start_time))*1000) + " transactions/sec (" + to_string(total_transactions) + "/"  + to_string((last_instruction - start_time) / 1000.0) + ")\n";
	write(out_file,out_msg.c_str(),out_msg.size());

	write_lock.unlock();

	exit(0);

}

void worker_thread(){

	while(true){
		work_lock.lock();

		if(work.size() > 0){
			struct work_item * item = work.front();
			work.pop();

			work_lock.unlock();
			Trans(item->t_n);

			string out_msg = "D" + to_string(item->work_id);
			write(item->client_sock,out_msg.c_str(),out_msg.size());

			work_counter.at(item->client_id).second += 1;

			write_lock.lock();

			stringstream time_precision;
			time_precision << fixed << setprecision(2) << (float) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;

			out_msg = time_precision.str() + ": #" + to_string(item->work_id) + " (DONE) from host " + item->client_name + "\n";
			write(out_file,out_msg.c_str(),out_msg.size());

			write_lock.unlock();
			sleep(0);

			// string work_finished = "D" + to_string(item->work_id);
			// cout << "WORK FINISHED: " << "D" + to_string(item->work_id) << endl;
			// sleep(0);
		}
		else{
			work_lock.unlock();
			sleep(0);
		}

	}
}

void client_thread(int client_sock,string client_name,int client_id){
	int read_size;
	char client_message[2000] = {0};

	//Receive a message from client
	while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
	{
		if(client_message[0] == 'T' || client_message[0] == 't'){
			


			//add work to queue
			work_lock.lock();

			struct work_item item(client_sock,curr_work_id++,atoi(client_message + 1),client_name,client_id);

			work.push(&item);

			last_instruction = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

			work_lock.unlock();

			//denote that transaction has been received.
			write_lock.lock();

			stringstream time_precision;
			time_precision << fixed << setprecision(2) << (float) duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;

			string out_msg = time_precision.str() + ": #" + to_string(item.work_id) + " (T" + to_string(item.t_n) + ") from host " + client_name + "\n";

			write(out_file,out_msg.c_str(),out_msg.size());

			write_lock.unlock();
		}
		//Send the message back to client
		//write(client_sock , client_message , strlen(client_message));
		memset(client_message, 0, 2000);
	}
	
	if(read_size == 0)
	{
		cout << "Client Disconnected." << endl;
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , read_size , c;
	struct sockaddr_in server;
	char client_message[2000];
	struct sockaddr_in client;

	char hostname[1000] = {0};
	gethostname(hostname,1000);

	output_file_name = string(hostname) + "." + to_string(getpid());
	out_file = open(output_file_name.c_str(),O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	string out_msg = "";
	
	int argv_size = 0;
	while(*(argv + argv_size) != NULL){argv_size++;}

	if(argv_size < 2){
		cout << "Not enough arguments." << endl;
		exit(0);
	}

	int port = atoi(*(argv + 1));
	
	out_msg = "Using port " + to_string(port) + "\n";
	write(out_file,out_msg.c_str(),out_msg.size());


	signal(SIGINT,exit_handler);
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		cout << "Could not create socket." << endl;
	}
	cout << "Socket created." << endl;
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	cout << "Bind done!" << endl;

	
	//start thread for detecting timeout
	thread timeout_thread = thread(detect_timeout);

	//start worker thread for processing client transactions
	thread consumer_thread = thread(worker_thread);

	
	int curr_id = 0;

	//add incoming clients to their own unique thread.
	while(1){
		//Listen
		listen(socket_desc , 3);

		//Accept and incoming connection
		cout << "Waiting for incoming connections..." << endl;
		c = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}
		cout << "Connection Accepted." << endl;

		char hostname[2000] = {0};

		//Receive a reply from the server
		if( recv(client_sock , hostname , 2000 , 0) < 0)
		{
			cout << "recv failed." << endl;
			break;
		}

		start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		
		//put each client onto their own thread
		clients.push_back(thread(client_thread,client_sock,string(hostname),curr_id++));
		work_counter.push_back(pair<string,int>(string(hostname),0));
	}
	
	return 0;
}