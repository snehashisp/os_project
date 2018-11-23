#ifndef PASTRY_API
#define PASTRY_API
#include<iostream>
#include<string>
#include <sstream>
#include "pastry_overlay.h"
#include "socket_layer.h"
#include "message_queue.h"
using namespace std;


vector<string> parse(string s, char delim);
void addToQueue(enum msg_type mType, string data);
class Pastry_api {

	Pastry_overlay overlay;
	Socket_layer sockets;

	std::map<int,std::string> dht;

	std::string *look_up(int key);
	void add_key_value_pair(int key,std::string value);
	void delete_key_value_pair(int key);

	public:

	void init();
	void recv_overlay_thread();
	void recv_user_thread();



};

#endif