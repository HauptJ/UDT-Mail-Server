/*
* http.cpp 
* TCP HTTP client
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
// CURL
#include <curl/curl.h>
#include <algorithm>
#include "functions.h"

/*** Forward Declarations ***/

void TCPclient(char *hostname, char *portNO);

/*** Function Implementations ***/

void TCPclient(char *hostname, char *portNO) {

  std::cout << "TCP reciever started " << std::endl;

  bool QUIT = false;
  bool AUTH = false;
  int counter = 0;

  std::string code = "";

  std::string userName = "";
  std::string password = "";
  int emailCount; // for loop to get emails sent back from server. Send to
                  // server.

  int sock;
  int n;

  unsigned int length;
  struct sockaddr_in server, from;
  struct hostent *hp;
  char inBuffer[1536];

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    error("socket");

  server.sin_family = AF_INET;
  hp = gethostbyname(hostname);
  if (hp == 0)
    error("Unknown host");

  bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
  server.sin_port = htons(atoi(portNO));

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    error("ERROR connecting");
  }

  length = sizeof(struct sockaddr_in);

  // AUTHENTICATION //

  // Write AUTH
  bzero(inBuffer, 1536);

  n = write(sock, "AUTH", strlen("AUTH") + 1);
  if (n < 0) {
    error("ERROR sending AUTH");
  }

  n = read(sock, inBuffer, 1535);
  if (n < 0) {
    error("ERROR recieving from server");
  }
  printf("%s\n", inBuffer);

  code = inBuffer;
  std::cout << "AUTH BUFFER CONV" << std::endl;
  std::cout << code << std::endl;

  // check is ok returned. If not abort
  if (code != "334-dXNlcm5hbWU6") {
    error("ERROR AUTH username not ready");
  }

  // Server must be ready for AUTH
  std::cout << "Enter username: " << std::endl;
  // remove @ from user input
  std::string userNameTmp = "";
  std::cin >> userNameTmp; // take in username

  userName = userNameTmp.substr(0, userNameTmp.find("@"));
  std::cout << "userName cleaned: " << userName << std::endl;

  // clear buffer
  bzero(inBuffer, 1536);

  n = write(sock, userName.c_str(), strlen(userName.c_str()));
  if (n < 0) {
    error("ERROR sending AUTH");
  }

  n = read(sock, inBuffer, 1535);
  if (n < 0) {
    error("ERROR recieving from server");
  }
  printf("%s\n", inBuffer);

  code = inBuffer;
  std::cout << code << std::endl;

  // new password is returned
  if (code != "334-cGFzc3dvcmQ6") {
    code.erase(0, 4); // remove code from new password string
    std::string plaintxt = base64Decode(code); // decode new password
    std::cout << "NEW PASSWORD: " << plaintxt << std::endl;

    // Delay restart //

    // Wait 5 seconds
    sleep(5);

    // restart client
    TCPclient(hostname, portNO);

    // error("NEW PASSWORD");
  }

  // We are cleared to send the password
  if (code == "334-cGFzc3dvcmQ6") {
    // Server must be ready for AUTH
    std::cout << "Enter password: " << std::endl;
    std::string plainPasswd = "";
    std::cin >> plainPasswd; // take in password as plain text
    password = base64Encode(plainPasswd);

    // clear buffer
    bzero(inBuffer, 1535);


    n = write(sock, password.c_str(), strlen(password.c_str()));

    if (n < 0) {
      error("ERROR sending pass");
    }

    // check if we are authenticated
    // clear buffer
    bzero(inBuffer, 1535);

    n = read(sock, inBuffer, 1535);
    if (n < 0) {
      error("ERROR recieving from server");
    }

    std::cout << "Reply code from server" << std::endl;
    printf("%s\n", inBuffer);

    code = inBuffer;
    std::cout << code << std::endl;

    // check is fail returned. If so abort
    if (code == "535") {
      error("INVALID PASSWORD");
    }
  }

  std::cout << "Generating GET request " << std::endl;

  // get info from user
  std::cout << "Enter username: " << std::endl;
  std::cin >> userName;

  std::cout << "How many emails would you like?: " << std::endl;
  std::cin >> emailCount;
  std::string emailCountString = std::to_string(emailCount);

  // generate GET request buffer string
  std::string GETresquestHostString(hostname);

  std::string GETrequestBegin = "GET _/db/";
  std::string GETrequestMiddle = "/+ HTTP/1.1 \n Host:<";
  std::string GETrequestEnd = "> \n Count: ";
  std::string GETrequestString = GETrequestBegin + userName + GETrequestMiddle +
                                 GETresquestHostString + GETrequestEnd +
                                 emailCountString;
  std::cout << "GET Rquest" << std::endl;
  std::cout << GETrequestString << std::endl;

  n = write(sock, GETrequestString.c_str(), strlen(GETrequestString.c_str()));
  if (n < 0) {
    error("ERROR sending to server");
  }

  bzero(inBuffer, 2048);
  n = read(sock, inBuffer, 2047);
  if (n < 0) {
    error("ERROR recieving from server");
  }

  std::cout << "Reply from server" << std::endl;
  printf("%s\n", inBuffer);

  writeOKReply(inBuffer);

  close(sock);

  exit(1); // exit gracefully
}
