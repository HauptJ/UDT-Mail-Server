/*
* client.cpp 
* Client driver to start TCP HTTP and UDT SMTP servers simultaniously
* Joshua Haupt
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string> // strings
#include <iostream>
// Error Function
#include "functions.h"
// Clients
#include "smtp.h"
#include "http.h" 



int main(int argc, char *argv[]) {

  if (argc < 4) {
    std::cout << "Hostname, UDP Port, TCP Port" << std::endl;
    exit(0);
  }

  int mode;

  std::cout << "Send [0] or recieve [1] Emails? " << std::endl;
  std::cin >> mode;

  if (mode == 0) {
    UDTclient(argv[1], argv[2]);
  } else if (mode == 1) {
    TCPclient(argv[1], argv[3]);
  } else {
    error("Bad Input");
  }

  return 0;
}


