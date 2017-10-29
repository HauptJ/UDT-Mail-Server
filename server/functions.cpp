/*
* functions.cpp 
* Server helper functions
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


/*** Forward Declarations ***/

void error(const char *msg);
void writeEmail(std::string from, std::string to, std::string message);
std::string writePassword(std::string user); // set new random password for a user
std::string currentTime(); // returns current time and date
bool fileExists(const std::string &filename); // check if file exists
std::string readFile(const std::string &filename);
//Verify Password
std::string passValid(std::string user, std::string userPasswd);
//CURL
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string curlCaller(std::string URL);
//LOGGING
void syslog(std::string time, std::string fromIP, std::string toIP, std::string protocol, std::string code, std::string desc);


/*** Function Implementations ***/

//Error handling
//Abort execution after displaying error message
void error(const char *msg) {
  perror(msg);
  exit(1);
}


// Get current time
std::string currentTime() {

  time_t rawTime;
  time(&rawTime);

  std::string currTime = ctime(&rawTime);

  return currTime;
}


//Logging
void syslog(std::string time, std::string fromIP, std::string toIP, std::string protocol, std::string code, std::string desc){
  
  std::string logLine = "Time: " + time + " From IP: " + fromIP + " To IP: " + toIP + " Protocol: " + protocol + " Code: " + code + " Desc: " + desc + "\n";
  
  std::ofstream logFile(".server_log", std::ios::app);

  // could not open file :(
  if (!logFile) {
    // error and exit
    error("ERROR could not write to log file");
  }

  // write to file
  logFile << logLine << std::endl;

  logFile.close(); // close the email file
}


