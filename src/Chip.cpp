#pragma once
#include "Chip.h"
// #include "Functionalities.h"
using namespace std;

#include<iostream>

#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <string.h>


// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <chrono>
using namespace std;

int
set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        // error_message ("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    // tty.c_iflag &= ~IGNBRK;     // disable break processing
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); 
    // Disable any special handling of received bytes
    tty.c_lflag = 0;        // no signaling chars, no echo,
                    // no canonical processing
    tty.c_oflag = 0;        // no remapping, no delays
    tty.c_cc[VMIN]  = 50;        // read doesn't block
    tty.c_cc[VTIME] = 50;        // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        // error_message ("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void
set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        // error_message ("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 50 : 0;
    tty.c_cc[VTIME] = 50;        // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
        // error_message ("error %d setting term attributes", errno);
    }
        
}





char *portname = "/dev/ttyACM0";
int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);

// const unsigned int data_len=  256*256*10; //256*10;//256*256*10;


// int Maximum_Read = 256;
// int step_read = int(chip_package_len/Maximum_Read);

// char data_send[data_len*4];
// char *data_char;


Chip::Chip(){
    counter_comm[0]=counter_comm[1]=counter_loc=0;

    AddTimes=AddBits=CompareTimes=CompareBits=AESTimes=AESBits=XORTimes=0;
    TransInBits=TransInTimes=TransOutBits=TransOutTimes=0;
    ReluTimes=Max_Comparetimes=SoftmaxTimes=LayerNormTimes=0;
    ClockTime=0;
    Simtime=0;
    AES_throughput=111.3e9;
    AES_latency=12.6e-6;
    Add_gate_latency=40;
    frequency=870e6;
    Adder_numbers=100;
    set_interface_attribs (fd, B230400, 0);  // set speed to 115,200 bps, 8n1 (no parity)

    set_blocking (fd, 1);     // set blocking
    // for(int i = 0;i<256;i++){
    //     Chip_mask3[i] = 0-Chip_mask1[i]-Chip_mask2[i];
    // }
    
    unsigned long int temp1=0;
    int q_temp;
    double m_d = exp(1);
    
    double temp;
	double ln2 = log(2);
    for(int i=0;i<4;i++){
        temp1=pow(2,31-FLOAT_PRECISION)*(i+1);
		temp = temp1/ln2;

		q_L[i]=floor(temp);
		temp = pow(2,temp-q_L[i]);
        
		m_L[i] = frexp(temp, &q_temp)*M_MAX;
    	q_L[i] = q_L[i]+q_temp;
    }
    // q_L_3 = q_L;
    // m_L= frexp(m_d, &q_temp)*M_MAX;
    // q_L = q_L+q_temp;

    
    // m_d=m_d/2*3;
    // m_L_3= frexp(m_d, &q_temp)*M_MAX;
    // q_L_3=q_L_3+q_temp+1;


    for (int i=0;i<chip_package_len;i++){
        Chip_mask3[i]= -Chip_mask1[i]-Chip_mask2[i];
    }
};

void Chip::set_party(int partyNum){
    chip_partyNum = partyNum;
    if(chip_partyNum==0 && ARDUINO){
        unsigned char a = 1;
        write (fd, &a, 1);
        cout<<"chip init complete"<<endl;
    }
}
void Chip::Reset(){
    counter_comm[0]=counter_comm[1]=counter_loc=0;

    AddTimes=AddBits=CompareTimes=CompareBits=AESTimes=AESBits=XORTimes=0;
    TransInBits=TransInTimes=TransOutBits=TransOutTimes=0;
    ReluTimes=Max_Comparetimes=SoftmaxTimes=LayerNormTimes=0;
    ClockTime=0;
    Simtime=0;
    unsigned char a = 1;
    if(ARDUINO){
        write (fd, &a, 1);
    }
    cout<<"chip reset complete"<<endl;
}
#if EMPTYRUN
void Chip::ChipGenMask(const vector<myType>& x, vector<myType>& x_dot, size_t size){
    int cbegin=clock();
    // TransInBits+=sizeof(myType)*size;
    // TransInTimes+=1;
    TransOutBits+=sizeof(myType)*size;
    TransOutTimes+=1;
    myType mask;
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    counter_comm[0]+=size;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+size/100*Add_gate_latency/frequency;
    x_dot = x;
}

void Chip::ChipGenMask(size_t size){
    int cbegin=clock();
    // TransInBits+=sizeof(myType)*size;
    // TransInTimes+=1;
    TransOutBits+=sizeof(myType)*size;
    TransOutTimes+=1;
    myType mask;
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    counter_comm[0]+=size;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+size/100*Add_gate_latency/frequency;
}

void Chip::ChipGenMask(const vector<longType>& x, vector<longType>& x_dot, size_t size){
    int cbegin=clock();
    // TransInBits+=sizeof(myType)*size;
    // TransInTimes+=1;
    TransOutBits+=sizeof(longType)*size;
    TransOutTimes+=1;
    longType mask;
    AESTimes+=size;
    AESBits+=sizeof(longType)*size;
    AddTimes+=size;
    AddBits+=sizeof(longType)*size;
    counter_comm[0]+=size;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(longType)*size/AES_throughput+AES_latency+100/frequency+size/100*Add_gate_latency/frequency;
    x_dot = x;
}

void Chip::ChipGenMask(const vector<smallType>& x, vector<smallType>& x_dot, size_t size){
    int cbegin=clock();
    // TransInBits+=sizeof(smallType)*size;
    // TransInTimes+=1;
    TransOutBits+=sizeof(smallType)*size;
    TransOutTimes+=1;
    smallType mask;
    AESTimes+=size;
    AESBits+=sizeof(smallType)*size;
    // AddTimes+=size;
    AddBits+=sizeof(smallType)*size;
    counter_comm[0]+=size;
    ClockTime+=clock()-cbegin;
    // Simtime += sizeof(smallType)*size/AES_throughput+AES_latency+100/frequency+size/100*Add_gate_latency/frequency;
}


