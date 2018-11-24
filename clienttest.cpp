
#include<cstdio>
#include<stdlib.h>
#include"socket_layer.h"
#include"pastry_overlay.h"
#include<iostream>

extern Message_queue pastry_api_overlay_in, pastry_api_user_in;
extern Message_queue pastry_overlay_socket_in, pastry_overlay_api_in;
extern Message_queue pastry_socket_overlay_in;


using namespace std;

/*socket layer testing
using namespace std;
int main(int argc,char *argv[]) {

    int port = atoi(argv[1]);
    Socket_layer sl;
    sl.init(port,port);
    int nodeid,port2;
    cin >> nodeid >> port2;
    sl.add_ip_port(nodeid,"0.0.0.0",port2);
    sl.send_data(nodeid,"1#data");

    string data;
    while(1) {

        cin>>nodeid >> data;
        sl.send_data(nodeid,data);
    }
}
*/

int main(int argc,char *argv[]) {


    Pastry_overlay po;
    Socket_layer so;

    int port = atoi(argv[1]);
    so.init(port,port);
    po.init(port,&so);

    int nodeid,port2;
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
}