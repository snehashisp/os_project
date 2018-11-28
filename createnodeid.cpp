#include <fstream>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include <math.h>

char host[NI_MAXHOST];
int port = 8080;
void fetch_myIp_address()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                           host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if(!(strcmp(ifa->ifa_name,"en0")))
            {
                break;
            }
        }
    }
}

void create()
{
    char ip_port[20];
    char myport[5];
    unsigned char key[21];

    fetch_myIp_address();
    memset(ip_port,0,sizeof(20));
    int len = snprintf(myport,5,"%d\n",port);
    strcat(ip_port,host);
    strcat(ip_port,myport);
    printf("%s length=%d\n", ip_port,strlen(ip_port));

    SHA1((unsigned char *)ip_port,strlen(ip_port),key);
    for (int i = 0; i < 20; i++) {
               printf("%02x", key[i]);
        //snprintf(f_id+2*i,4,"%02x",key[i]);
        }
        printf("\n");
}

int main(int argc,const char * argv[])
{

    if(strcmp(argv[0],"create"))
    {
        create();
    }
}