#include "pastry_overlay.h"
#include <cmath>
#include <climits>
#include <cstring>

using namespace std;
void swap(int &a,int &b) {
	int temp = a;
	a = b;
	b = temp;
}

void print_in_hex(int key,int width) {

	char format[10];
	sprintf(format,"%%0%dX ",width);
	printf(format,key);
}

int Pastry_overlay :: get_hex_at_pos(int key,int pos) {

	char hex[20],format[10];
	sprintf(format,"%%0%dX",max_rows);
	sprintf(hex,format,key);
	if (hex[pos] <= '9' && hex[pos] >= '0') return hex[pos] - '0';
	else if(hex[pos] >= 'A' && hex[pos] <= 'F') return 10 + hex[pos] - 'A';
}


void Pastry_overlay :: init(int current_node_id,int l_size,int m_size,int max_rows,int max_cols) {

	this->current_node_id = current_node_id;
	this->l_size = l_size;
	this->m_size = m_size;
	this->max_rows = max_rows;
	this->max_cols = max_cols;
	
	leaf_set = new int[l_size];
	for(int i = 0; i < l_size/2; i++) 
		leaf_set[i] = 0;
	for(int i = l_size/2; i < l_size; i++)
		leaf_set[i] = INT_MAX;

	neighbour_set = new int[m_size]{INT_MAX};
	for(int i = 0; i < l_size; i++) neighbour_set[i] = INT_MAX;

	route_table = new int*[max_rows];
	for(int i = 0; i < max_rows; i++) {
		route_table[i] = new int[max_cols];
		for(int j=0; j < max_cols; j++) route_table[i][j] = INT_MAX;
	}
	
	thread *recv_socket_thread = new thread(&Pastry_overlay::recv_socket_thread,this);
}

int Pastry_overlay :: longest_prefix(int key) {

	char cur_node[10];
	char match_key[10];
	char format[10];

	sprintf(format,"%%0%dX",max_rows);
	sprintf(cur_node,format,current_node_id);
	sprintf(match_key,format,key);
	int i=0;
	while(cur_node[i] != '\0' && cur_node[i] == match_key[i]) i++;
	return i;

}


void Pastry_overlay :: add_to_table(int key) {

	printf("\n");
	print_in_hex(key,max_rows);
	if(key > current_node_id && key < leaf_set[l_size -1]) {
		for(int i = l_size/2; i < l_size; i++) 
			if (leaf_set[i] > key) {
				route_mutex.lock();
				swap(leaf_set[i],key);
				route_mutex.unlock();
			}
	}
	else if(key < current_node_id && key > leaf_set[l_size/2-1]) {
		for(int i = 0; i < l_size/2; i++)
			if (leaf_set[i] < key) {
				route_mutex.lock();
				swap(leaf_set[i],key);
				route_mutex.unlock();
			}
	}
	if (key != 0 || key != INT_MAX) {

		int shl = longest_prefix(key);
		int pos = get_hex_at_pos(key,shl);
		route_mutex.lock();
		if((int)abs(key - current_node_id) < (int)abs(route_table[shl][pos] - current_node_id)) route_table[shl][pos] = key;
		route_mutex.unlock();
	}
}

void Pastry_overlay :: remove_from_table(int key) {

	if(key > current_node_id && key < leaf_set[l_size -1]) {
		for(int i = l_size/2; i < l_size; i++) 
			if (leaf_set[i] == key) {
				route_mutex.lock();
				leaf_set[i] = INT_MAX;
				sort(leaf_set + l_size/2,leaf_set + l_size);
				route_mutex.unlock();
			}
	}
	else if(key < current_node_id && key > leaf_set[l_size/2 -1]) {
		for(int i = 0; i < l_size/2; i++)
			if (leaf_set[i] == key) {
				route_mutex.lock();
				leaf_set[i] = 0;
				sort(leaf_set,leaf_set + l_size/2,greater<int>());
				route_mutex.unlock();
			}
	}
	else {

		int shl = longest_prefix(key);
		int pos = get_hex_at_pos(key,shl);
		route_mutex.lock();
		if(route_table[shl][pos] == key) route_table[shl][pos] = INT_MAX;
		route_mutex.unlock();
	}

}

int Pastry_overlay :: get_next_route(int key) {

	if(key <= leaf_set[l_size-1] && key >=leaf_set[l_size/2-1]) {

		int min_node = (int)abs(key - current_node_id),dest = current_node_id;

		route_mutex.lock();
		for(int i = 0; i < l_size; i++) {
			if (min_node > ((int)abs(key - leaf_set[i]))) {
				min_node = (int)abs(key - leaf_set[i]);
				dest = leaf_set[i];
			}
		}
		route_mutex.unlock();
		return dest;
	}
	else {

		int shl = longest_prefix(key);
		int pos = get_hex_at_pos(key,shl);

		route_mutex.lock();
		if( route_table[shl][pos] != INT_MAX) {
			route_mutex.unlock();
			return route_table[shl][pos];
		}
		else {

			int min_node = (int)abs(key - current_node_id),dest;

			for(int i = 0; i < l_size; i++) {
				if (min_node > ((int)abs(key - leaf_set[i]))) {
					min_node = (int)abs(key - leaf_set[i]);
					dest = leaf_set[i];
				}
		 	}
		 	for(int i = 0; i < max_rows; i++) {
		 		for(int j = 0; j < max_cols; j++) {
		 			if (min_node > ((int)abs(key - route_table[i][j]))) {
		 				min_node = (int)abs(key - route_table[i][j]);
		 				dest = route_table[i][j];
		 			}
		 		}
		 	}
		 	for(int i = 0; i < m_size; i++) {
		 		if (min_node > ((int)abs(key - neighbour_set[i]))) {
					min_node = (int)abs(key - neighbour_set[i]);
					dest = neighbour_set[i];
		 		}
		 	}
		 	route_mutex.unlock();
		 	return dest;
		}
	}
}

void Pastry_overlay :: recv_socket_thread() {

	while(1) {

		message *mess;
		while((mess = get_from_queue()) == NULL) {

			if (mess -> type == SEND_ROW) {

			}
			else if(mess -> type == ROUTE) {

				int key;
				sscanf(mess->data.c_str(),"%d#",&key);
				key = key >> 16;
				string new_message = 

			}
			else if(mess -> type == RECV_ROW) {

			}
			else if(mess -> type == SEND_TABLE) {

			}
			else if(mess -> type == RECV_TABLE) {

			}
			else if(mess -> type == REROUTE) {


			}
		}
	}
}


void Pastry_overlay :: display_table() {

	char format[10];
	sprintf(format,"%%0%dX ",max_rows);

	printf("\nCurrent Node ");
	printf(format,current_node_id);
	printf("\n");

	printf("Leaf Set\n");
	for(int i = 0; i < l_size; i++) {
		if(leaf_set[i] != INT_MAX || leaf_set != 0)
			printf(format,leaf_set[i]);
		else printf(format,0);
 	}
 	printf("\nRoute Table\n");
 	for(int i = 0; i < max_rows; i++) {
 		for(int j = 0; j < max_cols; j++) {
 			if(route_table[i][j] != INT_MAX)
 				printf(format,route_table[i][j]);
 			else printf(format,0);
 		}
 		printf("\n");
 	}
 	printf("\nNeighbourhood Table\n");
 	for(int i = 0; i < m_size; i++) {
 		if(neighbour_set[i] != INT_MAX)
 			printf(format,neighbour_set[i]);
 		else printf(format,0);
 	}
 	printf("\n");
}