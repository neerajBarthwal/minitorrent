#include <stdio.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream> 
#include <map> 
#include <iterator> 
#include <string>
#ifndef torrent_H
#define torrent_H
class Message{

    public:

    std::string action;
    std::string msg;
    

};

//create torrent
std::string create_torrent_file(std::string, std::string, std::string, std::string, std::string);
std::string parse_file_path(std::string path);
std::string create_sha_of_sha(std::string);
#endif