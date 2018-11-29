#include<queue>
#include<string>
#include<mutex>

#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE


//Types of messages

enum msg_type {EXIT, SEND_TABLE, RECV_TABLE, ADD_NODE, INIT, INIT_FINAL,
	PUT, GET, LSET, NSET, QUIT, SHUTDOWN, RESPONSE, RE_REPLICATE, REPLICATE, REPAIR,FIND};


struct message {

	msg_type type;
	std::string data;

};

//Message queues are non blocking queues to store message transfers between different layers
class Message_queue {

	std::mutex lock;
	std::queue<message *> m_queue;
	public:

	//simple message queue structure with locks for the queue
	int add_to_queue(message *m);
	message *get_from_queue();

	//Take care before calling this because it pop out the elements from the queue
	void printQueue();

};


extern Message_queue pastry_api_overlay_in, pastry_socket_overlay_in;
extern Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
//extern Message_queue pastry_socket_overlay_in;

message *extract_message(std::string data);

#endif