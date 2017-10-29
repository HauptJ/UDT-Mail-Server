/*
* http.cpp 
* TCP HTTP server
* Joshua Haupt
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <string>   // strings
#include <iostream> // file IO
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
// clock
#include <time.h>
#include <ctime>
// file IO
#include <fstream>
#include <sys/stat.h>
// CURL
#include <curl/curl.h>
#include <algorithm>
// Functions
#include "functions.h"

/*** Forward Declarations ***/

void HTTPserver(int sock);
void TCPserver(char *portNO);

/*** Global Variables ***/

std::string G_HTTPClientIP = "8.8.8.8"; // dummy IP address in case actual IP is not returned
std::string G_HTTPServerIP = "0.0.0.0"; // dummy IP address in case actual IP is not returned

/*** Function Implementations ***/

void HTTPserver(int sock) {

  std::cout << "Starting HTTP server " << std::endl;

  int n;
  int pid;

  char inBuffer[2048];

  std::string username;
  std::string password;

  bool AUTH = false;

  bool EXISTS = false;
  bool GET = false;

  // Hold IP as string
  char cliAddr[INET_ADDRSTRLEN];
  char srvAddr[INET_ADDRSTRLEN];

  // Initial
  // clear buffers
  bzero(inBuffer, 2048);

  // n = recvfrom(sock_fd, inBuffer, 2048, 0, (struct sockaddr *)&cli_addr,
  // &cli_len);
  n = read(sock, inBuffer, 2047);
  if (n < 0) {
    error("ERROR Recieving from client");
  }
  std::cout << "Recieved initial command from client" << std::endl;
  printf("%s\n", inBuffer);

  // Client IP to string
  // inet_ntop(AF_INET, &(cli_addr.sin_addr), cliAddr, INET_ADDRSTRLEN);
  // G_HTTPClientIP = cliAddr;
  std::cout << "Client IP: " << G_HTTPClientIP << std::endl;

  // convert inBuffer to command string
  std::string command(inBuffer);

  /*** BEGIN AUTHENTICATION ***/

  // Check if AUTH recieved from client
  if (AUTH == false && command.find("AUTH") != std::string::npos) {
    std::cout << "AUTH recieved" << std::endl;
    std::cout << "334-dXNlcm5hbWU6" << std::endl;
    syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP",
           "334-dXNlcm5hbWU6", "");
    n = write(sock, "334-dXNlcm5hbWU6", 20);
    if (n < 0) {
      error("ERROR sending 334-dXNlcm5hbWU6");
    }

    // Take in username //
    // clear buffers
    bzero(inBuffer, 2048);

    n = read(sock, inBuffer, 2047);
    if (n < 0) {
      error("ERROR AUTH user");
    }
    printf("AUTH MSG from client : %s\n", inBuffer);
    std::string user(inBuffer);
    std::cout << "AUTH user BUFFER CONVERSION" << std::endl;
    std::cout << user << std::endl;

    username = user;

    // Check if user exists

    std::string rslt = passValid(user, "null");

    if (rslt != "FALSE" && rslt != "TRUE") { // User does not exist
      std::cout << "User does not exist" << std::endl;
      // std::string rslt = passValid(user,"null");
      std::cout << "NPass: " << rslt << std::endl;
      std::string NPcode = "330 " + rslt;
      std::cout << "NPcode " << NPcode << std::endl;
      std::string tmpdesc = "USER: " + user + " RSLT: " + rslt + " NEW USER";
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "330",
             tmpdesc);
      n = write(sock, NPcode.c_str(), 2047);

      // Restart TCP server
      close(sock);
      // TCPserver(portNO);
      // break;

      if (n < 0) {
        error("ERROR sending 330");
      }
    }

    // clear buffers
    bzero(inBuffer, 2048);

    // Tell client ready for password
    std::cout << "334-cGFzc3dvcmQ6" << std::endl;
    syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP",
           "334-cGFzc3dvcmQ6", "");
    n = write(sock, "334-cGFzc3dvcmQ6", 2047);
    if (n < 0) {
      error("ERROR sending 334-cGFzc3dvcmQ6");
    }

    // Take in password //
    // clear buffers
    bzero(inBuffer, 2048);

    n = read(sock, inBuffer, 2047);
    if (n < 0) {
      error("ERROR AUTH password");
    }
    printf("AUTH MSG from client : %s\n", inBuffer);
    std::string passwd(inBuffer);
    std::cout << "AUTH passwd BUFFER CONVERSION" << std::endl;
    std::cout << passwd << std::endl;

    password = passwd;
    // check if user authenticates

    std::string res = passValid(user, passwd);

    if (res == "TRUE") {
      AUTH = true;
      std::cout << "235" << std::endl;
      std::string tmpdesc2 =
          "USER: " + user + "PASS: " + passwd + " RES: " + res + " AUTH = true";
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "235",
             tmpdesc2);
      n = write(sock, "235", 2047);
      if (n < 0) {
        error("ERROR sending 235");
      }

    } else if (res == "FALSE") {
      std::cout << "535" << std::endl;
      std::string tmpdesc3 = "USER: " + user + "PASS: " + passwd + " RES: " +
                             res + " AUTH = false";
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "535",
             tmpdesc3);
      n = write(sock, "535", 2047);
      if (n < 0) {
        error("ERROR sending 535");
      }
    } else { // A new password was set so close the connection
      std::cout << "NPassR: " << res << std::endl;
      std::string NPcode = "330 " + res;
      std::cout << "NPcode " << NPcode << std::endl;
      std::string tmpdesc4 =
          "USER: " + user + "PASS: " + passwd + " RES: " + res + " NEW USER2";
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "330",
             tmpdesc4);
      n = write(sock, NPcode.c_str(), 2047);

      // Restart TCP server

      close(sock);
      // TCPserver(portNO);
      // break;

      if (n < 0) {
        error("ERROR sending 330");
      }
    }
  }

  /*** END AUTHENTICATION ***/

  if (AUTH == true) {
    // clear buffers
    bzero(inBuffer, 2048);

    n = read(sock, inBuffer, 2047);
    if (n < 0)
      error("ERROR Recieving from client");
    std::cout << "Recieved message from client" << std::endl;
    printf("%s\n", inBuffer);

    // convert inBuffer to command string
    std::string command(inBuffer);

    // the last character in command should have the count value

    std::string lastChar = command.substr(command.length() - 1);
    std::cout << "DBG: last char: " << lastChar << std::endl;
    int cnt = 0;
    cnt = atoi(lastChar.c_str());
    std::cout << "DBG: cnt: " << cnt << std::endl;

    // get directory from GET request string
    std::string directory = "";
    int firstDir = command.find("_");
    int lastDir = command.find_last_of("+");
    directory = command.substr(firstDir + 2, (lastDir - firstDir) - 2);
    // directory = "test"; //TEMP BYPASS
    std::cout << "DBG: directory: " << directory << std::endl;

    // get hostname from GET request string
    std::string hostname = "";
    int firstHost = command.find("<");
    int lastHost = command.find_last_of(">");
    hostname = command.substr(firstHost, (lastHost - firstHost));
    std::cout << "DBG: hostname: " << hostname << std::endl;

    // check if file exists if so, set exists to true
    std::string testEmail = directory + std::to_string(cnt) + ".email";

    // works?
    if (readFile(testEmail) != "FILE ERROR") {
      EXISTS = true;
    } else {
      EXISTS = false;
    }

    std::cout << "File: " << testEmail << std::endl;

    // send 200 OK
    if ((GET == false && EXISTS == true && AUTH == true) &&
        ((command.find("GET") != std::string::npos) &&
         (command.find("Count:") != std::string::npos))) {
      std::cout << "200" << std::endl;
      std::string GETreplyHostString(hostname);
      std::string GETreplyTimeString = currentTime();
      std::string GETreplyBegin = "HTTP/1.1 200 OK \n Server:<";
      std::string GETreplyMiddle = "> \n Last-Modified: ";
      std::string GETreplyString = GETreplyBegin + GETreplyHostString +
                                   GETreplyMiddle + GETreplyTimeString +
                                   "/n Message: " + std::to_string(cnt) +
                                   readFile(testEmail);

      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "200",
             GETreplyString);
      n = write(sock, GETreplyString.c_str(), 2047);
      if (n < 0) {
        error("ERROR sending 200");
      }
      GET = true;
    }

    // send 404 Not Found
    else if ((GET == false && EXISTS == false && AUTH == true) &&
             ((command.find("GET") != std::string::npos) &&
              (command.find("Count:") !=
               std::string::npos))) { // return 404 not found
      std::cout << "404" << std::endl;
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "404",
             "Not Found");
      n = write(sock, "404", 2047);
      if (n < 0) {
        error("ERROR sending 404");
      }
    }

    // SEND 400
    else if ((GET == false && AUTH == true) &&
             !((command.find("GET") != std::string::npos) &&
               (command.find("Count:") !=
                std::string::npos))) { // return 400 bad request
      std::cout << "400" << std::endl;
      syslog(currentTime(), G_HTTPClientIP, G_HTTPServerIP, "TCP", "400",
             "Bad Request");
      n = write(sock, "400", 2047);
      if (n < 0) {
        error("ERROR sending 400");
      }
    }

    // Restart UDP server
    close(sock);
    // TCPserver(portNO);
    // break;

    // reset variables then repeat - WE SHOULD NOT GO HERE
    AUTH = false;
    GET = false;
    EXISTS = false;
  }
}

