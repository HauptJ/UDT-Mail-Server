#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void error(const char *msg);
void writeOKReply(std::string reply);
// CURL caller functions
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
std::string curlCaller(std::string URL);
// Base64 Authentication
std::string base64Encode(std::string userpass);
std::string base64Decode(std::string userpass);

#endif
