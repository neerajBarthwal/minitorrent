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
#include <string>

using namespace std;

 class Tracker{
    
   public:
     map<string,set<string>> seederList;

 }tracker;

 vector<string> split(const string& s, char delimiter){
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter)){
      tokens.push_back(token);
   }
   return tokens;
}

 string send_seeder_list(string file_hash){

    string response="";
    if(file_hash.empty() || file_hash=="")
        return "Invalid hash";
    
    auto it = tracker.seederList.find(file_hash);
    
    if(it!=tracker.seederList.end()){
        set<string> client_info = it->second;
        int add_delimeter = 1;
        for(auto it = client_info.begin();it!=client_info.end();it++){
          if(add_delimeter ==1)
            response = response+*it+";";
        }
    }
   return response;
 }


 void remove_shared_file(string file_hash, string client_info){
   
   // search the file hash for which client info is to be removed
   auto map_itr = tracker.seederList.find(file_hash);

   if(map_itr!=tracker.seederList.end()){
     set<string> curr_client_info = map_itr->second;
     // if client is the only one seeding the file, remove entry from tracker
     if(curr_client_info.size()==1){
       tracker.seederList.erase(file_hash);
     }else{
       //else, delete the client info and update tracker accordingly
       set<string>::iterator it = curr_client_info.begin();
       
       while(it!=curr_client_info.end()){
         
         set<string>::iterator current = it++;
         string client = *it;

         if(client==client_info){
           curr_client_info.erase(current);
           break;
         }

       }
     }

     //erase information from file
      string sed_on_file = "sed -i -r '/^("+file_hash+").*("+client_info+")$/d' seeder_file.txt";
      system(sed_on_file.c_str());
        
  }
}


 void update_seeder_list(string hash, string client_info){
   cout<<"in update seeder list: "<<hash<<" "<<client_info;
   //1. update map
   auto map_itr = tracker.seederList.find(hash);
    if( map_itr!= tracker.seederList.end()){
        //2. if exists, update client info
        set<string> curr_client_info= map_itr->second;
        curr_client_info.insert(client_info);
        tracker.seederList[hash] = curr_client_info;

    }else{
      //3. add new hash and client info
      set<string> client_info_set;
      client_info_set.insert(client_info);
      tracker.seederList.insert(make_pair(hash,client_info_set));  

    }

    //4. update seeder list file 
    ofstream seeder_file("seeder_file.txt", std::ios_base::app | std::ios_base::out);
    string tofile = hash+"|"+client_info;
    cout<<"writing to file: "<<tofile<<endl;
    seeder_file<<tofile<<"\n";
    cout<<"printing tracker: "<<endl;
     for(auto it = tracker.seederList.begin(); it!= tracker.seederList.end();it++){
    string hash = it->first;
    set<string> myset = it->second;

    //cout <<"\n"<<hash<<" ";
    for(auto it2 = myset.begin();it2!=myset.end();it2++){
      cout<<*it2<<" ";
    }
  }
 }

 void close_client(string client_info){
   cout<<"in close client: "<<client_info<<endl;
   for(auto map_itr=tracker.seederList.begin(); map_itr!=tracker.seederList.end(); map_itr++){
     set<string> client_info_set = map_itr->second;

     set<string>::iterator it = client_info_set.begin();
       
        while(it!=client_info_set.end()){
         
          set<string>::iterator current = it++;
          string client = *current;

          if(client.find(client_info) != std::string::npos){
            client_info_set.erase(current);
            break;
          }

        }
      
   }

   //string sed_on_file = "sed -i -r '/.*("+client_info+")$/d' seeder_file.txt";
   //system(sed_on_file.c_str());


 }

 string parse_message_for_action(string message){
   cout<<"in parse: "<<message<<endl;
   string response_message;
   vector<string> message_from_client = split(message,'|');
   cout<<"close size: "<<message_from_client.size();
   cout<<message_from_client[0]<<" "<<message_from_client[1]<<endl;
   string action = message_from_client[0];
   
   if(action=="share"){
     string hash = message_from_client[1];
     string client_info = message_from_client[2]+"|"+message_from_client[3];
     cout<<"before update: "<<hash<<" "<<client_info<<endl;
     update_seeder_list(hash,client_info);
     response_message="OK";
   }else if(action=="get"){
     string hash = message_from_client[1];
     response_message = send_seeder_list(hash);
   }else if(action=="remove"){
     cout<<"\n in remove: "<<"arg1: "<<message_from_client[1]<<" "<<message_from_client[2];
     remove_shared_file(message_from_client[1],message_from_client[2]);
     response_message="OK";
   }else if(action=="close"){
     cout<<"Closing client: "<<message_from_client[1];
     close_client(message_from_client[1]);
     response_message="OK";
   }

  return response_message;
 }



string parse_file_path(string path){

    if(path.empty() || path==""){
        cerr<<"FAILURE: INVALID_ARGUMENTS: filename";
       // exit 1;
    }
    char actualpath [PATH_MAX];
    char *full_path = realpath(path.c_str(),actualpath);
    string apath(full_path);

    return apath;
}


