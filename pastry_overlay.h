#include "message_queue.h"
#include "socket_layer.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <algorithm>

typedef int key_type;

#ifndef PASTRY_OVERLAY
#define PASTRY_OVERLAY

#define IDLEN 4
#define LSIZE 16
#define MSIZE 16
#define MAX_ROWS 4
#define MAX_COLS 16

void print_in_hex(int key,int width);

class Pastry_overlay {

	int total_nodes;
	Socket_layer *sock_layer;

	int current_node_id;

	std::mutex route_mutex;
	key_type *leaf_set;
	int l_size;
	key_type *neighbour_set;
	int m_size;
	key_type **route_table;
	int max_rows,max_cols;

	public:
	int longest_prefix(int key);
	int get_hex_at_pos(int key,int pos);
	void add_to_table(int key);
	void remove_from_table(int key);
	std::string get_row(int i);
	std::string get_leaf();
	void route(message *mess);
	//public:

	key_type get_next_route(int key);
	int repair_node(int key);
	int iniitialize_table(int ip,int port);
	void display_table();
	int quit();

	void init(int current_node_id,Socket_layer *sl,int l_size = LSIZE,int m_size = MSIZE,int max_rows = MAX_ROWS,int max_cols = MAX_COLS);
	void recv_api_thread();
	void recv_socket_thread();


};

#endif

