#include <map>
#include <thread>
#include "message_queue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <cstdio>
#include <arpa/inet.h>
#include "message_queue.h"

#ifndef SOCKET_LAYER
#define SOCKET_LAYER
#define BUFFER_SIZE 1024
#define DEFAULT_QUEUE 10


std::string extract_socket_info(std::string data,int *node_id,std::string *ip,int *port);


class Socket_layer {

	std::thread *incoming_thread;
	std::thread *overlay_thread;
	//incoming sockets
	int incoming_socket;
	struct sockaddr_in address;
	//total number of nodes in pastry net 
	int total_nodes;


	std::mutex socket_mutex;
	//ip and port of all nodes in the pastry net as a map with node id as key
	std::map<int,std::string> ip_list;
	std::map<int,int> port_list;

	//cache holding recently established connections
	std::mutex recent_conn_mutex;
	std::map<int,int> recent_conn;

	public:

	int cur_node_id;
	int cur_port;
	std::string cur_ip;

	void add_ip_port(int node_id,std::string ip,int port);
	void remove_ip_port(int node_id);
	int  send_data(int node_id,std::string message);

	//inititalize 
	int init(int cur_node_id,std::string ip,int port);

	void recv_overlay();
	void recv_node(int conn,int node_id,std::string data);
	void incoming_conn();


};

#endif