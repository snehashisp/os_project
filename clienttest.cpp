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
    api.recv_user_thread();
}

