#include <map>
#ifndef SOCKET_LAYER
#define SOCKET_LAYER
#define BUFFER_SIZE 1024

#include "message_queue.h"

class Socket_layer {

	//incoming sockets
	int incoming_socket;
	//total number of nodes in pastry net 
	int total_nodes;
	//ip and port of all nodes in the pastry net as a map with node id as key
	std::map<int,int> node_ip_list;
	std::map<int,int> node_port_list;

	//cache holding recently established connections
	std::map<int,int> recent_conn;

	//example functions to be called from receiving thread/sending thread
	void add_ip_port(int node_id,int ip,int port);
	void remove_ip_port(int node_id);

	public:

	void init();
	void recv_overlay_thread();
	void recv_socket_thread();

};

#endif	