Compile:
Run makefile in client and serv directory.

Requires: 
You may or may not need to install these packages using your package manager. 
The following package names reffer to the default Ubuntu package library. They may be named
differently depending on which package manager you use.

Clang++ 3.8 
later versions may work but have not been tested.

libcurl4-openssl-dev 
libcurl4 OpenSSL: For base64 encoding and decoding and random string generation
refer to REPORT

libudt-dev 
UDT: https://en.wikipedia.org/wiki/UDP-based_Data_Transfer_Protocol

If using Ubuntu Server 16.04 the following commands will install all required packages:
apt-get update && apt-get upgrade -y
apt-get install -y git build-essential clang-3.8 clang++-3.8 lldb-3.8 gcc g++ libboost-all-dev libudt-dev libcurl4-openssl-dev



Run:
server: ./server UDP TCP
client: ./client hostname UDP TCP

Note:
Working command sequence can be found in REPORT.

