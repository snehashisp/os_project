#include "pastry_overlay.h"
#include <cmath>
#include <climits>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;
void swap(int &a,int &b) {
	int temp = a;
	a = b;
	b = temp;
}


extern Message_queue pastry_api_overlay_in, pastry_api_user_in;
extern Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
extern Message_queue pastry_socket_overlay_in;

int get3min(int lset[],int n,int **min_arr) {

	int *ret = new int[3];
	int k = 0,i = 0,j = n/2;
	while(i < n/2 && j < n && k < 3) {
		if (lset[i] == 0 && lset[j] == INT_MAX) 
			break;
		else if (lset[j] == INT_MAX || lset[i] < lset[j]) {
			ret[k] = lset[i];
			k++; 

		}
		else if (lset[i] == 0 || lset[j] < lset[i]) {
			ret[k] = lset[j];
			k++; j++; 
		}
	}
	*min_arr = ret;
	return k;
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


void Pastry_overlay :: init(int current_node_id,Socket_layer *sl,int l_size,int m_size,int max_rows,int max_cols) {

	this->current_node_id = current_node_id;
	this->l_size = l_size;
	this->m_size = m_size;
	this->max_rows = max_rows;
	this->max_cols = max_cols;
	this->sock_layer = sl;
	
	leaf_set = new key_type[l_size];
	for(int i = 0; i < l_size/2; i++) 
		leaf_set[i] = 0;
	for(int i = l_size/2; i < l_size; i++)
		leaf_set[i] = INT_MAX;

	neighbour_set = new key_type[m_size]{INT_MAX};
	for(int i = 0; i < l_size; i++) neighbour_set[i] = INT_MAX;

	route_table = new key_type*[max_rows];
	for(int i = 0; i < max_rows; i++) {
		route_table[i] = new key_type[max_cols];
		for(int j=0; j < max_cols; j++) route_table[i][j] = INT_MAX;
	}
	
	thread *recv_socket_thread = new thread(&Pastry_overlay::recv_socket_thread,this);
	thread *recv_overlay_thread = new thread(&Pastry_overlay::recv_api_thread,this);
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

	if(key > current_node_id && key < leaf_set[l_size -1]) {
		for(int i = l_size/2; i < l_size; i++) {
			if (leaf_set[i] == key) return;
			if (leaf_set[i] > key) {
				route_mutex.lock();
				swap(leaf_set[i],key);
				route_mutex.unlock();
			}
		}
	}
	else if(key < current_node_id && key > leaf_set[l_size/2-1]) {
		for(int i = 0; i < l_size/2; i++) {
			if (leaf_set[i] == key) return;
			if (leaf_set[i] < key) {
				route_mutex.lock();
				swap(leaf_set[i],key);
				route_mutex.unlock();
			}
		}
	}
	if (key != current_node_id && (key != 0 || key != INT_MAX)) {

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

key_type Pastry_overlay :: get_next_route(int key) {

	if(key <= leaf_set[l_size-1] && key >=leaf_set[l_size/2-1]) {

		int min_node = (int)abs(key - current_node_id),dest = current_node_id;

		route_mutex.lock();
		for(int i = 0; i < l_size; i++) {
			if(leaf_set[i] == 0 || leaf_set[i] == INT_MAX) continue;
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

void Pastry_overlay :: recv_api_thread() {

	while(1) {

		message *mess;
		while((mess = pastry_api_overlay_in.get_from_queue()) != NULL) {
			//cout<<"Inside recv_api_thread"<<endl;
			//pastry_api_overlay_in.printQueue();
			if(mess -> type == PUT || mess -> type == GET) {
				//printf("fsdsfdsf");
				route(mess);
				delete(mess);
			}
			/*
			else if(mess -> type == REPLICATE) {

				printf("replicate message received %s\n",mess -> data);
				int *ret;
				int k = get3min(leaf_set,l_size,&ret);
				printf("Return k %d\n",k);
				string data = to_string((int)REPLICATE) + string("#") + mess -> data;
				if(k >= 2) {
					printf("Sending replicate to %d \n",ret[0]);
					sock_layer -> send_data(ret[0],data);
					printf("Sending replicate to %d \n",ret[1]);
					sock_layer -> send_data(ret[1],data);
				}
				else if(k == 1) {
					printf("Sending replicate to %d \n",ret[0]);
					sock_layer -> send_data(ret[0],data);
				}

			}
			else if(mess -> type == RE_REPLICATE) {	

				int *ret;
				int k = get3min(leaf_set,l_size,&ret);
				string data = to_string((int)REPLICATE) + string("#") + mess -> data;
				sock_layer -> send_data(ret[k-1],data);

			}
			*/
			else if(mess -> type == RESPONSE) {
				printf(" RESPONSE %s \n",mess -> data.c_str());
				int nodeid,port;
				string ip;
				string dummy;
				sscanf(mess -> data.c_str(),"%d#%[^#]#%d#%s",&nodeid,ip.c_str(),&port,dummy.c_str());
				sock_layer -> add_ip_port(nodeid,ip,port);
				add_to_table(nodeid);
				printf(" RESPONSE %s \n",mess -> data.c_str());
				sock_layer -> send_data(nodeid,to_string((int)RESPONSE) + string("#") + mess -> data);
			}
		}
	}
}

void Pastry_overlay :: repair(message *mess) {

}

void Pastry_overlay :: route(message *mess) {

	int key;
	sscanf(mess->data.c_str(),"%d#",&key);
	key = key & ((1 << 16) - 1);
	int next_node = get_next_route(key);
	if(next_node == current_node_id) {
		if(mess -> type == INIT) {
			string msg = to_string((int)INIT_FINAL) + "#";
			sock_layer -> send_data(key,msg);
			//delete(mess);
		}
		else
		{
			//cout<<mess->data<<endl;
			while(!pastry_overlay_api_in.add_to_queue(mess));
		}
		//printf("\nMessage received %s\n",mess->data.c_str());
	}
	else {

		string new_message = to_string((int)mess->type) + string("#") + mess -> data.c_str();
		printf("\nRerouted message %d %s\n",next_node,new_message.c_str());
		//printf("\nRerouted to %d \n",next_node);
		if(!sock_layer -> send_data(next_node,new_message)) {
			remove_from_table(next_node);
		}
	}
}

message *Pastry_overlay :: get_table_message() {

	message *mess = new message();
	string temp;
	mess -> type = RECV_TABLE;
	string sendstr = to_string(current_node_id) + string("#") + sock_layer -> cur_ip + string("#") + to_string(sock_layer -> cur_port) + "#";
	for(int i = 0; i < l_size; i++) {
		temp = sock_layer -> get_ip_port(leaf_set[i]);
		sendstr = sendstr + to_string(leaf_set[i]) + "#" + temp + "#";
	}
	for(int i = 0; i < max_rows; i++) {
		for(int j = 0; j < max_cols; j++) {
			temp = sock_layer -> get_ip_port(route_table[i][j]);
			sendstr = sendstr + to_string(route_table[i][j]) + "#" + temp + "#";
		}
	}
	for(int i = 0; i < m_size; i++) {
		temp = sock_layer -> get_ip_port(neighbour_set[i]);
		sendstr = sendstr + to_string(neighbour_set[i]) + "#" + temp + "#";
	}
	//printf("Sending table format %s\n",sendstr.c_str());
	mess -> data = sendstr;
	return mess;
}

void Pastry_overlay :: update_table_message(message *mess) {

	vector <string> tokens; 
    stringstream ss(mess -> data); 
    string word; 
    while(getline(ss, word, '#'))
        tokens.push_back(word); 
    add_to_table(atoi(tokens[0].c_str()));
    //cout << tokens[1];
    sock_layer -> add_ip_port(atoi(tokens[0].c_str()),string(tokens[1].c_str()),atoi(tokens[2].c_str()));
    //printf("received data %s\n",mess -> data.c_str());

    //printf("leaf set");
    int tp = 3;
    for(int i = 0; i < l_size; i++) {
    	//cout << tokens[tp++] << " ";
    	int id = atoi(tokens[tp++].c_str());
    	if(id != 0 && id != INT_MAX) {
    		add_to_table(id);
    		string ip = tokens[tp++];
    		int port = atoi(tokens[tp++].c_str());
    		sock_layer -> add_ip_port(id,string(ip.c_str()),port);
    	}
    }
    for(int i = 0; i < max_rows; i++) {
    	for(int j = 0; j < max_cols; j++) {
    		//cout << tokens[tp++] << " ";
	    	int id = atoi(tokens[tp++].c_str());
	    	if(id != 0 && id != INT_MAX) {
	    		add_to_table(id);
	    		string ip = tokens[tp++];
	    		int port = atoi(tokens[tp++].c_str());
	    		sock_layer -> add_ip_port(id,string(ip.c_str()),port);
	    	}
    	}
    	//cout << endl;
    }
    printf("Updated table \n");
    display_table();
}

void Pastry_overlay :: recv_socket_thread() {

	printf("Socket overlay thread called ");
	while(1) {
		//printf("fsdfdsf\n");

		message *mess;
		while((mess = pastry_socket_overlay_in.get_from_queue()) != NULL) {

			//printf("%d \n", mess -> type);

			if(mess -> type == PUT || mess -> type == GET) {
				route(mess);
				//delete(mess);
			}
			else if(mess -> type == ADD_NODE) {

				int nodeid;
				sscanf(mess->data.c_str(),"%d",&nodeid);
				add_to_table(nodeid);
				printf("Added %d to table\n",nodeid);
				display_table();
				delete(mess);

			}
			else if(mess -> type == SEND_TABLE || mess -> type == INIT || mess -> type == REPAIR) {

				if(check_init == false) {
					while(!pastry_socket_overlay_in.add_to_queue(mess));
					continue;
				}

				string ip;
				int nid, port;
				sscanf(mess -> data.c_str(),"%d#%[^#]#%d",&nid,ip.c_str(),&port);
				sock_layer -> add_ip_port(nid,string(ip.c_str()),port);
				message *ret_mess = get_table_message();
				string msg = to_string((int)ret_mess -> type) + string("#") + ret_mess -> data;
				//printf("\nSending %s\n",ret_mess -> data.c_str());
				sock_layer -> send_data(nid,msg);
				delete(ret_mess);
				if (mess -> type == INIT) {
					msg = to_string(SEND_TABLE) + string("#") + to_string(current_node_id) + string("#") 
						+ sock_layer ->  cur_ip + string("#") + to_string(sock_layer -> cur_port);
					sock_layer -> send_data(nid,msg);
					route(mess);
				}
				delete(mess);
			}
			else if(mess -> type == RECV_TABLE) {
				update_table_message(mess);
				delete(mess);
			}
			else if(mess -> type == INIT_FINAL) {
				printf("Final init arrived\n");
				queue<message *> mess_queue;
				while(mess = pastry_socket_overlay_in.get_from_queue()) {
					if(mess -> type != RECV_TABLE) mess_queue.push(mess);
					else update_table_message(mess);
				}
				while(!mess_queue.empty()) {
					while(!pastry_socket_overlay_in.add_to_queue(mess_queue.front()));
					mess_queue.pop();

				}
				check_init = true;
				//sock_layer -> remove_ip_port(0);
				printf("Node initialization done\n");
			}
			else if(mess -> type == RESPONSE || mess -> type == REPLICATE) {
				while(!pastry_overlay_api_in.add_to_queue(mess));
			}
		}

	}
	printf("Socket overlay exited\n");
}

void Pastry_overlay :: initialize_table(int nodeid,std::string ip,int port) {

	
	if(ip == sock_layer -> cur_ip && port == sock_layer -> cur_port) {
		printf("Node self initialized\n");
		check_init = true;
		return;
	}
	sock_layer -> add_ip_port(nodeid,ip,port);
	string msg = to_string(INIT) + string("#") + to_string(current_node_id) + string("#") 
		+ sock_layer ->  cur_ip + string("#") + to_string(sock_layer -> cur_port);
	if(!sock_layer -> send_data(nodeid,msg)) {
		printf("Failed to contact initialization node\n");
	}
	else printf("initialization started\n");
}

void Pastry_overlay :: display_table() {

	char format[10];
	sprintf(format,"%%0%dX ",max_rows);

	vector<int> nodes;
	printf("\nCurrent Node ");
	printf(format,current_node_id);
	printf("\n");
	
	printf("Leaf Set\n");
	for(int i = 0; i < l_size; i++) {
		if(leaf_set[i] != INT_MAX && leaf_set[i] != 0) {
			printf(format,leaf_set[i]);
			nodes.push_back(leaf_set[i]);
		}
		else printf(format,0);
 	}
 	printf("\nRoute Table\n");
 	for(int i = 0; i < max_rows; i++) {
 		for(int j = 0; j < max_cols; j++) {
 			if(route_table[i][j] != INT_MAX){
 				printf(format,route_table[i][j]);
 				nodes.push_back(route_table[i][j]);
 			}
 			else printf(format,0);
 		}
 		printf("\n");
 	}
 	printf("\nNeighbourhood Table\n");
 	for(int i = 0; i < m_size; i++) {
 		if(neighbour_set[i] != INT_MAX){
 			printf(format,neighbour_set[i]);
 			nodes.push_back(neighbour_set[i]);
 		}
 		else printf(format,0);
 	}
 	printf("\n");

 	printf("Nodes and corresponding ip port mappings \n");
 	for(auto i = nodes.begin(); i != nodes.end(); i++) {
 		string str = sock_layer -> get_ip_port(*i);
 		printf("Node id %d: ip and port (ip # port) : %s\n", *i,str.c_str());
 	}

}