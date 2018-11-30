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
    int close=1;

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
            close = api.recv_user_thread();
            if(close == 0 && pastry_api_overlay_in.get_size_queue()==0 && pastry_socket_overlay_in.get_size_queue()==0 && pastry_overlay_socket_in.get_size_queue()==0 && pastry_overlay_api_in.get_size_queue()==0)
                break;
        }
}

