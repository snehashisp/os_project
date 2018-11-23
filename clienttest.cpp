#include<cstdio>
#include<stdlib.h>
#include"socket_layer.h"
#include"pastry_api.h"
#include "message_queue.h"
#include<iostream>

using namespace std;
int main(int argc,char *argv[]) {
    Pastry_api api;
    api.init();


    message *msg=new message();
    msg->type=PUT;
    msg->data="10#anuj";

    message *msg2=new message();
    msg2->type=GET;
    msg2->data="123#10";

    while(!pastry_overlay_api_in.add_to_queue(msg));
    while(!pastry_overlay_api_in.add_to_queue(msg2));

    cout<<"adsdndsd";
    while(1){
        // cout<<"hello";
        pastry_api_overlay_in.printQueue();
    }


    // api.recv_user_thread();
    // int port = atoi(argv[1]);
    // Socket_layer sl;
    // sl.init(port,port);
    // int nodeid,port2;
    // cin >> nodeid >> port2;
    // sl.add_ip_port(nodeid,"0.0.0.0",port2);
    // sl.send_data(nodeid,"1#data");

    // string data;
    // while(1) {

    //     cin>>nodeid >> data;
    //     sl.send_data(nodeid,data);
    // }
}
