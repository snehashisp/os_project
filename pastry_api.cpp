#include "pastry_api.h"	
#define print(A) cout<<A<<endl;


void Pastry_api :: init(){
	recvOverlayThread = new thread(&Pastry_api :: recv_overlay_thread,this);
}

void Pastry_api :: printDHT(){
	cout<<"KEY\t"<<"VALUE"<<endl;
	for (auto i =dht.begin(); i != dht.end(); ++i)
		cout<<(i->first)<<"\t"<<(i->second)<<endl;
}

void Pastry_api :: replicate(int key, string value){
	add_key_value_pair(key,value);
}

void Pastry_api :: recv_overlay_thread(){
	while(1){
			message *msg=pastry_overlay_api_in.get_from_queue();
			if(msg){
				if(msg->type==PUT){
					//ADD into dht
					//data format key#value#nodeid#ip#port
					// print("hello");
					print(msg->data);
					std::vector<string> keyValue=parse(msg->data,'#');
					// printf("In PUT %s %s\n", keyValue[0].c_str(), keyValue[1].c_str() );
					add_key_value_pair(atoi(keyValue[0].c_str()),keyValue[1]);
					replicate(atoi(keyValue[0].c_str()),keyValue[1]);

					message *msg=new message();
					msg->type=RESPONSE;
					msg->data=keyValue[2]+"#"+keyValue[3]+"#"+keyValue[4]+"#success";

					while(!pastry_api_overlay_in.add_to_queue(msg));
					
					message *msg2=new message();
					msg2->type=REPLICATE;
					msg2->data=keyValue[0]+"#"+keyValue[1]+"#"+to_string(nodeId)+"#"+ip+"#"+to_string(port);

					while(!pastry_api_overlay_in.add_to_queue(msg2));

				}
				else if(msg->type==GET){
					//get value from my lookup
					//data will be key#sourcenodeid#ip#port
					std::vector<string> nodeIdKey=parse(msg->data,'#');
					// printf("In GET %s %s\n", nodeIdKey[0].c_str(), nodeIdKey[1].c_str() );
					string value=look_up(atoi(nodeIdKey[0].c_str()));
					// print(value);
					message *msg=new message();
					msg->type=RESPONSE;
					msg->data=nodeIdKey[1]+"#"+nodeIdKey[2]+"#"+nodeIdKey[3]+"#"+value+"#"+nodeIdKey[0];

					// print(msg->data);
					while(!pastry_api_overlay_in.add_to_queue(msg));

				}
				else if(msg->type==REPLICATE){
					//replicate
					//data will be key#value#nodeid#ip#port
					std::vector<string> nodeIdKey=parse(msg->data,'#');
					replicate(atoi(nodeIdKey[0].c_str()),nodeIdKey[1]);
					message *msg=new message();
					msg->type=RESPONSE;
					msg->data=nodeIdKey[2]+"#"+nodeIdKey[3]+"#"+nodeIdKey[4]+"#success";
					while(!pastry_api_overlay_in.add_to_queue(msg));
				}
				else if (msg->type== RESPONSE)
				{
					/* receive from destination */
					//data will be sourcenodeid#ip#port#value#key
					std::vector<string> list=parse(msg->data,'#');
					if(list[3]!="success"){
						print("Found Value: "+list[3]);
					}
					else{
						print("Succesully Put.");
					}

				}
			}
	}
}

void Pastry_api:: add_key_value_pair(int key, string value){
	dht[key]=value;
}

string Pastry_api ::  look_up(int key){
	if(dht.find(key)!=dht.end())
		return dht[key];
	return "NuLL";
}

vector<string> parse(string s, char delim){
	vector <string> tokens; 
    stringstream ss(s); 
    string word; 
    while(getline(ss, word, delim))
        tokens.push_back(word); 
    return tokens;
}


void cliAddToQueue(enum msg_type mType, string data){
	//add to overlay queue
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

void Pastry_api :: putOperation(string keystr,string value)
{
	int key=atoi(keystr.c_str());
	key = key & ((1 << 16) - 1);
	// cout<<key<<endl;
	if(key == nodeId){
		// cout<<"Putting here";
		add_key_value_pair(atoi(keystr.c_str()),value);
	}
	else
	{
		message *msg=new message();
		msg->type=PUT;
		msg->data=keystr + '#' + value + '#'+ to_string(nodeId)+ '#' + ip +'#' +to_string(port);
		while(!pastry_api_overlay_in.add_to_queue(msg));
	}
}

void Pastry_api :: getOperation(string keystr)
{
	int key=atoi(keystr.c_str());
	// key = key & ((1 << 16) - 1);
	//if(key == nodeId){
	
	string value=look_up(key);
	if(value!="NuLL")
		cout<<"Value Found:  "<<look_up(atoi(keystr.c_str()));
	else
	{
		message *msg =new message();
		msg->type=GET;
		msg->data=keystr + '#'+ to_string(nodeId)+ '#' + ip +'#' +to_string(port);
		while(!pastry_api_overlay_in.add_to_queue(msg));	
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
				if(totalWords>1){
					port=atoi(cli[1].c_str());
					if(!port){
						print("please provide valid port");			
					}
					else{
						print("Port "+ to_string(port) +" assigned succesfully.");					
					}
				}
			}
			else if(opcode=="create"){
				// print("create code");
				if(port){
					nodeId=createNode(port,host);
					ip=host;
					print("Node id created "+ to_string(nodeId)+ " with ip "+ ip + " on port "+ to_string(port) );
					sockets.init(nodeId,ip,port);
					overlay.init(nodeId,&sockets);
				}
			}
			else if(opcode=="join"){
				// print("join code");
				if(totalWords>2)
					overlay.initialize_table(atoi(cli[1].c_str()),cli[2],atoi(cli[3].c_str()));
			}
			else if(opcode=="put"){
				// print("put code");
				if(totalWords>2)
					putOperation(cli[1],cli[2]);
			}
			else if(opcode=="get"){
				// print("get code");
				if(totalWords>1)
					getOperation(cli[1]);
			}
			else if(opcode=="lset"){
				// print("lset code");
				overlay.display_table();
			}
			else if(opcode=="nset"){
				// print("nset code");
				overlay.display_table();
			}
			else if(opcode=="routetable"){
				// print("routetable code");
				overlay.display_table();
			}
			else if(opcode=="printDHT"){
				// print("printDHT code");
				printDHT();
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