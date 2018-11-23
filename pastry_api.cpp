#include "pastry_api.h"	
#define print(A) cout<<A<<endl;

vector<string> parse(string s, char delim){
	vector <string> tokens; 
    stringstream ss(s); 
    string word; 
    while(getline(ss, word, delim))
        tokens.push_back(word); 
    return tokens;
}


void addToQueue(enum msg_type mType, string data){
	message *msg=new message();
	msg->type=mType;
	msg->data=data;
	pastry_api_overlay_in.add_to_queue(msg);
}

long createNode(int port,char host[]){
	char ip_port[20];
    char myport[5];
	char temp[41];
    unsigned char key[21];
    fetch_myIp_address(host);
    memset(ip_port,0,sizeof(20));
    int len = snprintf(myport,5,"%d\n",port);
    string mysha;
    strcat(ip_port,host);
    strcat(ip_port,myport);

    // printf("%s length=%d\n", ip_port,strlen(ip_port));
    SHA1((unsigned char *)ip_port,strlen(ip_port),key);
    for (int i = 0; i < 20; i++){
    	// printf("%02x", key[i]);
		snprintf(temp+i*2,4,"%02x", key[i]);
    }
   	// cout<<endl;
    mysha=temp;
    // cout<<stol(mysha.substr(0,4), nullptr, 16)<<"\n"; 
    return stol(mysha.substr(0,4), nullptr, 16);
}

void fetch_myIp_address(char host[NI_MAXHOST]){
    struct ifaddrs *ifaddr, *ifa;
    int family,s;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if(!(strcmp(ifa->ifa_name,"e")))
                break;
        }
    }
}


void Pastry_api:: recv_user_thread(){
	string s;
	while(1){
		getline(cin,s);
		vector<string> cli=parse(s,' ');
		int totalWords=cli.size();
		if(totalWords>0){
			string opcode=cli[0];
			if(opcode=="port"){
				print("port code");
				if(totalWords>1)
					// addToQueue(PORT,cli[1]);
					port=atoi(cli[1].c_str());
					if(!port)
						print("please provide valid port");
			}
			else if(opcode=="create"){
				print("create code");
				addToQueue(CREATE,"");
				if(port){
					nodeId=createNode(port,host);
					cout<<nodeId;
					sockets.init(nodeId,port);
				}
			}
			else if(opcode=="join"){
				print("join code");
				if(totalWords>2)
					addToQueue(JOIN,cli[1]+"#"+cli[2]);
				pastry_api_overlay_in.printQueue();
			}
			else if(opcode=="put"){
				print("put code");
				if(totalWords>2)
					addToQueue(PUT,cli[1]+"#"+cli[2]);
			}
			else if(opcode=="get"){
				print("get code");
				if(totalWords>1)
					addToQueue(GET,cli[1]);
			}
			else if(opcode=="lset"){
				print("lset code");
				addToQueue(LSET,"");
			}
			else if(opcode=="nset"){
				print("nset code");
				addToQueue(NSET,"");
			}
			else if(opcode=="routetable"){
				print("routetable code");
				addToQueue(ROUTETABLE,"");	
			}
			else if(opcode=="quit"){
				print("quit code");
			}
			else if(opcode=="shutdown"){
				print("shutdown code");
			}
			else{
				print("Invalid command");
			}
		}
		else
			print("Please provide some command")

	}
}
