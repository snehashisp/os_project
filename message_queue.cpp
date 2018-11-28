#include "message_queue.h"

using namespace std;

Message_queue pastry_api_overlay_in, pastry_socket_overlay_in;
Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
//Message_queue pastry_socket_overlay_in;

void Message_queue :: printQueue(){

	while(!m_queue.empty()){
		message *m=m_queue.front();
		printf("msgtype=%d data=%s\n",m->type,m->data.c_str() );
		m_queue.pop();
	}
}

int Message_queue ::  add_to_queue(message *m) {

	if(lock.try_lock()) {
		m_queue.push(m);
		lock.unlock();
		return 1;
	}
	return 0;

}

message *Message_queue :: get_from_queue() {

	if(lock.try_lock()) {
		if(m_queue.empty()){
			lock.unlock();
			return NULL;
		}
		message *ret = m_queue.front();
		m_queue.pop();
		lock.unlock();
		return ret;
	}
	return NULL;
}

message *extract_message(string data) {

	message *new_message = new message;
	char mess[3096];
	sscanf(data.c_str(),"%d#%s",&(new_message -> type),mess);
	new_message -> data = string(mess);
	return new_message;
};