
#include<cstdio>
#include<stdlib.h>
#include"socket_layer.h"
#include"pastry_overlay.h"
#include<iostream>

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
    int tot,key;
    cin >> key;
    po.init(key);

    cin >> tot;
    while(tot--) {
        cin >> key;
        po.add_to_table(key);
    }

    po.display_table();

    int test_route;
    cin >> test_route;
    print_in_hex(test_route,MAX_ROWS);

    print_in_hex(po.get_next_route(test_route),MAX_ROWS);
}