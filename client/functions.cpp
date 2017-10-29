/*
* functions.cpp 
* Client helper functions
* Joshua Haupt
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string> // strings
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
// fileIO
#include <fstream>
//CURL
#include <curl/curl.h>
#include <algorithm>


/*** Forward Declarations ***/

void error(const char *msg);
void writeOKReply(std::string reply);
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string curlCaller(std::string URL);
std::string base64Encode(std::string userpass);
std::string base64Decode(std::string userpass);


/*** Function Implementations ***/

void error(const char *msg) {
  perror(msg);
  exit(0);
}

//CURL callback
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//CURL caller
std::string curlCaller(std::string URL) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str()); // call URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    std::cout << readBuffer << std::endl;
  
  } else { //CURL failed return error
    readBuffer = "CURL ERROR";
    error("ERROR Calling external server");
  }
  return readBuffer;
}

std::string base64Encode(std::string userpass){
  
  // encode userpass to base64
  std::string encodeURL = "https://services.117.bz/tools/base64.php?fctn=encode&string=" + userpass + "&salt=447";
  std::string userPassEncoded = curlCaller(encodeURL);
  
  //remove newline
  userPassEncoded.erase(std::remove(userPassEncoded.begin(), userPassEncoded.end(), '\n'), userPassEncoded.end());
  
  return userPassEncoded;
  
}

std::string base64Decode(std::string userpass){
  
  std::cout <<"DBG NPorig: " << userpass << std::endl;
  
  //remove newline
  userpass.erase(std::remove(userpass.begin(), userpass.end(), '\r'), userpass.end());
  
  //remove newline
  userpass.erase(std::remove(userpass.begin(), userpass.end(), '\n'), userpass.end());
  
  
  // encode userpass to base64
  std::string decodeURL = "https://services.117.bz/tools/base64.php?fctn=decode&string=" + userpass + "&salt=447";
  std::cout << "Decode URL: " << decodeURL << std::endl;
  std::string userPassDecoded = curlCaller(decodeURL);
  
  //remove newline
  userPassDecoded.erase(std::remove(userPassDecoded.begin(), userPassDecoded.end(), '\n'), userPassDecoded.end());
  
  std::cout <<"DBG NPdec: " << userPassDecoded << std::endl;
  
  return userPassDecoded;
  
}


void writeOKReply(std::string reply) {

  using namespace std;

  ofstream outfile("recieve.txt", ios::app); // enable appending

  // file could not be openned
  if (!outfile) {
    // Print an error and exit
    error("ERROR recieve.txt could not be opened for writting."); // display
                                                                  // error and
                                                                  // close
                                                                  // program
  }

  outfile << reply << std::endl;
}
