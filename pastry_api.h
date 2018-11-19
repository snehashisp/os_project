#ifndef PASTRY_API
#define PASTRY_API
#include "pastry_overlay.h"
#include "socket_layer.h"
#include "message_queue.h"

class Pastry_api {

	Pastry_overlay overlay;
	Socket_layer sockets;

	std::map<int,std::string> dht;

	std::string *look_up(int key);
	void add_key_value_pair(int key,std::string value);
	void delete_key_value_pair(int key);

	public:

	void recv_overlay_thread();
	void recv_user_thread();



};

#endif