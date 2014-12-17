#include <stdio.h>
#include <string>
#include <sstream>

using namespace std;

string to_string(int val){
    stringstream ss; 
    ss << val;
    return ss.str();
}

string charto_string(char val){
    stringstream ss; 
    ss << val;
    return ss.str();
}