// void Chip::ChipGenShare(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& x_dot, size_t size){
//     int cbegin=clock();
//     TransInBits+=(sizeof(myType)+sizeof(RSSVectorMyType))*size;
//     TransInTimes+=1;
//     TransOutBits+=(sizeof(RSSVectorMyType))*size;
//     TransOutTimes+=1;
//     //RSSVectorMyType mask(size);
//     AESTimes+=size*2;
//     AESBits+=sizeof(myType)*size*2;
//     AddTimes+=size*2;
//     AddBits+=sizeof(myType)*size*2;
//     counter_comm[0]+=size*2;
//     counter_comm[1]+=size*2;
//     ClockTime+=clock()-cbegin;
//     Simtime += 2*sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+2*size/100*Add_gate_latency/frequency;
// }

// void Chip::ChipGenShare(const vector<smallType>& x, const RSSVectorSmallType& x_share, RSSVectorSmallType& x_dot, size_t size){
//     int cbegin=clock();
//     TransInBits+=(sizeof(smallType)+sizeof(RSSSmallType))*size;
//     TransInTimes+=1;
//     TransOutBits+=(sizeof(RSSSmallType))*size;
//     TransOutTimes+=1;
//     // RSSSmallType mask(size);
//     AESTimes+=size*2;
//     AESBits+=sizeof(smallType)*size*2;
//     // AddTimes+=size*2;
//     AddBits+=sizeof(smallType)*size*2;
//     counter_comm[0]+=size*2;
//     counter_comm[1]+=size*2;
//     ClockTime+=clock()-cbegin;
//     // Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+size/100*Add_gate_latency/frequency;
// }

void Chip::ChipPC(const vector<myType>& x, const RSSVectorMyType& x_share, const vector<myType>& c,  vector<smallType>& b, 
                        size_t size){
    int cbegin=clock();
                            
    TransInBits+=(sizeof(myType)*3)*size;
    TransInTimes+=1;
    TransOutBits+=(sizeof(smallType)+sizeof(myType))*size;
    TransOutTimes+=1;
    // RSSVectorMyType sharemask(size);
    //recover secret
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size*3;
    AddBits+=sizeof(myType)*size*3;
    //Do ReLU
    AESTimes+=size*2;
    AESBits+=sizeof(myType)*size*2;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    CompareTimes+=size;
    XORTimes+=size;

    counter_comm[0]+=size;
    counter_loc+=size*2;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+4*size/100*Add_gate_latency/frequency;
}

void Chip::ChipPC(const vector<smallType>& x, const RSSVectorSmallType& x_share, const vector<smallType>& c, vector<smallType>& b, 
                        size_t size){

    int cbegin=clock();
    TransInBits+=(sizeof(smallType)*3)*size;
    TransInTimes+=1;
    TransOutBits+=(2*sizeof(smallType))*size;
    TransOutTimes+=1;
    // RSSSmallType sharemask(size);
    //recover secret
    AESTimes+=size;
    AESBits+=sizeof(smallType)*size;
    // AddTimes+=size*3;
    AddBits+=sizeof(smallType)*size*3;
    //Do ReLU
    AESTimes+=size*2;
    AESBits+=sizeof(smallType)*size*2;
    // AddTimes+=size;
    AddBits+=sizeof(smallType)*size;
    CompareTimes+=size;
    XORTimes+=size;

    counter_comm[0]+=size;
    counter_loc+=size*2;
    ClockTime+=clock()-cbegin;
    // Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+4*size/100*Add_gate_latency/frequency;
}


