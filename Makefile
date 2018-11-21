main : message_queue.o socket_layer.o test.cpp clienttest.cpp
	g++ -g -o main test.cpp socket_layer.o message_queue.o -pthread	
	g++ -g -o client clienttest.cpp socket_layer.o message_queue.o -pthread