// send emails requested from GET request
void TCPserver(char *portNO) {

  std::cout << "Starting TCP server " << std::endl;

  int sock_fd, newsock_fd;
  int length;
  int n;
  int pid;
  socklen_t cli_len;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;

  // Hold IP as string
  char cliAddr[INET_ADDRSTRLEN];
  // char srvAddr[INET_ADDRSTRLEN];

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0)
    error("ERROR Opening socket");
  length = sizeof(serv_addr);
  bzero(&serv_addr, length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(atoi(portNO));
  if (bind(sock_fd, (struct sockaddr *)&serv_addr, length) < 0)
    error("ERROR Binding");

  listen(sock_fd, 5);

  // cli_len = sizeof(struct sockaddr_in);
  cli_len = sizeof(cli_addr);

  while (1) {

    newsock_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_len);

    if (newsock_fd < 0) {
      error("ERROR on accept");
    }

    // Client IP to string
    inet_ntop(AF_INET, &(cli_addr.sin_addr), cliAddr, INET_ADDRSTRLEN);
    G_HTTPClientIP = cliAddr;
    std::cout << "Client IP: " << G_HTTPClientIP << std::endl;

    pid = fork();

    if (pid == 0) {
      close(sock_fd);
      HTTPserver(newsock_fd);
      exit(0);
    } else {
      close(newsock_fd);
    }
  } // end while
  close(sock_fd);
  // return 0; //we should not go here
}
