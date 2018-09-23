#include "torrent.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <linux/limits.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <sstream>
#include <iomanip>
using namespace std;



string get_file_name_from_path(string);
string create_sha1_fingerprint(string);
size_t do_stat(string);


string create_sha_of_sha(string sha1_fingerprint){
        
        unsigned char sha1_hash[20];
        
        SHA1((unsigned char*)sha1_fingerprint.c_str(), sha1_fingerprint.size(), sha1_hash);
        
        std::stringstream s;
        for (int i = 0; i < 20; ++i) 
            s << std::hex << std::setfill('0') << std::setw(2) << (unsigned short) sha1_hash[i];
       string hash=s.str();
       
       return hash;  
}

string prepare_server_request(string file_hash, string file_name, string client_ip_port){
    string sha_of_sha = create_sha_of_sha(file_hash);
    string req = "share|"+sha_of_sha+"|"+file_name+"|"+client_ip_port;
    return req;
        
}
/**
 *  create torrent for the file to be shared. 
 */
string create_torrent_file(string tracker1_ip_port, string tacker2_ip_port, string client_ip_port, string file_path, string torren_file_name){
   
    fstream f;
    string full_file_path = parse_file_path(file_path);
    string file_name = get_file_name_from_path(full_file_path);
    string sha1_fingerprint = create_sha1_fingerprint(full_file_path);
   
    f.open(torren_file_name, ios::out|ios::in|ios::trunc);
    f<<tracker1_ip_port<<"\n";
    f<<tacker2_ip_port<<"\n";
    f<<file_name<<"\n";
    f<<do_stat(full_file_path)<<"\n";
    f<<sha1_fingerprint<<"\n";
    
    string server_request = prepare_server_request(sha1_fingerprint,full_file_path,client_ip_port);
    f.close();
    return server_request;
}

/**
 * @param: path: path of the file for which SHA1 hash to be calculated.
 * 
 * The file is read in chunks of 512KB and a SHA1 hash is calculated over each chunk. 20 chars, of each chunk 
 * are appened to string and this is stored in the torrent file.  
 * */
string create_sha1_fingerprint(string path){
      
    string sha1_fingerprint="";
    ifstream file(path, ifstream::binary);
      
    size_t total_size = do_stat(path);
    //keep chunk size as 512KB
    size_t chunk_size = 512*1024;

    //calculate total chunks  
    size_t total_chunks = total_size/chunk_size;
    // check if last chunk has remainder data.
    size_t last_chunk_size = total_size%chunk_size;
     
    //if last chunk size is not zero
    if(last_chunk_size!=0){
        //increare the total chunk to accomodate the uneven last chunk.
        total_chunks++;
    }else{
        //else, last chunk is of 512KB
        last_chunk_size = chunk_size;
    }
    // iterate over chunks.
    for(int chunk=0;chunk<total_chunks;chunk++){
        
        size_t current_chunk_size =chunk_size;

        if(chunk==total_chunks-1){
              chunk_size = last_chunk_size;
        }
        //read chunk from file
        char *chunk_data = new char[current_chunk_size];
        file.read(chunk_data, current_chunk_size);
        //create SHA1 hash
        unsigned char sha1_hash[20];
        SHA1((unsigned char*)chunk_data, strlen(chunk_data), sha1_hash);
        
        std::stringstream s;
        for (int i = 0; i < 10; ++i) 
            s << std::hex << std::setfill('0') << std::setw(2) << (unsigned short) sha1_hash[i];
        
        string hash=s.str();
        sha1_fingerprint+=hash;      
    }
      file.close();
    
    return sha1_fingerprint;
}
size_t do_stat(string path){

    struct stat fileinfo;
    stat(path.c_str(), &fileinfo);
    return fileinfo.st_size;
}

string get_file_name_from_path(string path){
    int first_index;
    int  second_index=0;
    for(int i=path.size()-1;i>=0;i--){
        if(path[i]=='/'){
            first_index = i;
            break;
        }
    }

    string fname = path.substr(first_index+1,path.size()-1);

    // for(int i=fname.size()-1;i>=0;i--){
    //     if(fname[i]=='.'){
    //         second_index = i;
    //         break;
    //     }
    // }
    // string name = fname.substr(0,second_index);
    return fname;
}

string parse_file_path(string path){

    if(path.empty() || path==""){
        cerr<<"FAILURE: INVALID_ARGUMENTS: filename";
        //exit 1;
    }
    char actualpath [PATH_MAX];
    char *full_path = realpath(path.c_str(),actualpath);
    string apath(full_path);

    return apath;
}