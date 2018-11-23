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
					addToQueue(PORT,cli[1]);
			}
			else if(opcode=="create"){
				print("create code");
				addToQueue(CREATE,"");
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
