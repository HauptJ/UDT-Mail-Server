/*
* smtp.cpp 
* UDT SMTP server 
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
#include "/usr/include/udt/udt.h"
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

//using namespace std;

/*** Forward Declarations ***/

void UDTserver(char *PortNO);
void* SMTPserver(void* usocket);

/*** Global Variables ***/

std::string G_SMTPClientIP = "8.8.8.8";
std::string G_SMTPServerIP = "0.0.0.0";

/*** Function Implementations ***/


void* SMTPserver(void* usocket){
	/*** BEGIN SMTP SERVER FUNCTIONALITY ***/
	
	std::cout << "SMTPserver() entered" << std::endl;
	
	UDTSOCKET smtpsock = *(UDTSOCKET*)usocket;
	delete (UDTSOCKET*)usocket;
	
	
        int n;
        char inBuffer[2048];

        bool helo = false;
        bool auth = false;
        bool mailFrom = false;
        bool rcptTo = false;
        bool data = false;
        bool messageRecv = false;
        bool quit = false;

        // info from client
        std::string username = ""; // AUTH username
        std::string password = ""; // AUTH password
        std::string from = "";
        std::string to = "";
        std::string message = "";

        // safety counter
        int counter = 0; // TODO? add loop exit smtp error code?

        while (quit == false &&
               counter < 15) { // break if QUIT is recieved OR if
                               // safety counter reaches 15 as there
                               // should be no more than 7 iterations

          bzero(inBuffer, 2048);

          // n = read(sock_fd, inBuffer, 2047);
          /*n = recvfrom(sock_fd, inBuffer, 2048, 0, (struct sockaddr *)&cli_addr,
                       &cli_len);*/
		   if (UDT::ERROR == UDT::recv(smtpsock, inBuffer, 2048, 0))
			{
				std::cout << "recv:" << UDT::getlasterror().getErrorMessage() << std::endl;
			}
	
	std::cout << "SMTPserver() msg recieved?" << std::endl;
          printf("MSG from client : %s\n", inBuffer);
          std::string command(inBuffer);
          std::cout << "BUFFER CONVERSION" << std::endl;
          std::cout << command << std::endl;

          // Check if QUIT command is recieved from client
          if (command.find("QUIT") != std::string::npos) {
            std::cout << "QUIT recieved" << std::endl;
            // quit = true;
            std::cout << "221" << std::endl;

			if (UDT::ERROR == UDT::send(smtpsock, "221", 5, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}
            syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP", "221",
                   "QUIT");
            quit = true;
            // close connection - done at end of function call
            // can it be done here as well????
            close(smtpsock);
            break;
          }

          // Take in DATA
          if (helo == true && auth == true && mailFrom == true &&
              rcptTo == true && data == true && messageRecv == false &&
              quit == false) { // we recieved a message from the client that
            message = command; // take in message

            if (command.find("<CRLF>.<CRLF>") !=
                std::string::npos) { // Message contains <CLRF>.<CLRF>

              std::cout << "250" << std::endl;
			  
			if (UDT::ERROR == UDT::send(smtpsock, "250", 5, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}

			  
              std::cout << "MESSAGE recieved" << std::endl;
              writeEmail(from, to, message); // write email to file
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "250", message);
              messageRecv = true;
            } else { // Message does not contain a valid ending abort
              std::cout << "554" << std::endl;
			  
			if (UDT::ERROR == UDT::send(smtpsock, "554", 5, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}

			  
              std::cout << "MESSAGE BAD" << std::endl;
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "554", message);
            }
          }

          // Check if HELO command is recieved from client
          // FIRST: check if HELO is false
          if ((helo == false && auth == false && mailFrom == false &&
               rcptTo == false && data == false && messageRecv == false &&
               quit == false) &&
              command.find("HELO") != std::string::npos) {
            std::cout << "HELO recieved" << std::endl;
            syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP", "250",
                   "HELO");
            helo = true;
            std::cout << "250" << std::endl;
			
			if (UDT::ERROR == UDT::send(smtpsock, "250", 5, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}

			
          }

          /*** BEGIN AUTHENTICATION ***/

          // Check if AUTH recieved from client
          if ((helo == true && auth == false && mailFrom == false &&
               rcptTo == false && data == false && messageRecv == false &&
               quit == false) &&
              command.find("AUTH") != std::string::npos) {
            std::cout << "AUTH recieved" << std::endl;
            std::cout << "334-dXNlcm5hbWU6" << std::endl;
            syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                   "334-dXNlcm5hbWU6", "");
			
			if (UDT::ERROR == UDT::send(smtpsock, "334-dXNlcm5hbWU6", 20, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}


            // Take in username //
            bzero(inBuffer, 2048);

						 
						 
			if (UDT::ERROR == UDT::recv(smtpsock, inBuffer, 2048, 0))
			{
				std::cout << "recv:" << UDT::getlasterror().getErrorMessage() << std::endl;
			}

			
            printf("AUTH MSG from client : %s\n", inBuffer);
            std::string user(inBuffer);
            std::cout << "AUTH user BUFFER CONVERSION" << std::endl;
            std::cout << user << std::endl;

            username = user;

            // Check if user exists

            std::string rslt = passValid(user, "null");

            if (rslt != "FALSE" && rslt != "TRUE") {
              std::cout << "User does not exist" << std::endl;
              std::cout << "NPass: " << rslt << std::endl;
              std::string NPcode = "330 " + rslt;
              std::cout << "NPcode " << NPcode << std::endl;
              std::string tmpdesc =
                  "USER: " + user + " RSLT: " + rslt + " NEW USER";
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "330", tmpdesc);

			  if (UDT::ERROR == UDT::send(smtpsock, NPcode.c_str(), strlen(NPcode.c_str()) + 1, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
              // close and break the connection
              close(smtpsock);
              break;
            }

            // Tell client ready for password
            std::cout << "334-cGFzc3dvcmQ6" << std::endl;
            syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                   "334-cGFzc3dvcmQ6", "");
			
			if (UDT::ERROR == UDT::send(smtpsock, "334-cGFzc3dvcmQ6", 20, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}

            // Take in password //
            bzero(inBuffer, 2048);

						 
			if (UDT::ERROR == UDT::recv(smtpsock, inBuffer, 2048, 0))
			{
				std::cout << "recv:" << UDT::getlasterror().getErrorMessage() << std::endl;
			}
			
            printf("AUTH MSG from client : %s\n", inBuffer);
            std::string passwd(inBuffer);
            std::cout << "AUTH passwd BUFFER CONVERSION" << std::endl;
            std::cout << passwd << std::endl;

            password = passwd;
            // check if user authenticates

            std::string res = passValid(user, passwd);

            if (res == "TRUE") {
              std::cout << "235" << std::endl;
              auth = true;
              std::string tmpdesc2 = "USER: " + user + "PASS: " + passwd +
                                     " RES: " + res + " AUTH = true";
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "325", tmpdesc2);
			  
			  if (UDT::ERROR == UDT::send(smtpsock, "235", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}

            } else if (res == "FALSE") {
              std::cout << "535" << std::endl;
              std::string tmpdesc3 = "USER: " + user + "PASS: " + passwd +
                                     " RES: " + res + " AUTH = FALSE";
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "535", tmpdesc3);
			  
			  if (UDT::ERROR == UDT::send(smtpsock, "535", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
              // close and break the connection
              close(smtpsock);
              break;
            } else { // A new password was set so close the connection REDUNDANT
              std::cout << "NPassR: " << res << std::endl;
              std::string NPcode = "330 " + res;
              std::cout << "NPcode " << NPcode << std::endl;
              std::string tmpdesc4 = "USER: " + user + "PASS: " + passwd +
                                     " RES: " + res + " NEW USER2";
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "330", tmpdesc4);
			  
			  if (UDT::ERROR == UDT::send(smtpsock, NPcode.c_str(), strlen(NPcode.c_str()) + 1, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
			  
              // close and break the connection
              close(smtpsock);
              break;
            }
          }
          
          
          
          //auth = true; //TEMP AUTH BYPASS
          
          /*** END AUTHENTICATION ***/

          // Check if MAIL FROM command is recieved from client.
          // If it is, extract sender email
          // FIRST: check if HELO is true and MAIL_FROM is false
          if ((helo == true && auth == true && mailFrom == false &&
               rcptTo == false && data == false && messageRecv == false &&
               quit == false) &&
              command.find("MAIL-FROM") != std::string::npos) {
            std::cout << "MAIL-FROM recieved" << std::endl;
            // std::cout << "helo is: " << helo << std::endl;
            // check if email address contains @
            if (command.find("@447.edu") != std::string::npos) {
              // remove junk from string
              std::string tmp1 = command.substr(10, command.find(">"));
              std::cout << "TEMP FROM: " << tmp1 << std::endl;

              from = tmp1; // take in from address

              // DEBUG
              std::string fromuser = from.substr(1, from.find("@") - 1);
              std::cout << "FROM: " << fromuser << " username: " << username
                        << std::endl;
                        
              username = fromuser; //TEMP AUTH BYPASS

              // check if username and MAIL-TO are or are not the same
              if (fromuser != username) { // Not the same
                // return fail
                std::cout << "500" << std::endl; // fail
                syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                       "500", "MailFrom From and username mismatch");
				
				if (UDT::ERROR == UDT::send(smtpsock, "500", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
				
                // close and break the connection
                close(smtpsock);
                break;
              }
              if (fromuser == username) {
                // return ok
                mailFrom = true;
                std::cout << "250" << std::endl; // ok
                syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                       "250", "MailFrom ok");
				
				if (UDT::ERROR == UDT::send(smtpsock, "250", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
				
              }
            } else {
              // return fail
              std::cout << "500" << std::endl; // fail
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "500", "MailFrom fail");
			  
			  if (UDT::ERROR == UDT::send(smtpsock, "500", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
            }
          }

          // Check if RCPT_TO command is recieved from client.
          // If it is, extract reciever email address
          // FIRST: check if HELO is true, MAIL_FROM is true and RCPT_TO is
          // false
          if ((helo == true && auth == true && mailFrom == true &&
               rcptTo == false && data == false && messageRecv == false &&
               quit == false) &&
              command.find("RCPT-TO") != std::string::npos) {
            std::cout << "RCPT-TO recieved" << std::endl;
            // std::cout << "mailFrom is: " << mailFrom << std::endl;
            // check if email address contains @447.edu
            if (command.find("@447.edu") != std::string::npos) {

              // remove junk from string
              std::string tmp1 = command.substr(8, command.find(">"));
              std::cout << "TEMP TO: " << tmp1 << std::endl;
              to = tmp1; // take in to address
              // return ok
              rcptTo = true;
              std::cout << "250" << std::endl; // ok
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "250", "RCPT ok");

			  
			  if (UDT::ERROR == UDT::send(smtpsock, "250", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
            } else {
              // return fail
              std::cout << "550" << std::endl; // fail
              syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP",
                     "500", "RCPTTO fail");
			  
			  	if (UDT::ERROR == UDT::send(smtpsock, "500", 5, 0))
				{
					std::cout << "send: " << UDT::getlasterror().getErrorMessage();
				}
			  
            }
          }

          // Check if DATA command is recieved from client
          // FIRST: check if HELO is true, MAIL_FROM is true, RCPT_TO is true,
          // and
          // DATA is false
          if ((helo == true && auth == true && mailFrom == true &&
               rcptTo == true && data == false && messageRecv == false &&
               quit == false) &&
              command.find("DATA") != std::string::npos) {
            std::cout << "DATA recieved" << std::endl;
            std::cout << "354" << std::endl;
            syslog(currentTime(), G_SMTPClientIP, G_SMTPServerIP, "TCP", "354",
                   "Data OK");
			
			if (UDT::ERROR == UDT::send(smtpsock, "354", 5, 0))
			{
				std::cout << "send: " << UDT::getlasterror().getErrorMessage();
			}
			
            data = true;
          }
        }
	

    /*** END SMTP SERVER FUNCTIONALITY ***/
	
	return NULL;
	
}

void UDTserver(char *portNO) {

  std::cout << "Starting UDP server " << std::endl;
  
  int pid;


  // Hold IP as string
  char cliAddr[INET_ADDRSTRLEN];
  char srvAddr[INET_ADDRSTRLEN];
  
  /*** BEGIN UDT ***/
  
  UDTSOCKET serv, smtpsock;
  
  serv = UDT::socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  int port = atoi(portNO);
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(my_addr.sin_zero), '\0', 8);

  if (UDT::ERROR == UDT::bind(serv, (sockaddr*)&my_addr, sizeof(my_addr)))
  {
    std::cout << "bind: " << UDT::getlasterror().getErrorMessage();
  }

  UDT::listen(serv, 10);

  int namelen;
  sockaddr_in cli_addr;
  int clilen = sizeof(cli_addr);

  //UDTSOCKET recver = UDT::accept(serv, (sockaddr*)&cli_addr, &clilen);

  /*char ip[16];
  std::cout << "new connection: " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << std::endl;*/
  
  
  /*** END UDT ***/
  
	while (1) {
        smtpsock = UDT::accept(serv, (sockaddr *) &cli_addr, &clilen);
		char ip[16];
		std::cout << "new connection: " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << std::endl;
		G_SMTPClientIP = inet_ntoa(cli_addr.sin_addr);

			 
	/*** BEGIN SMTP SERVER FUNCTIONALITY ***/

	//////////////////////////////////////////////////////
	
	pthread_t rcvthread;
	pthread_create(&rcvthread, NULL, SMTPserver, new UDTSOCKET(smtpsock));
	pthread_detach(rcvthread);

    /*** END SMTP SERVER FUNCTIONALITY ***/
			 

     } /* end of while */
     UDT::close(serv);
}