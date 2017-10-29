#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

#endif
