/*
 * basicSockets.cpp
 *
 *  Created on: Aug 3, 2015
 *      Author: froike(Roi Inbar) 
 * 	Modified: Aner Ben-Efraim
 * 
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "basicSockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>
#include <netinet/tcp.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
using namespace std;

#define bufferSize 256

#ifdef __linux__
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>

	#define Sockwrite(sock, data, size) write(sock, data, size) 
	#define Sockread(sock, buff, bufferSize) read(sock, buff, bufferSize)
	//#define socklen_t int
#elif _WIN32
	//#pragma comment (lib, "ws2_32.lib") //Winsock Library
	#pragma comment (lib, "Ws2_32.lib")
	//#pragma comment (lib, "Mswsock.lib")
	//#pragma comment (lib, "AdvApi32.lib")
	#include<winsock.h>
	//#include <ws2tcpip.h>
	#define socklen_t int
	#define close closesocket
	#define Sockwrite(sock, data, size) send(sock, (char*)data, size, 0)
	#define Sockread(sock, buff, bufferSize) recv(sock, (char*)buff, bufferSize, 0)
	
#endif

/*GLOBAL VARIABLES - LIST OF IP ADDRESSES*/
char** localIPaddrs;
int numberOfAddresses;

//For communication measurements
CommunicationObject commObject;



std::string exec(const char* cmd) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return result;
}

string getPublicIp()
{
	string s = exec("dig TXT +short o-o.myaddr.l.google.com @ns1.google.com");
    s = s.substr(1, s.length()-3); 
    return s;
}

char** getIPAddresses(const int domain)
{
  char** ans;
  int s;
  struct ifconf ifconf;
  struct ifreq ifr[50];
  int ifs;
  int i;

  s = socket(domain, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket");
    return 0;
  }

  ifconf.ifc_buf = (char *) ifr;
  ifconf.ifc_len = sizeof ifr;

  if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
    perror("ioctl");
    return 0;
  }

  ifs = ifconf.ifc_len / sizeof(ifr[0]);
  numberOfAddresses = ifs+1;
  ans = new char*[ifs+1];

  string ip = getPublicIp(); 
  ans[0] = new char[ip.length()+1];
  strcpy(ans[0], ip.c_str());
  ans[0][ip.length()] = '\0';

  for (i = 1; i <= ifs; i++) {
    char* ip=new char[INET_ADDRSTRLEN];
    struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

    if (!inet_ntop(domain, &s_in->sin_addr, ip, INET_ADDRSTRLEN)) {
      perror("inet_ntop");
      return 0;
    }

    ans[i]=ip;
  }

  close(s);

  return ans;
}

int getPartyNum(char* filename)
{

	FILE * f = fopen(filename, "r");

	char buff[STRING_BUFFER_SIZE];
	char ip[STRING_BUFFER_SIZE];

	localIPaddrs=getIPAddresses(AF_INET);
	string tmp;
	int player = 0;
	//for (int i = 0; i < numberOfAddresses; i++)
	//	cout << localIPaddrs[i] << endl;
	while (true)
	{
		fgets(buff, STRING_BUFFER_SIZE, f);
		sscanf(buff, "%s\n", ip);
		for (int i = 0; i < numberOfAddresses; i++)
			if (strcmp(localIPaddrs[i], ip) == 0 || strcmp("127.0.0.1", ip)==0)
				return player;
		player++;
	}
	fclose(f);

}

BmrNet::BmrNet(char* host, int portno) {
	// zmq::context_t context{1};
    // // construct a REQ (request) socket and connect to interface
    // zmq::socket_t socket{context, zmq::socket_type::req};
	this->port = portno;
#ifdef _WIN32
	this->Cport = (PCSTR)portno;
#endif
	this->host = host;
	this->is_JustServer = false;
	for (int i = 0; i < NUMCONNECTIONS; i++) this->socketFd[i] = -1;
#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}
	else printf("WSP Initialised.\n");
#endif

}

BmrNet::BmrNet(int portno) {
	this->port = portno;
	this->host = "";
	this->is_JustServer = true;
	// this->socket->setsockopt(ZMQ_LINGER, 100);
	for (int i = 0; i < NUMCONNECTIONS; i++) this->socketFd[i] = -1;
#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}
	else printf("WSP Initialised.\n");
#endif
}

BmrNet::~BmrNet() {
	for (int conn = 0; conn < NUMCONNECTIONS; conn++)
	close(socketFd[conn]);
	socket->close();
	#ifdef _WIN32
		WSACleanup();
	#endif
	//printf("Closing connection\n");
}

bool BmrNet::listenNow(){
	
	context = new zmq::context_t(1);
	socket = new zmq::socket_t(*context, ZMQ_PULL);
	// socket = new zmq::socket_t(*context, ZMQ_REP);
	// socket->setsockopt(ZMQ_ROUTER_RAW,1);
	// socket->setsockopt(ZMQ_GSSAPI_PLAINTEXT,1);
	string recv_address = "tcp://*:" + std::to_string(port);//"tcp://127.0.0.1:" + std::to_string(port);
	cout<<recv_address<<endl;
	socket->bind(recv_address);

	// pause();
	return 1;
}


bool BmrNet::connectNow(){
	
	context = new zmq::context_t(1);
	socket = new zmq::socket_t(*context, ZMQ_PUSH);
	// socket = new zmq::socket_t(*context, ZMQ_REQ);
	// socket->setsockopt(ZMQ_ROUTER_RAW,1);
	// socket->setsockopt(ZMQ_GSSAPI_PLAINTEXT,1);
	string send_address = "tcp://"+  std::string(host) + ":" + std::to_string(port);
	cout<<send_address<<endl;
	socket->connect(send_address);

	// pause();
	return 1;
}

void BmrNet::fixSend( int size){
	commObject.reduceSent(size);
}

void BmrNet::fixReceive( int size){
	commObject.reduceSent(size);
}

bool BmrNet::sendMsg(const void* data, int size, int conn){

	unsigned char buffer_in[sizeof(CMD_SUCCESS)];
	// int left = size;
	// int n;
	// assert (socket);
	// int rc = zmq_send (socket, data, size, ZMQ_SNDMORE);
	// cout<<"rc"<<rc<<" errno " << errno <<endl; assert (rc == 0);
	socket->send((unsigned char*)data,size);//,ZMQ_SNDMORE);

	// socket->recv(buffer_in,sizeof(CMD_SUCCESS));
	// cout<<"send"<<size<<endl;

	commObject.incrementSent(size);

	return true;
}

bool BmrNet::receiveMsg(void* buff, int size, int conn){

	// unsigned char buffer_in[sizeof(CMD_SUCCESS)]="OK";
	// int left = size;
	// int n;
	// memset(buff, 0, (unsigned long)size);
	// assert (socket);
	// int rc = zmq_recv (socket, buff, size, 1); 
	// cout<<"rc"<<rc<<"errno "<<errno <<endl; assert (rc == 0);
	socket->recv(buff,size);
	// socket->send(buffer_in,sizeof(CMD_SUCCESS));
	// cout<<"recv"<<size<<endl;

	commObject.incrementRecv(size);

	return true;
}