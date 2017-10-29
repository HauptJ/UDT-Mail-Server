/*
* smtp.cpp 
* UDT SMTP server
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
#include <arpa/inet.h>
#include <netdb.h>
#include "/usr/include/udt/udt.h"
#include "functions.h"

using namespace std;

/*** Forward Declarations ***/

void UDTclient(char *hostname, char *portNO); // works

/*** Function Implementations ***/

void UDTclient(char *hostname, char *portNO) {

  std::cout << "Starting UDP client " << std::endl;

  struct hostent *hp;

  char inBuffer[1536];
  std::string input = "";

  UDTSOCKET client = UDT::socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  int port = atoi(portNO);
  // serv_addr.sin_port = htons(port);
  hp = gethostbyname(hostname);
  if (hp == 0)
    error("Unknown host");

  bcopy((char *)hp->h_addr, (char *)&serv_addr.sin_addr, hp->h_length);
  serv_addr.sin_port = htons(port);
  int length = sizeof(sockaddr_in);
  // inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  memset(&(serv_addr.sin_zero), '\0', 8);

  // connect to the server, implict bind
  if (UDT::ERROR ==
      UDT::connect(client, (sockaddr *)&serv_addr, sizeof(serv_addr))) {
    cout << "connect: " << UDT::getlasterror().getErrorMessage();
  }

  for (int i = 0; i < 50; i++) {

    std::cout << "Entering loop " << std::endl;

    std::cout << "Enter stuff: " << std::endl;
    bzero(inBuffer, 1536);
    input = "";
    // getline(std::cin, input);

    std::cin >> input;

    if (UDT::ERROR ==
        UDT::send(client, input.c_str(), strlen(input.c_str()) + 1, 0)) {
      cout << "send: " << UDT::getlasterror().getErrorMessage();
    }

    std::cout << "MSG SENT: " << std::endl;

    if (UDT::ERROR == UDT::recv(client, inBuffer, 1535, 0)) {
      cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
    }

    std::string code = inBuffer;
    std::cout << code << std::endl;

    // check for new password
    // new password is returned
    if (code.find("330") != std::string::npos) {
      code.erase(0, 4); // remove code from new password string
      std::string plaintxt = base64Decode(code);
      std::cout << "NEW PASSWORD: " << plaintxt << std::endl;

      // Delay restart //
      // close existing connection
      close(client);

      // Wait 5 seconds
      sleep(5);

      // restart client
      UDTclient(hostname, portNO);

      // error("NEW PASSWORD");
    }

    // check is ok returned. If not abort

    if (code == "500") { // QUIT successful
      // close connection
      UDT::close(client);
      error("ERROR mail from / to");
    }

    if (code == "535") { // QUIT successful
      // close connection
      UDT::close(client);
      error("ERROR bad password");
    }

    if (code == "221") { // QUIT successful
      // close connection
      UDT::close(client);
      error("QUIT Accepted");
    }

    if (code == "503") { // QUIT unsuccessful
      // close connection
      UDT::close(client);
      error("ERROR bad sequence");
    }

  } // END FOR

  // close socket in case loop expires
  UDT::close(client);
  error("ERROR client loop expired");
}
