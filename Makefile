main : message_queue.o socket_layer.o clienttest.cpp
	g++ -g -o client clienttest.cpp socket_layer.o message_queue.o -pthread