void service_request(int client_sockfd){

        char size_buffer[2048];
        int size_buffer_len = 2048;
        //receive size of client message
        recv(client_sockfd,size_buffer,size_buffer_len,0);
        cout<<"1. received"<<endl;
        printf("\nClient sent size:  %s", size_buffer);
        //initialize buffer to receive message.
        string message_size(size_buffer);
        stringstream msg_size(message_size);
        int msg_buffer_len;
        msg_size>>msg_buffer_len;
        //Message buffer initialized.
        char msg_buffer[msg_buffer_len];
        msg_buffer[msg_buffer_len]='\0';
        //Send Ok, get command.
        string response ="OK";
        send(client_sockfd,response.c_str(),response.size()+1,0);
        cout<<"2. sent.."<<endl;
        //receive the command from client.
        recv(client_sockfd,msg_buffer,msg_buffer_len,0);
        cout<<"3. received"<<endl;
        cout<<"\nclient message: "<<msg_buffer<<endl;
        //Convert the command received to string
        string message(msg_buffer);
        cout<<"\nafter converting to string: "<<message<<endl;
        // parse the message and build response.
        string response_to_client = parse_message_for_action(message);
        cout<<"resp to client to be sent.."<<response_to_client<<endl;
        // send size of response to client.
        //response = to_string(response_to_client.size()); 
        //cout<<"response size: "<<response<<endl;  
        //string client_info = send_seeder_list(message);
        send(client_sockfd,response_to_client.c_str(),response_to_client.size()+1,0);
        cout<<"4. sent size to client...";
        //wait for client to send OK,
        // memset(size_buffer,'\0',sizeof(size_buffer));
        // if(recv(client_sockfd,size_buffer,1024,0)<0)
        //   cout << "hello there " << "\n";
        // string msg = (string) size_buffer;
        // cout << msg << endl;
        // cout<<endl;
        // cout<<"5. received..";
        // string message2(size_buffer);

        // if(message2=="OK"){
        //     send(client_sockfd,response_to_client.c_str(),response_to_client.size()+1,0);
        //     cout<<"6. sent.."<<endl;
        // }

}

void start_tracker(string tracker_ip, int tracker_port){
  //cout<<"in here..";
  int server_sockfd, client_sockfd;
     
    socklen_t server_len,client_len;
     
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
     
    bzero((char *)&server_address, sizeof(server_address));
     
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(tracker_ip.c_str());
    server_address.sin_port = tracker_port;
   
    server_len = sizeof(server_address);
   
   if( bind(server_sockfd, (struct sockaddr *)&server_address, server_len)<0){
     cerr<<"Could not bind socket";
   }
    
  listen(server_sockfd, 1);

  client_len = sizeof(client_address);

  while(true){

    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
    //printf("Connection accepted from %s:%d : %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), client_sockfd);

    thread t1(service_request,client_sockfd);

    t1.detach();
  } 

  close(client_sockfd);

}


/**
 * @param: seeder file path: This file contains the information i.e. mapping between file hashes and client
 * IP and Port. This information is pipe(|) separated.
 * 
 * FILE FORMAT: <sha_hash>|file_path|<client_ip>:<client_port>
 * 
 * The following code reads the file line by line, parses the file and initializez the tacker DS.
 * Tracker maintains a map of file# string and set of clients who currently seed this file.
 * 
 * */
void initialize_seeder_list(string seeder_file_path){

  // 1. open seeder file.
  ifstream seeder_file;
  seeder_file.open(seeder_file_path);

  //2. Read seeder file line by line.
  string seeder_info;

  while(getline(seeder_file,seeder_info)){
    // 3. Initialize tracker data structure for every seeder info.
    //cout<<"\n"<<seeder_info;
    vector<string> seeder = split(seeder_info, '|');
    
    auto map_itr = tracker.seederList.find(seeder[0]);
    if( map_itr!= tracker.seederList.end()){
        //4. if exists, update client info
        set<string> curr_client_info= map_itr->second;
        curr_client_info.insert(seeder[1]+"|"+seeder[2]);
        tracker.seederList[seeder[0]] = curr_client_info;

    }else{
      //5. add new hash and client info
      set<string> client_info;
      client_info.insert(seeder[1]+"|"+seeder[2]);
      tracker.seederList.insert(make_pair(seeder[0],client_info));  

    }
      

  }
  cout<<"Tracker Seederlist initialized: "<<tracker.seederList.size();
  for(auto it = tracker.seederList.begin(); it!= tracker.seederList.end();it++){
    string hash = it->first;
    set<string> myset = it->second;

    cout <<"\n"<<hash<<" ";
    for(auto it2 = myset.begin();it2!=myset.end();it2++){
      cout<<*it2<<" ";
    }
  }

 // string response = send_seeder_list("5678");
  //cout<<response;

}

int main(int argc, char const *argv[]){

  if(argc <2){
    cerr<<"Error: Invalid arg list. Could not start tracker server.";
  }
  string tracker_ip = argv[1];
  string tracker_port_str = argv[2];
  string seeder_file_path = argv[3];
  
  seeder_file_path = parse_file_path(seeder_file_path);
  
  initialize_seeder_list(seeder_file_path);
  
  stringstream t_port(tracker_port_str);
  int tracker_port;
  t_port>>tracker_port;
  
  start_tracker(tracker_ip,tracker_port);
  
  return 0;
}