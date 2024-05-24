#pragma once
#include "CPU.h"
// #include "Functionalities.h"
using namespace std;
#include <sstream>
#include <fstream>

CPU::CPU(){
    AddTimes=Comptimes=MatMultimes=Gatetimes=ReceiveSize=0;
    log="";
}

void CPU::Reset(){
    AddTimes=Comptimes=MatMultimes=Gatetimes=ReceiveSize=0;
    log="";
}

void CPU::CPU_Add(int times){
    AddTimes+=times;
    std::ostringstream s;
    s<<times;
    log += "Add:"+s.str()+"\n";
}
void CPU::CPU_Gate(int times){
    Gatetimes+=times;
    std::ostringstream s;
    s<<times;
    log += "Gate:"+s.str()+"\n";
}

void CPU::CPU_Mul(int times){
    MatMultimes+=times;
    std::ostringstream s;
    s<<times;
    log += "Mul:"+s.str()+"\n";
}
void CPU::CPU_Comp(int times){
    Comptimes+=times;
    std::ostringstream s;
    s<<times;
    log += "Comp:"+s.str()+"\n";
}

void CPU::CPU_Recv(int size){
    ReceiveSize+=size;
    std::ostringstream s;
    s<<size;
    log += "Recv:"+s.str()+"\n";
}
void CPU::Write(string filename){
    ofstream myfile;
    myfile.open (filename.c_str(),fstream::app);
    myfile<<AddTimes<< endl <<Comptimes<< endl <<MatMultimes<< endl <<Gatetimes<< endl <<ReceiveSize<<endl;
    myfile<< log;
}

CPU cpu;