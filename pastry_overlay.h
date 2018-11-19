#include "message_queue.h"
#ifndef PASTRY_OVERLAY
#define PASTRY_OVERLAY

#define BASE 4
#define LSIZE 16
#define MSIZE 16
#define MAX_ROWS 4
#define MAX_COLS 16

class Pastry_overlay {

	int total_nodes;
	
	int current_node_id;

	int leaf_set_greater[LSIZE/2];
	int leaf_set_smaller[LSIZE/2];
	int neighbour_set[MSIZE];
	int route_table[MAX_ROWS][MAX_COLS];
	

	int get_next_route(int key);
	int repair_node(int key);
	int longest_prefix(int key);
	int iniitialize_table(int ip,int port);
	int quit();

	public:

	void init();
	void recv_api_thread();
	void recv_socket_thread();


};

#endif

