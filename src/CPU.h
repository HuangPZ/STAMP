
#pragma once
#include <stdio.h> 
#include <iostream>
#include <string>
using namespace std;


// extern void log_print(string str);
class CPU
{
private:

public:
    // memset(counter_loc, 0, sizeof(counter_loc));


    int AddTimes,Comptimes,MatMultimes,Gatetimes,ReceiveSize;
    string log;

	//Constructor and initializer
	CPU();

	//Functions
    void Reset();
    void CPU_Gate(int times);
    void CPU_Add(int times);
    void CPU_Mul(int times);
    void CPU_Comp(int times);
    void CPU_Recv(int amount);
    void Write(string filename);
};

