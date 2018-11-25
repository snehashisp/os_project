
main : message_queue.o socket_layer.o pastry_overlay.o test.cpp clienttest.cpp 
	g++ -g -o client clienttest.cpp socket_layer.o pastry_overlay.o message_queue.o -pthread -lssl -lcrypto

