

#include "connect.h"
#include <thread>
#include <mutex> 
#include "secCompMultiParty.h"
#include <vector>
using namespace std;

#define STRING_BUFFER_SIZE 256
extern void error(string str);


//this player number
extern int partyNum;

//communication
string * addrs;
BmrNet ** communicationSenders;
BmrNet ** communicationReceivers;

//Communication measurements object
extern CommunicationObject commObject;

// extern float LAN_ping;
// extern float LAN_Com;
// extern float WAN_ping;
// extern float WAN_Com;

float LAN_ping = 0.0002;
float LAN_Com = 625000000;
float WAN_ping = 0.07;
float WAN_Com = 40000000;


double diff_c(timespec start, timespec end)
{
    timespec temp;

    if ((end.tv_nsec-start.tv_nsec)<0)
    {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else 
    {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp.tv_sec + (double)temp.tv_nsec/1E9;
}

//setting up communication
void initCommunication(string addr, int port, int player, int mode)
{
	

}

void initializeCommunication(int* ports)
{
	int i;
	communicationSenders = new BmrNet*[NUM_OF_PARTIES];
	communicationReceivers = new BmrNet*[NUM_OF_PARTIES];
	thread *threads = new thread[NUM_OF_PARTIES * 2];
	for (i = 0; i < NUM_OF_PARTIES; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2 + 1] = thread(initCommunication, addrs[i], ports[i * 2 + 1], i, 0);
			threads[i * 2] = thread(initCommunication, "127.0.0.1", ports[i * 2], i, 1);
		}
	}
	for (int i = 0; i < 2 * NUM_OF_PARTIES; i++)
	{
		if (i != 2 * partyNum && i != (2 * partyNum + 1))
			threads[i].join();//wait for all threads to finish
	}

	delete[] threads;
}

void initializeCommunicationSerial(int* ports)//Use this for many parties
{
	communicationSenders = new BmrNet*[NUM_OF_PARTIES];
	communicationReceivers = new BmrNet*[NUM_OF_PARTIES];
	for (int i = 0; i < NUM_OF_PARTIES; i++)
	{
		if (i == partyNum)
			continue;
		char temp[25];
		strcpy(temp, addrs[i].c_str());
		if(i == nextParty(partyNum)){
			communicationSenders[i] = new BmrNet(temp, ports[i * 2 ]);
			communicationSenders[i]->connectNow();
			communicationReceivers[i] = new BmrNet(ports[partyNum * 2 + 1]);
			communicationReceivers[i]->listenNow();

		}
		if(i == prevParty(partyNum)){
			communicationSenders[i] = new BmrNet(temp, ports[i * 2 + 1]);
			communicationSenders[i]->connectNow();
			communicationReceivers[i] = new BmrNet(ports[partyNum * 2 ]);
			communicationReceivers[i]->listenNow();

		}
		
	}
}

void initializeCommunication(char* filename, int p)
{
	FILE * f = fopen(filename, "r");
	partyNum = p;
	char buff[STRING_BUFFER_SIZE];
	char ip[STRING_BUFFER_SIZE];
	
	addrs = new string[NUM_OF_PARTIES];
	int * ports = new int[NUM_OF_PARTIES * 2];


	for (int i = 0; i < NUM_OF_PARTIES; i++)
	{
		fgets(buff, STRING_BUFFER_SIZE, f);
		sscanf(buff, "%s\n", ip);
		addrs[i] = string(ip);
		//cout << addrs[i] << endl;
		ports[2 * i] = 32000 + i * 2;
		ports[2 * i + 1] = 32000 + i * 2 + 1;
	}

	fclose(f);
	initializeCommunicationSerial(ports);

	delete[] ports;
}


//synchronization functions
void sendByte(int player, char* toSend, int length, int conn)
{
	communicationSenders[player]->sendMsg(toSend, length, conn);
	// totalBytesSent += 1;
}

void receiveByte(int player, int length, int conn)
{
	char *sync = new char[length+1];
	communicationReceivers[player]->receiveMsg(sync, length, conn);
	delete[] sync;
	// totalBytesReceived += 1;
}

void synchronize(int length)
{
	char* toSend = new char[length+1];
	memset(toSend, '0', length+1);
	vector<thread *> threads;
	for (int i = 0; i < NUM_OF_PARTIES; i++)
	{
		if (i == partyNum) continue;
		for (int conn = 0; conn < NUMCONNECTIONS; conn++)
		{
			threads.push_back(new thread(sendByte, i, toSend, length, conn));
			threads.push_back(new thread(receiveByte, i, length, conn));
		}
	}
	for (vector<thread *>::iterator it = threads.begin(); it != threads.end(); it++)
	{
		(*it)->join();
		delete *it;
	}
	threads.clear();
	delete[] toSend;
}


void start_communication()
{
	if (commObject.getMeasurement())
		error("Nested communication measurements");

	commObject.reset();
	commObject.setMeasurement(true);
}

void pause_communication()
{
	if (!commObject.getMeasurement())
		error("Communication never started to pause");

	commObject.setMeasurement(false);
}

void resume_communication()
{
	if (commObject.getMeasurement())
		error("Communication is not paused");

	commObject.setMeasurement(true);
}

void end_communication(string str,string fn)
{
	cout << "----------------------------------------------" << endl;
	cout << "Communication, " << str << ", P" << partyNum << ": " 
		 << (float)commObject.getSent()/1000000 << "MB (sent) " 
		 << (float)commObject.getRecv()/1000000 << "MB (recv)" << endl;
	cout << "Rounds, " << str << ", P" << partyNum << ": " 
		 << commObject.getRoundsSent() << "(sends) " 
		 << commObject.getRoundsRecv() << "(recvs)" << endl; 
	cout << "WAN comm time: " 
		 << (float)commObject.getSent()/WAN_Com +
		  (float)commObject.getRoundsSent()*WAN_ping  << endl; 
	cout << "LAN comm time: " 
		 << (float)commObject.getSent()/LAN_Com +
		  (float)commObject.getRoundsSent()*LAN_ping  << endl; 
	cout << "----------------------------------------------" << endl;	
	ofstream myfile;
	myfile.open (fn.c_str(),fstream::app);
	myfile << "----------------------------------------------" << endl;
	myfile << "Communication, " << str << ", P" << partyNum << ": " 
		 << (float)commObject.getSent()/1000000 << "MB (sent) " 
		 << (float)commObject.getRecv()/1000000 << "MB (recv)" << endl;
	myfile << "Rounds, " << str << ", P" << partyNum << ": " 
		 << commObject.getRoundsSent() << "(sends) " 
		 << commObject.getRoundsRecv() << "(recvs)" << endl; 
	myfile << "WAN comm time: " 
		 << (float)commObject.getSent()/WAN_Com +
		  (float)commObject.getRoundsSent()*WAN_ping  << endl; 
	myfile << "LAN comm time: " 
		 << (float)commObject.getSent()/LAN_Com +
		  (float)commObject.getRoundsSent()*LAN_ping  << endl; 
	myfile << "----------------------------------------------" << endl;
	myfile.close();
	commObject.reset();
}

void sendRoundfix(size_t player, size_t size)
{
	communicationSenders[player]->fixSend(size);
		
	
}

void receiveRoundfix(size_t player, size_t size)
{
	communicationReceivers[player]->fixReceive(size);
}