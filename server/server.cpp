/*
* server.cpp 
* Driver to start both UDT (SMTP) and TCP (HTTP) server processes simultaniously 
* Joshua Haupt
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <string>   // strings
#include <iostream> // file IO
// Servers
#include "smtp.h"
#include "http.h" 

/*** Function Implementation ***/

int main(int argc, char *argv[]) {

  if (argc < 3) {
    std::cout << "UDP Port, TCP Port " << std::endl;
    exit(1);
  }
  
  //Start both TCP (HTTP) and UDT (SMTP) server processes simultaniously.

  pid_t pid = fork();

  if (pid == 0) {
    TCPserver(argv[2]);
  } else if (pid > 0) {
    UDTserver(argv[1]);
  } else {
    std::cout << "Servers failed to start" << std::endl;
    return 1;
  }

  return 0;
}