void Chip::ChipReLU(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& y, RSSVectorSmallType& b,
 size_t size){
    TransInBits+=(sizeof(myType))*size;
    TransInTimes+=1;
    TransOutBits+=(sizeof(smallType)+sizeof(myType)*2)*size;
    TransOutTimes+=1;
    ReluTimes+=size;
    if(ARDUINO){
        unsigned int data_len = size;
        unsigned char a = 2;
        write (fd, &a, 1);
        write (fd, &data_len, 4);

        myType data[size];
        myType data_return1[size],data_return2[size];
        smallType data_return3[size],data_return4[size];

        for(int i = 0; i < size; i++){
            data[i] = x[i]+x_share[i].first+x_share[i].second;
        }

        long wirte_time=0, read_time=0, total_time = 0;
        // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // std::chrono::steady_clock::time_point begin_write, end_write, begin_read, end_read, begin_total, end_total;
        wirte_time = 0;

        unsigned int x;
        char tag_tmp = 'a';
        for(int i = 0 ; i < double(data_len)/chip_package_len ; i++) {
            // cout<<"???"<<endl;
            // begin_total = std::chrono::steady_clock::now();
            if((i+1)*chip_package_len<=data_len){
                // cout<<"sending"<<endl;
                // begin_write = std::chrono::steady_clock::now();
                write (fd, &data[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
                // write (fd, &data_send[i*chip_package_len*4], sizeof(unsigned int)*chip_package_len);
                // end_write = std::chrono::steady_clock::now();
                // wirte_time += std::chrono::duration_cast<std::chrono::nanoseconds> (end_write - begin_write).count();
                // begin_read = std::chrono::steady_clock::now();
                //  cout<<"receiving"<<endl;
                // for(int j = 0;j<step_read;j++){
                //         read (fd, &data_return[i*chip_package_len+j*Maximum_Read], sizeof(unsigned int)*Maximum_Read);              
                // }
                // cout<<"writen";
                read (fd, &data_return1[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
                // cout<<"reand1";
                // write (fd, &tag_tmp, sizeof(char));
                read (fd, &data_return2[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
                // cout<<"reand2";
                // read (fd, &data_return[i*chip_package_len], sizeof(unsigned int)*int(chip_package_len/2));
                
                // end_read = std::chrono::steady_clock::now();

                // read_time += std::chrono::duration_cast<std::chrono::milliseconds> (end_read - begin_read).count();
                // for(int j = 0; j < chip_package_len; j++){
                //     // x = data_return1[i*chip_package_len+j]+data_return2[i*chip_package_len+j]-Chip_mask1[j]-Chip_mask2[j];

                //     // if(((data[i*chip_package_len+j]>>31)?(unsigned int)0:data[i*chip_package_len+j])!=x){
                //     if(1){
                //     std::cout<<data[i*chip_package_len+j]<<" "<<data_return1[i*chip_package_len+j]<<" ";
                //     std::cout<<data_return2[i*chip_package_len+j]<<" "<<Chip_mask1[j]<<" ";
                //     cout<<Chip_mask2[j]<<" ";//<<x;
                //     cout<<" "<< i <<" "<<j<<" ";
                //     cout<<endl;
                //     // cout<<x<<endl;
                //     }
                
                read (fd, &data_return3[i*chip_package_len], sizeof(unsigned short int)*chip_package_len);
                // cout<<"reand1";
                // write (fd, &tag_tmp, sizeof(char));
                read (fd, &data_return4[i*chip_package_len], sizeof(unsigned short int)*chip_package_len);
                
                // }//TODO: What is wrong with this?????????????
                // cin.get();

            }
            else{
                // begin_write = std::chrono::steady_clock::now();
                write (fd, &data[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                // write (fd, &data_send[i*chip_package_len*4], sizeof(unsigned int)*(data_len-i*chip_package_len));
                // end_write = std::chrono::steady_clock::now();
                // wirte_time += std::chrono::duration_cast<std::chrono::nanoseconds> (end_write - begin_write).count();
                // begin_read = std::chrono::steady_clock::now();
                read (fd, &data_return1[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                // write (fd, &tag_tmp, sizeof(char));
                read (fd, &data_return2[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                // end_read = std::chrono::steady_clock::now();
                // read_time += std::chrono::duration_cast<std::chrono::milliseconds> (end_read - begin_read).count();
                read (fd, &data_return3[i*chip_package_len], sizeof(unsigned short int)*(data_len-i*chip_package_len));
                read (fd, &data_return4[i*chip_package_len], sizeof(unsigned short int)*(data_len-i*chip_package_len));
            }
            // end_total = std::chrono::steady_clock::now();
            // total_time += std::chrono::duration_cast<std::chrono::milliseconds> (end_total - begin_total).count();
            // if (std::chrono::duration_cast<std::chrono::milliseconds> (end_read - begin_read).count()<0){
            //     cout<<"read time wrong"<< endl;
            // }
            // if (std::chrono::duration_cast<std::chrono::milliseconds> (end_write - begin_write).count()<0){
            //     cout<<"write time wrong"<< endl;
            // }
            // if (i){
            //     cout<<i<< endl;
            // }
            
        }

        for(int i = 0 ; i < data_len; i++){
            y[i].first=data_return1[i];
            y[i].second = data_return2[i];
        }
    
    }

    else{
        myType x_temp;
        smallType b_temp;
        for(int i = 0 ; i < size; i++){
            x_temp = x[i]+x_share[i].first+x_share[i].second;

            b_temp = getMSB(x_temp);
            b[i].first=b_temp^1;
            b[i].second = Chip_mask3[i%chip_package_len]%2;
            y[i].first=x_temp*(1-b_temp)+Chip_mask1[i%chip_package_len];
            y[i].second = Chip_mask2[i%chip_package_len];
        }
    }

    int cbegin=clock();
    
    // TransInBits+=(sizeof(myType)*2)*size;
    // TransInTimes+=1;
    // TransOutBits+=(sizeof(myType)+sizeof(smallType))*size;
    // TransOutTimes+=1;
    // RSSVectorMyType sharemask(size);
    //recover secret
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size*3;
    AddBits+=sizeof(myType)*size*3;
    //Do ReLU
    AESTimes+=size*2;
    AESBits+=sizeof(myType)*size*2;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    CompareTimes+=size;
    XORTimes+=size;

    counter_comm[0]+=size;
    counter_loc+=size*2;
    ClockTime+=clock()-cbegin;
    Simtime += 3*sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+4*size/100*Add_gate_latency/frequency;
}

void Chip::ChipReLUP(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorSmallType& b, size_t size){
    RSSVectorMyType y(size);
    ChipReLU(x, x_share, y, b,size);
    TransOutBits-=(sizeof(myType))*size;
}

void Chip::ChipSoftmax(const vector<longType> q,const vector<longType> m_star,
const vector<longType> m_3, RSSVectorMyType& y, size_t rows, size_t columns, bool masked, size_t mask_ind){
    
    int cbegin=clock();
    size_t size = rows*columns;
    TransInBits+=(sizeof(longType)*3)*size;
    TransInTimes+=1;
    TransOutBits+=(sizeof(myType)*2)*size;
    TransOutTimes+=1;
    SoftmaxTimes+=size;
    if (columns*6>96*1024){ //larger than arduino buffer
        TransInBits+=(sizeof(myType)*1)*size;
        TransInTimes+=1;
        TransOutBits+=(sizeof(myType)*1)*size;
        TransOutTimes+=1;
    }
    multimes+=2*size;
    int input_count = chip_package_len-chip_package_len%columns;
    if (input_count == 0){
		input_count = chip_package_len;
	}
    int rows_apack = (int)(input_count/columns);
    if(ARDUINO){
        unsigned char a = 4;
        write (fd, &a, 1);
        int c_in = (int)columns;
        int r_in = (int)rows;
        write (fd, &c_in, 4);
        write (fd, &r_in, 4);
        vector<myType> data_return1(size),data_return2(size);

        for(int i = 0 ; i < double(rows)/input_count ; i++) {
            if((i+1)*input_count<=rows){
                write (fd, &q[i*input_count], sizeof(longType)*input_count);
                write (fd, &m_star[i*input_count], sizeof(longType)*input_count);
                write (fd, &m_3[i*input_count], sizeof(longType)*input_count);
                // cout<<"1";
                read (fd, &data_return1[i*rows_apack], sizeof(myType)*rows_apack);
                // cout<<"2";
                read (fd, &data_return2[i*rows_apack], sizeof(myType)*rows_apack);
                // cout<<"3";
            }
            else {
                write (fd, &q[i*input_count], sizeof(longType)*(size-i*input_count));
                write (fd, &m_star[i*input_count], sizeof(longType)*(size-i*input_count));
                write (fd, &m_3[i*input_count], sizeof(longType)*(size-i*input_count));

                read (fd, &data_return1[i*rows_apack], sizeof(myType)*(size-i*rows_apack));
                read (fd, &data_return2[i*rows_apack], sizeof(myType)*(size-i*rows_apack));
            }
            if (i) {
                // cout<<i<< endl;
            }
        }

        for(int i = 0 ; i < rows; i++){
            y[i].first=data_return1[i];
            y[i].second = data_return2[i];
        }
    }
    else {
        double m_temp;
        double* m_e = new double[size];  
        unsigned int* results1 = new unsigned int[size];  
        unsigned int* results2 = new unsigned int[size];  
        long int* q_e = new long int[size]; 
        long int q_temp;
        int flag1;
        double pow52=pow(2,52);
        if(masked){
            for (int k = 0; k < rows; k++) {
                for (int j = 0;j<=mask_ind+k;j++){
                    int i = k*columns+j;
                    m_temp = (double)m_star[i]*(double)m_3[i]/pow52;
                    if((q[i]>q_L[2]) || ((q[i]==q_L[2])&&(m_temp>m_L[2]))){
                        flag1=2;
                        q_e[i] = (long int)q[i] - (long int)q_L[3]; 
                    }
                    else if ((q[i]>q_L[0]) || ((q[i]==q_L[0])&&(m_temp>m_L[0]))){
                        flag1=1;
                        q_e[i] = (long int)q[i] - (long int)q_L[1];
                    }
                    else{
                        q_e[i] = q[i];
                        flag1=0;
                    }

                    if(flag1==2)
                        m_e[i] = (double)m_star[i]*(double)m_3[i]/m_L[1]/m_L[1];
                    else if (flag1)
                        m_e[i] = (double)m_star[i]*(double)m_3[i]/m_L[1]/pow52 ;
                    else
                        m_e[i] = (double)m_star[i]*(double)m_3[i]/pow52/pow52;
                
                }
            }
            for (int i = 0; i < rows; i++) {
                double sum = 0;
                double temp_m[columns];
                for (int j = 0;j<mask_ind+i;j++){
                    temp_m[j]=m_e[i*columns+j]*pow(2,(q_e[i*columns+j]));
                    sum+=temp_m[j];

                }

                for (int j = 0; j < columns; j++){
                    if(j<=i+ mask_ind){
                        temp_m[j] = temp_m[j]/sum;
                        y[i*columns+j].first = (unsigned int)(temp_m[j]*pow(2,FLOAT_PRECISION))+Chip_mask1[(i*columns+j)%input_count];
                        y[i*columns+j].second = Chip_mask2[(i*columns+j)%input_count];
                    }
                    else{
                        y[i*columns+j].first = Chip_mask1[(i*columns+j)%input_count];
                        y[i*columns+j].second = Chip_mask2[(i*columns+j)%input_count];
                    }
                    
                }

            }
        }
        else{

            for(int i = 0; i < size; i++){

                m_temp = (double)m_star[i]*(double)m_3[i]/pow52;
                if((q[i]>q_L[2]) || ((q[i]==q_L[2])&&(m_temp>m_L[2]))){
                    flag1=2;
                    q_e[i] = (long int)q[i] - (long int)q_L[3]; 
                }
                else if ((q[i]>q_L[0]) || ((q[i]==q_L[0])&&(m_temp>m_L[0]))){
                    flag1=1;
                    q_e[i] = (long int)q[i] - (long int)q_L[1];
                }
                else{
                    q_e[i] = q[i];
                    flag1=0;
                }

                if(flag1==2)
                    m_e[i] = (double)m_star[i]*(double)m_3[i]/m_L[1]/m_L[1];
                else if (flag1)
                    m_e[i] = (double)m_star[i]*(double)m_3[i]/m_L[1]/pow52 ;
                else
                    m_e[i] = (double)m_star[i]*(double)m_3[i]/pow52/pow52;
            
                
            }
            for (int i = 0; i < rows; i++) {
                double sum = 0;
                double temp_m[columns];

                for (int j = 0;j<columns;j++){
                    temp_m[j]=m_e[i*columns+j]*pow(2,(q_e[i*columns+j]));
                    sum+=temp_m[j];
                    
                }

                for (int j = 0; j < columns; j++){
                    temp_m[j] = temp_m[j]/sum;

                    y[i*columns+j].first = (unsigned int)(temp_m[j]*pow(2,FLOAT_PRECISION))+Chip_mask1[(i*columns+j)%input_count];
                    y[i*columns+j].second = Chip_mask2[(i*columns+j)%input_count];
                }

            }
        }
        delete[] m_e,results1,results2,q_e;
        
    }
}
void Chip::ChipLayerNorm(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& y, 
                        size_t rows, size_t columns){
    int cbegin=clock();
    size_t size = rows*columns;

    LayerNormTimes+=size;
    TransInBits+=(sizeof(myType)*1)*size;
    TransInTimes+=1;
    TransOutBits+=(sizeof(myType)*2)*size;
    TransOutTimes+=1;
    multimes+=2*size;
    int input_count = chip_package_len-chip_package_len%columns;
    int rows_apack = (int)(input_count/columns);
    if(ARDUINO){
        unsigned int data_len = size;
        unsigned char a = 5;
        write (fd, &a, 1);
        write (fd, &data_len, 4);
        myType data[size];
        myType data_return1[size],data_return2[size];
        smallType data_return3[size],data_return4[size];
        for(int i = 0; i < size; i++){
            data[i] = x[i]+x_share[i].first+x_share[i].second;
        }
        long wirte_time=0, read_time=0, total_time = 0;
        wirte_time = 0;
        unsigned int x;
        char tag_tmp = 'a';
        for(int i = 0 ; i < double(data_len)/chip_package_len ; i++) {
            if((i+1)*chip_package_len<=data_len){
                write (fd, &data[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
                read (fd, &data_return1[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
                read (fd, &data_return2[i*chip_package_len], sizeof(unsigned int)*chip_package_len);
            }
            else{
                // begin_write = std::chrono::steady_clock::now();
                write (fd, &data[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                read (fd, &data_return1[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                read (fd, &data_return2[i*chip_package_len], sizeof(unsigned int)*(data_len-i*chip_package_len));
                
            }

            
        }

        for(int i = 0 ; i < data_len; i++){
            y[i].first=data_return1[i];
            y[i].second = data_return2[i];
        }
    
    }
    else {
        double sum,sigma;
        double results[columns];
        smallType b_temp;
        for(int i = 0 ; i < rows; i++){
            sum=0;
            sigma = 0;

            for(int j = 0; j< columns; j++){
                results[j] = (static_cast<int32_t>(x[i*columns+j]+x_share[i*columns+j].first+x_share[i*columns+j].second))/(float)(1 << 13);
                sum+=results[j];
            }
            sum = sum/columns;
            for(int j = 0; j< columns; j++){
                results[j] = results[j]-sum;
                sigma += pow(results[j],2);
            }
            sigma = sigma/columns;
            sigma = sqrt(sigma);
            for(int j = 0; j< columns; j++){
                y[i*columns+j].first = floatToMyType((results[j])/sigma)+Chip_mask1[(i*columns+j)%input_count];
                y[i*columns+j].second = Chip_mask2[(i*columns+j)%input_count];
            }


        }

        
        
    }
}

void Chip::ChipMax(const vector<myType>& x, const RSSVectorMyType &x_share, RSSVectorMyType &max, RSSVectorSmallType &maxPrime,
                          size_t rows, size_t columns){

    int cbegin=clock();
    size_t size = rows*columns;
    Max_Comparetimes+=size;
    TransInBits+=(sizeof(myType)*1)*size;
    TransInTimes+=1;
    TransOutBits+=(sizeof(RSSVectorSmallType)+sizeof(myType))*rows;
    TransOutTimes+=1;
    int input_count = chip_package_len-chip_package_len%columns;
    if (input_count == 0){
		input_count = chip_package_len;
	}
    int rows_apack = (int)(input_count/columns);
    if(ARDUINO){
        unsigned char a = 3;
        write (fd, &a, 1);
        int c_in = (int)columns;
        int r_in = (int)rows;
        write (fd, &c_in, 4);
        write (fd, &r_in, 4);    
        myType data[size];
        for(size_t i = 0; i < size; i++){
                data[i] = x[i]+x_share[i].first+x_share[i].second;
                // cout<<i<<endl;
        }
        vector<myType> data_return1(size),data_return2(size);
        // cout<<"begin trans 2"<<endl;
        for(int i = 0 ; i < double(rows)/input_count ; i++) {
        	if((i+1)*input_count<=rows){
                write (fd, &data[i*input_count], sizeof(myType)*input_count);
                // cout<<"1";
                read (fd, &data_return1[i*rows_apack], sizeof(myType)*rows_apack);
                // cout<<"2";
                read (fd, &data_return2[i*rows_apack], sizeof(myType)*rows_apack);
                // cout<<"3";
            }
            else{
                write (fd, &data[i*input_count], sizeof(unsigned int)*(size-i*input_count));
                read (fd, &data_return1[i*rows_apack], sizeof(unsigned int)*(size-i*rows_apack));
                read (fd, &data_return2[i*rows_apack], sizeof(unsigned int)*(size-i*rows_apack));
            }

            if (i){
                // cout<<i<< endl;
            }                
        }

        for(int i = 0 ; i < rows; i++){
            max[i].first=data_return1[i];
            max[i].second = data_return2[i];
        }
    }
    else {   
        
        for(int i = 0 ; i < rows; i++){
            myType x_temp,x_max;
            int b_temp,b_max;
            x_max = x[i*columns]+x_share[i*columns].first+x_share[i*columns].second;
            b_max = i*columns;
            for(int j = 1 ; j < columns; j++){
                x_temp = x[i*columns+j]+x_share[i*columns+j].first+x_share[i*columns+j].second;
                if ((x_temp>x_max &&(getMSB(x_temp)<=getMSB(x_max)))||
                    (getMSB(x_temp)<getMSB(x_max))){
                    x_max = x_temp;
                    b_max = i*columns+j;
                }
                maxPrime[i*columns+j].first = 0;
                maxPrime[i*columns+j].second = 0;
            }
            // std::cout<<" max: "<<x_max<<"?"<<b_max<<endl;
            max[i].first=x_max+Chip_mask1[i%chip_package_len];
            max[i].second = Chip_mask2[i%chip_package_len];
            maxPrime[b_max].first=1;

        }
    }
    // TransInBits+=(sizeof(myType)*2)*size;
    // TransInTimes+=1;
    // TransOutBits+=(sizeof(myType)*2)*rows;
    // TransOutTimes+=1;
    // RSSVectorMyType sharemask(size);
    //recover secret
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size*3;
    AddBits+=sizeof(myType)*size*3;
    //Do Max
    AESTimes+=size*2;
    AESBits+=sizeof(myType)*size*2;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    CompareTimes+=size;
    XORTimes+=size;

    counter_comm[0]+=size;
    counter_loc+=size*2;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+size*Add_gate_latency/frequency;
}
void Chip::ChipMaxReLU(const vector<myType>& x, const vector<myType>& bias, RSSVectorMyType &y, RSSVectorSmallType &Prime, size_t truncation,
 						 size_t B, size_t tempSize, size_t Dout,bool maxpool, bool ReLU, bool BN, size_t poolSize, size_t stride_M, bool Is_CNN,
                          const vector<myType>&tempgamma, const vector<myType>&tempbeta){

    // assert(ReLU && "Must have relu");
    cout<<maxpool<<ReLU<<BN<<endl;
    int cbegin=clock();
    size_t size = B*tempSize*Dout;
    size_t rows = B;
    size_t columns = tempSize*Dout;
    size_t ow = sqrt(tempSize);
    size_t oh = ow;
    assert(oh*ow==tempSize && "Maxrelu not sqr figure size");
    Max_Comparetimes+=size;
    TransInBits+=(sizeof(myType)*1)*(x.size()+bias.size());
    TransInTimes+=1;
    TransOutBits+=(sizeof(smallType)*y.size()+sizeof(myType)*Prime.size());
    TransOutTimes+=1;
    int input_count = chip_package_len-chip_package_len%columns;
    if (input_count == 0){
		input_count = chip_package_len;
	}
    int rows_apack = (int)(input_count/columns);
    if(ARDUINO){
        
    }
    else {
        vector<myType> x_rec = x;
        dividePlain(x_rec, (1 << truncation));
        size_t sizeBeta = ow;
        size_t sizeD 	= sizeBeta*ow;
        size_t sizeB 	= sizeD*Dout;
        size_t counter 	= 0;
        size_t numPool  = floor(ow/stride_M+1)*floor(ow/stride_M+1);

        if(Is_CNN){
            for (int b = 0; b < B; ++b)
                for (size_t r = 0; r < Dout; ++r)
                    for (size_t beta = 0; beta < ow; beta++) 
                        for (size_t alpha = 0; alpha < ow; alpha++){
                            x_rec[b*sizeB + r*sizeD + beta *sizeBeta + alpha ]+=bias[r];
                        }
            // cout<<"acti "<<endl;
            // for (int b = 0; b < 2; ++b)
            //     for (size_t r = 0; r < Dout; ++r)
            //         for (size_t beta = 0; beta < ow; beta++) 
            //             for (size_t alpha = 0; alpha < ow; alpha++){
            //                 print_linear(x_rec[b*sizeB + r*sizeD + beta *sizeBeta + alpha ], "FLOAT");
            //             }
            // cout<<endl;
        }
        else{

            for (int b = 0; b < rows; ++b)
                for (size_t r = 0; r < columns; ++r){
                    // print_linear(x_rec[b*Dout + r ], "FLOAT");
                    x_rec[b*Dout + r ]+=bias[r]; 
                }
            // for (size_t r = 0; r < columns; ++r)
            //     print_linear(bias[r], "FLOAT");
            // cout<<"***********"<<endl;

        }
        if(maxpool&&ReLU){
            myType x_temp,x_max;
            size_t b_temp,b_max;//TODO Test if accurate
            int count = 0;
            for (int b = 0; b < B; ++b){
                size_t indB = b*sizeB;
                for (size_t r = 0; r < Dout; ++r){
                    size_t indr = r*sizeD;

                    for (size_t beta = 0; beta < ow-poolSize+1; beta+=stride_M) {
                        for (size_t alpha = 0; alpha < ow-poolSize+1; alpha+=stride_M){
                            x_max = 0;
                            b_max = 0;

                            for (int q = 0; q < poolSize; ++q){
							    for (int p = 0; p < poolSize; ++p){
                                    x_temp = x_rec[indB + indr + (beta+ q)*sizeBeta + (alpha + p)];

                                    if ((x_temp>x_max &&(getMSB(x_temp)<=getMSB(x_max)))||
                                        (getMSB(x_temp)<getMSB(x_max))){
                                        x_max = x_temp;
                                        b_temp = getMSB(x_temp);
                                        b_max = q*sizeBeta + p;
                                        
                                    }
                                        
                                    }
                                }
                            int bInd = indB + indr + (size_t)((beta*sizeBeta + alpha)/stride_M);
                            int i = indr + (size_t)((beta*sizeBeta + alpha)/stride_M);
                            // cout<<"BIND: "<<bInd<<endl;
                            // cout<<"b_max+(indB+indr)*numPool "<<b_max+(indB+indr)*numPool<<endl;
                            // cout<<x_max<<" * ";
                            y[count].first=x_max*(1-b_temp)+Chip_mask1[count%chip_package_len];
                            y[count].second = Chip_mask2[count%chip_package_len];
                            Prime[b_max+indB+indr].first=getMSB(x_max)==0;
                            Prime[b_max+indB+indr].second = Chip_mask3[count%chip_package_len]%2;
                            assert(count<y.size());
                            count++;
                            assert(b_max+indB+indr<Prime.size());
                            }
                        }
                    }
                // std::cout<<" max: "<<x_max<<"?"<<b_max<<endl;
            
            }

        }
        else if(maxpool&&BN){
            myType x_temp,x_max;
            size_t b_temp,b_max;//TODO Test if accurate
            int count = 0;
            vector<myType> y_temp(y.size());
            for (int b = 0; b < B; ++b){
                size_t indB = b*sizeB;
                for (size_t r = 0; r < Dout; ++r){
                    size_t indr = r*sizeD;

                    for (size_t beta = 0; beta < ow-poolSize+1; beta+=stride_M) {
                        for (size_t alpha = 0; alpha < ow-poolSize+1; alpha+=stride_M){
                            x_max = 0;
                            b_max = 0;

                            for (int q = 0; q < poolSize; ++q){
							    for (int p = 0; p < poolSize; ++p){
                                    x_temp = x_rec[indB + indr + (beta+ q)*sizeBeta + (alpha + p)];

                                    if ((x_temp>x_max &&(getMSB(x_temp)<=getMSB(x_max)))||
                                        (getMSB(x_temp)<getMSB(x_max))){
                                        x_max = x_temp;
                                        b_temp = getMSB(x_temp);
                                        b_max = q*sizeBeta + p;
                                        
                                    }
                                        
                                    }
                                }
                            int bInd = indB + indr + (size_t)((beta*sizeBeta + alpha)/stride_M);
                            int i = indr + (size_t)((beta*sizeBeta + alpha)/stride_M);
                            y_temp[count]=x_max;
                            }
                        }
                    }
                // std::cout<<" max: "<<x_max<<"?"<<b_max<<endl;
            
            }
            size_t m = y.size()/B; //can be wrong
            vector<float> mu(B,0),a_float(size,0),gamma_float(B,0),beta_float(B,0),sigma_2_float(B,0);
            for(int i = 0 ; i < B; i++){
                gamma_float[i] = (static_cast<int32_t>(tempgamma[i]))/(float)(1 << FLOAT_PRECISION);
                beta_float[i] = (static_cast<int32_t>(tempbeta[i]))/(float)(1 << FLOAT_PRECISION);
                for (int j = 0;j<m; j++){
                    a_float[i*m+j]=(static_cast<int32_t>(y_temp[i*m+j]))/(float)(1 << FLOAT_PRECISION);
                    mu[i]+=a_float[i*m+j];
                }
                mu[i] = mu[i] / m;
            }
            for(int i = 0 ; i < B; i++){
                for (int j = 0;j<m; j++){
                    sigma_2_float[i]+= pow(a_float[i*m+j]-mu[i],2);
                }
                sigma_2_float[i] = sigma_2_float[i]/m;
            }
            size_t EPSILON = (myType)(1 << (FLOAT_PRECISION - 8));
            float eps = (static_cast<int32_t>(EPSILON))/(float)(1 << FLOAT_PRECISION);

            for(int i = 0 ; i < B; i++){
                for (int j = 0;j<m; j++){
                    a_float[i*m+j]=gamma_float[i]*(a_float[i*m+j]-mu[i])/sqrt(sigma_2_float[i]+eps)+beta_float[i];
                    x_temp = floatToMyType(a_float[i*m+j]);//TODO check correct?;
                    y[i*m+j].first=x_temp+Chip_mask1[(i*m+j)%chip_package_len];
                    y[i*m+j].second = Chip_mask2[(i*m+j)%chip_package_len];
                }
                
            }
        }
        else if(BN){
            
            size_t m = columns; //can be wrong
            vector<float> mu(B,0),a_float(size,0),gamma_float(B,0),beta_float(B,0),sigma_2_float(B,0);
            for(int i = 0 ; i < B; i++){
                gamma_float[i] = (static_cast<int32_t>(tempgamma[i]))/(float)(1 << FLOAT_PRECISION);
                beta_float[i] = (static_cast<int32_t>(tempbeta[i]))/(float)(1 << FLOAT_PRECISION);
                for (int j = 0;j<m; j++){
                    a_float[i*m+j]=(static_cast<int32_t>(x_rec[i*m+j]))/(float)(1 << FLOAT_PRECISION);
                    mu[i]+=a_float[i*m+j];
                }
                mu[i] = mu[i] / m;
            }
            for(int i = 0 ; i < B; i++){
                for (int j = 0;j<m; j++){
                    sigma_2_float[i]+= pow(a_float[i*m+j]-mu[i],2);
                }
                sigma_2_float[i] = sigma_2_float[i]/m;
            }
            size_t EPSILON = (myType)(1 << (FLOAT_PRECISION - 8));
            float eps = (static_cast<int32_t>(EPSILON))/(float)(1 << FLOAT_PRECISION);
            myType x_temp;
            smallType b_temp;
            if(ReLU){
                for(int i = 0 ; i < B; i++){
                    for (int j = 0;j<m; j++){
                        a_float[i*m+j]=gamma_float[i]*(a_float[i*m+j]-mu[i])/sqrt(sigma_2_float[i]+eps)+beta_float[i];
                        x_temp = floatToMyType(a_float[i*m+j]);//TODO check correct?;
                        b_temp = getMSB(x_temp);
                        Prime[i*m+j].first=b_temp^1;
                        Prime[i*m+j].second = Chip_mask3[(i*m+j)%chip_package_len]%2;
                        y[i*m+j].first=x_temp*(1-b_temp)+Chip_mask1[(i*m+j)%chip_package_len];
                        y[i*m+j].second = Chip_mask2[(i*m+j)%chip_package_len];
                    }
                    
                }
            }
            
            
            else{
                for(int i = 0 ; i < B; i++){
                    for (int j = 0;j<m; j++){
                        a_float[i*m+j]=gamma_float[i]*(a_float[i*m+j]-mu[i])/sqrt(sigma_2_float[i]+eps)+beta_float[i];
                        x_temp = floatToMyType(a_float[i*m+j]);//TODO check correct?;
                        y[i*m+j].first=x_temp+Chip_mask1[(i*m+j)%chip_package_len];
                        y[i*m+j].second = Chip_mask2[(i*m+j)%chip_package_len];
                    }
                    
                }
            }

           
        }
        else if(ReLU){
 
            myType x_temp;
            smallType b_temp;
            for(int i = 0 ; i < size; i++){

                    x_temp = x_rec[i];//TODO check correct?;
                    b_temp = getMSB(x_temp);
                    Prime[i].first=b_temp^1;
                    Prime[i].second = Chip_mask3[i%chip_package_len]%2;
                    y[i].first=x_temp*(1-b_temp)+Chip_mask1[i%chip_package_len];
                    y[i].second = Chip_mask2[i%chip_package_len];
            

            }
           
        }

        else{
            error("wrong choice of non-linear operations");
        }
        
    }
    // TransInBits+=(sizeof(myType)*2)*size;
    // TransInTimes+=1;
    // TransOutBits+=(sizeof(myType)*2)*rows;
    // TransOutTimes+=1;
    // RSSVectorMyType sharemask(size);
    //recover secret
    AESTimes+=size;
    AESBits+=sizeof(myType)*size;
    AddTimes+=size*3;
    AddBits+=sizeof(myType)*size*3;
    //Do Max
    AESTimes+=size*2;
    AESBits+=sizeof(myType)*size*2;
    AddTimes+=size;
    AddBits+=sizeof(myType)*size;
    CompareTimes+=size;
    XORTimes+=size;

    counter_comm[0]+=size;
    counter_loc+=size*2;
    ClockTime+=clock()-cbegin;
    Simtime += sizeof(myType)*size/AES_throughput+AES_latency+100/frequency+size*Add_gate_latency/frequency;
    
}

void Chip::ChipBN(const RSSVectorMyType& a,const vector<myType>& recv_a,const RSSVectorMyType& gamma,const vector<myType>& recv_gamma,
                const RSSVectorMyType& beta, const vector<myType>& recv_beta, 
                RSSVectorMyType& b, size_t EPSILON, size_t m ,size_t B){
    
    int cbegin=clock();
    size_t size = m*B;
    Max_Comparetimes+=size;
    TransInBits+=(sizeof(myType))*(size*3+B*1);
    TransInTimes+=1;
    TransOutBits+=(sizeof(myType))*size*2;
    TransOutTimes+=1;
    multimes+=2*size;
    int input_count = chip_package_len-chip_package_len%m;
    if (input_count == 0){
		input_count = chip_package_len;
	}
    int rows_apack = (int)(input_count/m);
    vector<myType> a_plain(size,0),sigma_plain(B,0),gamma_plain(B,0),beta_plain(size,0);
    for(int i = 0 ; i < B; i++){
        gamma_plain[i] = gamma[i].first+gamma[i].second+recv_gamma[i];
        beta_plain[i] = beta[i].first+beta[i].second+recv_beta[i];
        for (int j = 0;j<m; j++){
            a_plain[i*m+j]=a[i*m+j].first+a[i*m+j].second+recv_a[i*m+j];
        }
    }
    if(ARDUINO){
        //TODO
    }
    else { 
        vector<float> mu(B,0),a_float(size,0),gamma_float(B,0),beta_float(B,0),sigma_2_float(B,0);
        for(int i = 0 ; i < B; i++){
            gamma_float[i] = (static_cast<int32_t>(gamma_plain[i]))/(float)(1 << FLOAT_PRECISION);
            beta_float[i] = (static_cast<int32_t>(beta_plain[i]))/(float)(1 << FLOAT_PRECISION);
            for (int j = 0;j<m; j++){
                a_float[i*m+j]=(static_cast<int32_t>(a_plain[i*m+j]))/(float)(1 << FLOAT_PRECISION);
                mu[i]+=a_float[i*m+j];
            }
            mu[i] = mu[i] / m;
        }
        for(int i = 0 ; i < B; i++){
            for (int j = 0;j<m; j++){
                sigma_2_float[i]+= pow(a_float[i*m+j]-mu[i],2);
            }
            sigma_2_float[i] = sigma_2_float[i]/m;
        }
        float eps = (static_cast<int32_t>(EPSILON))/(float)(1 << FLOAT_PRECISION);
        for(int i = 0 ; i < B; i++){
            for (int j = 0;j<m; j++){
                a_float[i*m+j]=gamma_float[i]*(a_float[i*m+j]-mu[i])/sqrt(sigma_2_float[i]+eps)+beta_float[i];
                b[i*m+j].first = floatToMyType(a_float[i*m+j])+Chip_mask1[(i*m+j)%input_count];
                b[i*m+j].second = Chip_mask2[(i*m+j)%input_count];
            }
            
        }


    }
}
#else
    void Chip::ChipGenMask(const vector<myType> & x, vector<myType>& x_dot, size_t size){}
    void Chip::ChipGenMask(const vector<smallType> & x, vector<smallType>& x_dot, size_t size){}
    void Chip::ChipGenMask(const vector<longType> & x, vector<longType>& x_dot, size_t size){}
    void Chip::ChipGenMask(size_t size){}
    void Chip::ChipGenShare(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& x_dot, size_t size){}
    void Chip::ChipGenShare(const vector<smallType>& x, const RSSVectorSmallType& x_share, RSSVectorSmallType& x_dot, size_t size){}
    void Chip::ChipPC(const vector<myType>& x, const RSSVectorMyType& x_share, const vector<myType>& c, vector<smallType>& b, 
                        size_t size){}
    void Chip::ChipPC(const vector<smallType>& x, const RSSVectorSmallType& x_share, const vector<smallType>& c, vector<smallType>& b, 
                        size_t size){}
    void Chip::ChipReLU(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& y, RSSVectorSmallType& b, 
                        size_t size){}
    void Chip::ChipReLUP(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorSmallType& b, 
                        size_t size){}
    void Chip::ChipMax(const vector<myType>& x, const RSSVectorMyType &x_share, RSSVectorMyType &max, RSSVectorSmallType &maxPrime,
 						 size_t rows, size_t columns){}
    void Chip::ChipMaxReLU(const vector<myType>& x, const vector<myType>& bias, RSSVectorMyType &max, RSSVectorSmallType &Prime, size_t truncation,
 						 size_t B, size_t tempSize, size_t Dout, bool maxpool, bool ReLU, bool BN, size_t poolSize, size_t stride_M, bool Is_CNN,
                          const vector<myType>&tempgamma, const vector<myType>&tempbeta){}
    void Chip::ChipSoftmax(const vector<longType> q,const vector<longType> m_star,const vector<longType> m_3, 
    RSSVectorMyType& y, size_t rows, size_t columns, bool masked, size_t mask_ind){}
    void Chip::ChipLayerNorm(const vector<myType>& x, const RSSVectorMyType& x_share, RSSVectorMyType& y, 
                        size_t rows, size_t columns){}
    void Chip::ChipBN(const RSSVectorMyType& a,const vector<myType>& recv_a,
                const RSSVectorMyType& gamma,const vector<myType>& recv_gamma,const RSSVectorMyType& beta, const vector<myType>& recv_beta, 
                RSSVectorMyType& b, size_t EPSILON, size_t m ,size_t B){}
#endif