void writeEmail(std::string from, std::string to, std::string message) {

  int fileCount = 0;

  std::string dirBegin = "db/";
  std::string mkdir = "mkdir ";
  std::string dir = "dir ";

  std::string emailExt = ".email";

  std::string fromUser = "";
  std::string toUser = "";

  std::string toUserDir = "";
  std::string countToUserDir = "";
  std::string makeToUserDir = "";
  std::string makeToUserFile = "";

  // create db folder
  // TODO - MAYBE: check if folder already exists
  system("mkdir db");

  // Generate email string
  std::string emailBegin = "Answer \n Date: ";
  std::string emailTime = currentTime();
  std::string emailBeginFrom = "From: ";
  std::string emailBeginTo = " \n To: ";
  std::string emailBeginEnd = " \n";

  std::string emailString = emailBegin + emailTime + emailBeginFrom + from +
                            emailBeginTo + to + emailBeginEnd + message;
  std::cout << "DBG: Email String: " << emailString << std::endl;

  // extract users

  // MAIL FROM -> fromUser
  // remove junk from string
  std::string tmp1 = from.substr(1, from.find("@")-1);
  std::cout << "TEMP FROMUSER: " << tmp1 << std::endl;

  fromUser = tmp1; // take in from address
  std::cout << "DBG: From user: " << fromUser << std::endl;

  // RCPT TO -> toUser
  // remove junk from string
  std::string tmp2 = to.substr(1, to.find("@")-1);
  std::cout << "TEMP TOUSER: " << tmp2 << std::endl;

  toUser = tmp2; // take in from address
  std::cout << "DBG: TOUSER: " << toUser << std::endl;

  // generate toUser directory
  toUserDir = dirBegin + toUser;
  makeToUserDir = mkdir + toUserDir;
  std::cout << "make to user directory CMD: " << makeToUserDir << std::endl;
  system(makeToUserDir.c_str());

  // generate email file

  // count number of files in user directory
  // if file exists in directory increment counter by 1

  std::string testMakeToUserFile = "";

  int loopCounter = 0;

  testMakeToUserFile = toUserDir + "/" + std::to_string(loopCounter) + emailExt;
  std::cout << testMakeToUserFile << " test file" << std::endl;
  std::cout << fileExists(testMakeToUserFile) << " does it exist?" << std::endl;

  while (fileExists(testMakeToUserFile) == true) {

    std::cout << testMakeToUserFile << " Exists" << std::endl;
    loopCounter++;
    testMakeToUserFile =
        toUserDir + "/" + std::to_string(loopCounter) + emailExt;
  }

  // fileCount = system(countToUserDir.c_str());
  std::cout << "Number of files in: " << toUserDir << " is " << fileCount
            << std::endl;

  // generate file name
  fileCount = loopCounter + 1;
  makeToUserFile = toUserDir + "/" + std::to_string(fileCount) + emailExt;
  std::cout << "DBG: Email File Name: " << makeToUserFile << std::endl;

  // Write the file

  std::ofstream emailFile(makeToUserFile);

  // could not open file :(
  if (!emailFile) {
    // error and exit
    error("ERROR could not write file");
  }

  // write to file
  emailFile << emailString << std::endl;

  emailFile.close(); // close the email file
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


/*** write password to file ***/

std::string writePassword(std::string user) {
  
  std::string pass = ""; // plain text random password
  std::string passBase64 = ""; // plain text random password encoded in base64
  
  std::string passFilePath = "db/" + user + "/.user_pass";
  
  //get random password
  pass = curlCaller("https://www.random.org/strings/?num=1&len=5&digits=on&upperalpha=on&loweralpha=on&unique=on&format=plain&rnd=new");
  
  //Check if password was generated. If not abort
  if(pass == "ERROR Calling external server"){
   error("ERROR Password generation"); 
  }
  
  //remove newline
  pass.erase(std::remove(pass.begin(), pass.end(), '\n'), pass.end());
  
  //remove newline
  pass.erase(std::remove(pass.begin(), pass.end(), '\r'), pass.end());
  
  std::cout << "NEW Password: " << pass << std::endl;
  
  //encode password in base64
  std::string encodeURL = "https://services.117.bz/tools/base64.php?fctn=encode&string=" + pass + "&salt=447";
  std::cout << "Encode URL: " << encodeURL << std::endl;
  passBase64 = curlCaller(encodeURL);
  
  //Check if password was encoded. If not abort
  if(passBase64 == "ERROR Calling external server"){
   error("ERROR Password encoding"); 
  }
  
  //remove new line
  passBase64.erase(std::remove(passBase64.begin(), passBase64.end(), '\n'), passBase64.end());
  
  passBase64.erase(std::remove(passBase64.begin(), passBase64.end(), '\r'), passBase64.end());
  
  
  
  //TEMP bypass
  //passBase64 = pass;
  
  std::cout << "NEW Encoded Password: " << passBase64 << std::endl;
  
  // Write the file
  
  // 1st make directory
  
  std::string mkdircmd = "mkdir db/" + user + "/";
  system(mkdircmd.c_str());
  std::string touchcmd = "touch " + passFilePath;
  system(touchcmd.c_str());
  

  std::ofstream passFile(passFilePath, std::ios::out);

  // could not open file :(
  if (!passFile) {
    // error and exit
    error("ERROR could not write to password file");
  }

  // write to file
  passFile << passBase64 << std::endl;

  passFile.close(); // close the email file
  
    
  //return pass;
  return passBase64;
  
}

/*** Check if user exists if so check if pass is correct. If not generate a pass and exit**/

std::string passValid(std::string user, std::string userPasswd){
  
  std::string newPass = "";
  std::string userPass = "";
  std::string userPassEncoded = "";
  std::string filePass = "";
  
  std::string out = "";
  
  // check if user exists
  
  std::string passFilePath = "db/" + user + "/.user_pass";
  
  // get encoded password from file. If this fails the user probably does not exist
  if(readFile(passFilePath) != "FILE ERROR") {
    filePass = readFile(passFilePath); // User exists get pass to compare against
    std::cout <<"File pws: " << filePass << std::endl;
  } else {
    // generate password for user
    newPass = writePassword(user);
    return newPass;
  }
  
  // encode userpass to base64
  //std::string encodeURL = "https://services.117.bz/tools/base64.php?fctn=encode&string=" + userPasswd + "&salt=447";
  //userPassEncoded = curlCaller(encodeURL);
  
  //quick fix to have encoding done by client
  userPassEncoded = userPasswd;
  
  //remove newline
  userPassEncoded.erase(std::remove(userPassEncoded.begin(), userPassEncoded.end(), '\n'), userPassEncoded.end());
  
  userPassEncoded.erase(std::remove(userPassEncoded.begin(), userPassEncoded.end(), '\r'), userPassEncoded.end());

  std::cout <<"userPassEncoded: " << userPassEncoded << " filePass: " << filePass << std::endl;
  
  if(userPassEncoded == filePass){
    std::cout << "Password is valid" << std::endl;
    out = "TRUE";
    return out;
  } else {
    std::cout << "Password is not valid" << std::endl;
    out = "FALSE";
    return out;
  }
  
  std::cout << "Password is not validE" << std::endl;
  out = "FALSE";
  return out;
}


// check if file exists
// RETURNS: true if file exists. false if file does not exist.
bool fileExists(const std::string &filename) { // broken

  std::ifstream file(filename.c_str());
  return file.good();
}

std::string readFile(const std::string &filename) {
  std::ifstream infile(filename);

  std::cout << "opening file: " << filename << std::endl;

  std::string output = "";

  // If we couldn't open the input file stream for reading
  if (!infile) {
    // Print an error and exit
    std::cout << "File must not exists" << std::endl;
    output = "FILE ERROR";
    return output;
  }

  // While there's still stuff left to read
  while (infile) {
    // read stuff from the file into a string and print it
    std::string strInput;
    getline(infile, strInput);
    output += strInput;
  }
  return output;
}
