#include "socket_layer.h"
#include <iostream>
#define debug(A) printf("%d\n",A)
using namespace std;

extern Message_queue pastry_api_overlay_in, pastry_api_user_in;
extern Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
//extern Message_queue pastry_socket_overlay_in;

//Server extracts the nodeid,ip and port from the message sent by the sender using this function.
string extract_socket_info(string data,int *node_id,string *ip,int *port) {

	char other_data[BUFFER_SIZE];
	char c_ip[20];//Storing IP address
	sscanf(data.c_str(),"%d#%[^#]#%d#%s",node_id,c_ip,port,other_data);
	*ip = string(c_ip);
	return string(other_data);

}

//This function is used for Server-Setup
int Socket_layer :: init(int cur_node_id,string ip, int port) {

	this -> cur_node_id = cur_node_id;
	this -> cur_ip = ip;
	this -> cur_port = port;
	printf("Initializing socket interface on %s %d",ip.c_str(),port);

	if ((incoming_socket = socket(AF_INET,SOCK_STREAM,0)) == 0) {
		perror("Error creating incoming socket");
		return 0;
	}
	else printf("Created Incoming socket\n");

	int opt = 1;
	if (setsockopt(incoming_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    { 
        perror("setsockopt failure"); 
        return 0;
    } 

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;
    int addrlen = sizeof(address);

    if (::bind(incoming_socket,(struct sockaddr *)&address,(socklen_t)addrlen) < 0) {
    	perror("bind failure\n");
    	return 0;
    }
    else printf("Successfully bound socket\n");

    if (listen(incoming_socket,DEFAULT_QUEUE) < 0) {
    	perror("listen failed");
    	return 0;
    }
    else printf("Listening to incoming connections \n");
    incoming_thread = new thread(&Socket_layer :: incoming_conn,this);
   // overlay_thread = new thread(&Socket_layer :: recv_overlay,this);

}	



void Socket_layer :: incoming_conn() {

	char input_buffer[BUFFER_SIZE];
    printf("listen thread running \n");
	while(1) {

		int conn = accept(incoming_socket, NULL,NULL);
		if(conn < 0) 
			perror("Failed to accept connection");
		else {

			printf("New connection encountered\n");
			memset(input_buffer,0,BUFFER_SIZE);
			int inp = 0;
			while(read(conn,input_buffer + inp,1) && input_buffer[inp++] != '|');
			input_buffer[inp-1] = '\0';
			string data(input_buffer);
			int node_id, port;
			string ip;
			string payload = extract_socket_info(data,&node_id,&ip,&port);
			add_ip_port(node_id,ip,port);
			
			//printf("%s\n",payload.c_str());
			recent_conn_mutex.lock();
			auto p = recent_conn.find(node_id);
			if(p != recent_conn.end()) {
				shutdown(p -> second,SHUT_WR);
				p -> second = conn;
			} 
			else recent_conn.insert(pair<int,int>(node_id,conn));
			recent_conn_mutex.unlock();

			//message *new_mem = new message();
			//new_mem -> type = ADD_NODE;
			//new_mem -> data = to_string(node_id);
			//while(!pastry_socket_overlay_in.add_to_queue(new_mem));

			thread *recv_thread = new thread(&Socket_layer :: recv_node,this,conn,node_id,string(payload.c_str()));
			printf("pastry connection established with %d %s \n",node_id,ip.c_str());
		}
	}

}

string Socket_layer :: get_ip_port(int node_id) {

	string ip,port;
	socket_mutex.lock();
	auto i = ip_list.find(node_id);
	if (i != ip_list.end()) {
		ip = i -> second;
	}
	else ip = "0";
	auto p = port_list.find(node_id);
	if (p != port_list.end()) {
		port = to_string(p -> second);
	}
	else port = "0";
	socket_mutex.unlock();
	return ip + string("#") + port;
}

void Socket_layer :: remove_ip_port(int nodeid) {
	
	socket_mutex.lock();
	ip_list.erase(nodeid);
	port_list.erase(nodeid);
	socket_mutex.unlock();

	recent_conn_mutex.lock();
	auto p = recent_conn.find(nodeid);
	if(p != recent_conn.end()) {
		printf("connection %d closed \n",p -> second);
		close(p -> second);
		//printf("%d\n",p -> second);
		recent_conn.erase(p);
	}
	recent_conn_mutex.unlock();
}


void Socket_layer :: add_ip_port(int node_id,string ip,int port) {

	//printf("node id %d ip %s port %d\n",node_id,ip.c_str(),port);

	socket_mutex.lock();
	auto i = ip_list.find(node_id);
	if (i == ip_list.end()) {
		ip_list.insert(pair<int,string>(node_id,ip));
	}
	else i -> second = ip;
	auto p = port_list.find(node_id);
	if (p == port_list.end()) {
		port_list.insert(pair<int,int>(node_id,port));
	}
	else p -> second = port;
	socket_mutex.unlock();

}


//This function is used for Client-Setup

int Socket_layer :: send_data(int node_id,string message) {

	int conn;
	recent_conn_mutex.lock();
	auto p = recent_conn.find(node_id);
	recent_conn_mutex.unlock();

	int rtype;
	sscanf(message.c_str(),"%d#",&rtype);

	string send_string = "";
	if(p == recent_conn.end()) {

		if(ip_list.find(node_id) == ip_list.end()) return 0;
		else {

			printf("Establishing connection to %d\n",node_id);
			string ip = ip_list[node_id];
			struct sockaddr_in serv_address;
			serv_address.sin_family = AF_INET;
			serv_address.sin_port = port_list[node_id];
			if((conn = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Failed to create socket");
				return 0;
			}
			inet_pton(AF_INET, ip.c_str(), &serv_address.sin_addr);
			if (connect(conn, (struct sockaddr *)&serv_address, (socklen_t)sizeof(serv_address)) < 0) { 
		    	printf("Failed to established connection %s %d\n",ip.c_str(),(int)serv_address.sin_port);
		       	return 0; 
		    } 
		    recent_conn_mutex.lock();
		    recent_conn.insert(pair<int,int>(node_id,conn));
		    recent_conn_mutex.unlock();

		    send_string = to_string(cur_node_id) + string("#") + cur_ip + string("#") + to_string((int)address.sin_port) + "#";
		    thread *recv_thread = new thread(&Socket_layer::recv_node,this,conn,node_id,"");

		    printf("connection established\n");

		}
	}
	recent_conn_mutex.lock();
	conn = recent_conn[node_id];
	recent_conn_mutex.unlock();

	send_string = send_string + message + string("|");
	if(write(conn,send_string.c_str(),send_string.size()) == -1) {
		printf("failed to send data\n");
		return 0;
	}
	printf("Sent message to %d type %d \n",node_id,rtype);
	return 1;

}

void Socket_layer :: recv_node(int conn,int node_id,string data) {

	char input_buffer[BUFFER_SIZE];
	int ret;
	do {
		printf("\n message %s\n", data.c_str());
		if(data.size() > 0) {
			message *mem = extract_message(data);
			printf("message Received from node %d type %d \n",node_id,mem -> type);
			if(mem -> type == EXIT) {
				recent_conn_mutex.lock();
				recent_conn.erase(node_id);
				recent_conn_mutex.unlock();
				close(conn);
				break;
			}
			else {
				while(!pastry_socket_overlay_in.add_to_queue(mem));
			}

		}
		data.clear();
		//printf("\nhere\n");
		int ip = 0;
		while((ret = read(conn,input_buffer + ip,1)) && input_buffer[ip++] != '|');
			//printf("%c ",input_buffer[ip-1]);
		/*
		ret = recv(conn,input_buffer,BUFFER_SIZE,MSG_PEEK);
		if(!ret) break;
		while(input_buffer[ip++] != '|');
		input_buffer[ip-1] = '\0';
		data = string(input_buffer);
		ret = recv(conn,input_buffer,ip,0);
		//printf("\nnot halted %s \n ",input_buffer);
		*/
		input_buffer[ip-1] = '\0';
		data = string(input_buffer);

	}while(ret);

	recent_conn_mutex.lock();	
	recent_conn.erase(node_id);
	recent_conn_mutex.unlock();
	close(conn);
	printf("Client %d exited \n",node_id);
}