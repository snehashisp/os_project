#include<cstdio>
#include<stdlib.h>
#include"socket_layer.h"
#include"pastry_api.h"
#include "message_queue.h"
#include"pastry_overlay.h"
#include<iostream>

extern Message_queue pastry_api_overlay_in, pastry_api_user_in;
extern Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
extern Message_queue pastry_socket_overlay_in;

using namespace std;

int main(int argc,char *argv[]) {
    Pastry_api api;
    api.init();


    // message *msg=new message();
    // msg->type=PUT;
    // msg->data="10#anuj";


    // message *msg2=new message();
    // msg2->type=GET;
    // msg2->data="123#10";

    // while(!pastry_overlay_api_in.add_to_queue(msg));
    // while(!pastry_overlay_api_in.add_to_queue(msg2));
    //     cin>>nodeid >> data;
    //     sl.send_data(nodeid,data);
    // }
    while(1)
        {
            api.recv_user_thread();
        }
}

/*
int main(int argc,char *argv[]) {


    Pastry_overlay po;
    Socket_layer so;

    int nodeid = atoi(argv[1]);
    int port = atoi(argv[3]);
    string ip = string(argv[2]);
    //node id ip port
    so.init(nodeid,string(ip.c_str()),port);
    po.init(nodeid,&so);

    int iport;
    cin >> nodeid >> iport;
    po.initialize_table(nodeid,ip,iport);


    while(1);
}
*/    /*
    cin >> nodeid >> port2;
    so.add_ip_port(nodeid,"0.0.0.0",port2);
    so.send_data(nodeid,"1#data");

    message *mem = new message();
    mem -> type = PUT;
    string data;
    cin >> data;
    mem -> data = data;
    while(!pastry_api_overlay_in.add_to_queue(mem));
    while(1);
<<<<<<< HEAD
    */

