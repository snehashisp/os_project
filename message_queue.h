#include<queue>
#include<string>
#include<mutex>

#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

//Types of messages
enum msg_type {};

struct message {

	msg_type type;
	std::string data;

};

class message_queue {

	std::mutex lock;
	std::queue<message *> m_queue;

	public:

	//simple message queue structure with locks for the queue
	void add_to_queue(message *m);
	message *get_from_queue();

};

message_queue pastry_api_in,pastry_api_out;
message_queue pastry_overlay_in,pastry_overlay_out;


#endif