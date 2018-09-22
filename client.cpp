#include "torrent.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <bits/stdc++.h>
#include <iostream>
#include <thread>

using namespace std;

string client_ip_port;
string client_ip;
int client_port;
string tracker1_ip_port;
string tracker2_ip_port;
string tracker1_ip;
int tracker1_port;
string tracker2_ip;
int tracker2_port;

string log_file;

vector<string> split(const string& s, char delimiter){
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter)){
      tokens.push_back(token);
   }
   return tokens;
}

void start_client_server(string client_ip, int client_port){
    //to be implemented
}

void send_message(int sockfd, string message){
    if(send(sockfd,message.c_str(),message.size()+1,0)<0){
       
    }
}

void connect_to_seeders(string clien_ip, int port, string file_to_download_hash){

    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(clien_ip.c_str());
    address.sin_port = port;
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);
  
    cout<<result;
    if(result == -1) {
        cout<<"Could not connect";
        exit(1);
    }
    send_message(sockfd,file_to_download_hash);
}

void  parse_seeder_list(string seeder_list){
    //cout<<"Recived seeder list: "<<seeder_list<<endl;
    vector<string> client_info = split(seeder_list,';');

    for(int i=0;i<client_info.size();i++){
        vector<string> client = split(client_info[i],'|');
        vector<string> client_ip_port = split(client[1],':');
        string client_ip = client_ip_port[0];
        string client_port_str = client_ip_port[1];
        stringstream ss(client_port_str);
        int client_port;
        ss>>client_port;
        thread connect_to_seeder(connect_to_seeders,client_ip,client_port);
        connect_to_seeder.detach();
        
    }

}

void send_request_to_tracker(string req_message){
    //printf("%s ",req_message.c_str());

    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 9735;
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);
  
    cout<<result;
    if(result == -1) {
        cout<<"Could not connect";
        exit(1);
    }
    //cout<<"req message size actual: "<<req_message.size()<<endl;
    int req_size = req_message.size();
    string req_message_size = to_string(req_size);
    //cout<<"\nSize of req message: "<<req_message_size<<endl;
    send_message(sockfd,req_message_size);
    cout<<"1. sent"<<endl;
    char recv_buffer[1024];
    int buffer_len = 1024;

    int length = recv(sockfd,recv_buffer,buffer_len,0);
    cout<<"2. received.."<<endl;
    printf("server res: %s\n", recv_buffer);
    string res(recv_buffer);

    if(res=="OK"){
        
        //cout<<"sending to server: "<<req_message<<endl;
        send_message(sockfd,req_message);
        //cout<<"3. sent"<<endl;

        memset(recv_buffer,'\0',sizeof(recv_buffer));
        
        length = recv(sockfd,recv_buffer,buffer_len,0);
        //printf("from server %s",recv_buffer);
        //cout<<endl;
        //cout<<"4. received"<<endl;

        vector<string> action = split(req_message,'|');
        if(action[0]== "get"){
            string response_from_server(recv_buffer);
            thread download_from_clients(parse_seeder_list,response_from_server);
            download_from_clients.detach();
        }
        
        // string res(recv_buffer);
        // stringstream size(res);
        // int response_size;
        // size>>response_size;
        // cout<<"response from server: "<<response_size<<endl;
        // char response_message[response_size];
        
        // string rsp="OK";
        // send_message(sockfd,rsp);
        // cout<<"5. sent.."<<endl;
        
        // memset(response_message,'\0',sizeof(response_message));
        // length = recv(sockfd,response_message,response_size,0);
        
        // cout<<"6. received.."<<endl;
        // printf("\nHere is the message: %s", response_message);
        
    }

}

string get_sha_from_torrent_file(string torrent_file_path){

    ifstream in(torrent_file_path.c_str());
    string s;
    for(int i = 0; i < 4; ++i)
       getline(in, s);

   getline(in,s);
   string sha_of_sha = create_sha_of_sha(s);
   string response = "get|"+sha_of_sha; 
   return response;
}

void print_client_menu(){

    string command;
    cout<<"############ ~~COOL~~TORRENT~~ ###############"<<endl;
    cout<<"\n1. Share: share <local file path> <filename>.mtorrent";
    cout<<"\n2. Download: ​ get <path to .mtorrent file> <destination path>";
    cout<<"\n3. Show Downloads: show downloads";
    cout<<"\n4. Remove: remove <filename.mtorrent>";
    cout<<"\n5. Exit";
    cout<<"\n\nEnter command: ";
    char ch;
    do{

        getline(cin,command);
        vector<string> cmd = split(command,' ');
        
        if(cmd[0]=="share"){
            string file_path = cmd[1];
            string torrent_file = cmd[2];
            string server_request = create_torrent_file(tracker1_ip_port,tracker2_ip_port,client_ip_port,file_path,torrent_file);
            thread share_request(send_request_to_tracker,server_request);
            share_request.detach();
        }else if(cmd[0]=="get"){
            string torrent_file = cmd[1];
            string destination = cmd[2];
            string torrent_file_path = parse_file_path(torrent_file);
            string req_to_server = get_sha_from_torrent_file(torrent_file_path);
            thread get_request(send_request_to_tracker,req_to_server);
            get_request.detach();
        }
    cout<<"Do you want to continue?(y/n): "<<endl;
   
    cin>>ch;    

    }while( ch=='y'|| ch=='Y');


}


int main(int argc, char const *argv[]){

    client_ip_port = argv[1];
    tracker1_ip_port = argv[2];
    tracker2_ip_port = argv[3];
    log_file = argv[4];

    vector<string> client_info = split(client_ip_port,':');
    client_ip = client_info[0];
    stringstream ss1(client_info[1]);
    
    ss1>>client_port;

    vector<string> tracker1info = split(tracker1_ip_port,':');
    tracker1_ip = tracker1info[0];
    
    stringstream ss2(tracker1info[1]);
    ss2>>tracker1_port;

    vector<string> tracker2info = split(tracker2_ip_port,':');
    tracker2_ip = tracker2info[0];
    
    stringstream ss(tracker2info[1]);
    ss>>tracker2_port;
    //thread client_server(start_client_server,client_ip, client_port);
   // client_server.detach();

    print_client_menu();
}
