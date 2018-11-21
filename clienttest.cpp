
#include<cstdio>
#include<stdlib.h>
#include"socket_layer.h"
#include<iostream>

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



/*
int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    string hello = "999#192.0.0.1#" + string(argv[1]) + "\n"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = atoi(argv[1]); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    write(sock , hello.c_str() , hello.size()); 
    

    //valread = read( sock , buffer, 1024); 
    //printf("%s\n",buffer ); 
    char p[200];
    while(1) {
        scanf("%s",p);
        hello = string(p);
        write(sock,hello.c_str(),hello.size());

    }
    return 0; 
} 
*/