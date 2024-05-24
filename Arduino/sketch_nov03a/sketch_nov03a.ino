#include <stdlib.h>     
#include <math.h> 
#define L 2147483648 //1<<31
#define fp 13
#define m_max 4503599627370496
#define m_max_half 67108864
// unsigned int p = 0;
// unsigned int intMaxModp = 0;
// unsigned int keyUpper,keyLower;
// unsigned int counterUpper, counterLower;


byte inbyte = 0;
int state = 0;
int substate = 0;
byte buffer_int32[4];
int read_time = 0, write_time = 0;
const int length_pack=256;
unsigned int data_in[length_pack];
unsigned int temp_pack1[length_pack],temp_pack2[length_pack];
unsigned long int data_in_long_1[length_pack],data_in_long_2[length_pack],data_in_long_3[length_pack];
unsigned short int data_short1[length_pack],data_short2[length_pack];
//uint8_t int temp_small1[length_pack],temp_small2[length_pack];
unsigned int mask1[length_pack],mask2[length_pack];
int columns,rows,rows_apack,input_count;

long int m_L = 0,m_L_3=0;
unsigned int q_L = 0,q_L_3=0;
    
//0: waiting for initialization
//1: Initialized
//2: ReLU
//3: Maxpooling
//4: Exp

void setup() {
  SerialUSB.begin (230400);
  while(!SerialUSB){}
//  SerialUSB.write("This state is :");
//  SerialUSB.println(state);
    int q_temp=0;
    double m_d = exp(1);
    for(int i =1;i<32-fp;i++){
        m_d = m_d*m_d;
        q_L=q_L*2;
        if(m_d>m_max_half){
            m_d = m_d/m_max_half;
            q_L = q_L+26;
        }
    }
    q_L_3 = q_L;
    m_L= frexp(m_d, &q_temp)*m_max;
    q_L = q_L+q_temp;

    
    m_d=m_d/2*3;
    m_L_3= frexp(m_d, &q_temp)*m_max;
    q_L_3=q_L_3+q_temp+1;

}

void loop() {
  while(!SerialUSB.available());
  if (SerialUSB.available()) {
    switch(state){
      case 0:{// waiting for initialization
        inbyte = SerialUSB.read();
//        SerialUSB.println(inbyte);
        if(inbyte == 1){
          initial();
          state = 1;
//          SerialUSB.write("This state is :");
//          SerialUSB.println(state);
        }
        
        break;}
      case 1:{// Initialized
        inbyte = SerialUSB.read();
        if(inbyte == 2){//ReLU
            state = 2;
//            SerialUSB.write("This state is :");
//            SerialUSB.println(state);
          }
         else if (inbyte == 3){//Maxpooling
            state = 3;
//            SerialUSB.write("This state is :");
//            SerialUSB.println(state);
          }
          else if (inbyte == 4){//Exp
            state = 4;
          }
        break;}
      case 2:{
        unsigned int n = 1;
        int count_input = 0;
        while(n){
          if(substate){
            count_input = 0;
//            SerialUSB.println("reading");
            while(!SerialUSB.available()){}
//            for(;count_input<length_pack;count_input++){
//              n--;
//              SerialUSB.readBytes(buffer_int32,4);
//              data_in[count_input]=recover_int32();
//            }
            if(n>=length_pack){
              n-=length_pack;
              SerialUSB.readBytes((uint8_t*)data_in,length_pack*4);
//              unsigned int a = recover_int32();
//              SerialUSB.println("FINISH PACK");
//              int t0 = millis();
//              SerialUSB.println(n);
  
              ReLU(data_in,length_pack);
            }
            else{
              SerialUSB.readBytes((uint8_t*)data_in,n*4);
              ReLU(data_in,n);
              n=0;
            }

//            write_time += t0-millis();
          }

          else{
//            SerialUSB.println(n);
            SerialUSB.readBytes(buffer_int32,4);
            n=recover_int32();
//            SerialUSB.println(n);
            substate = 1;
          }

        }
//        for(int i=0;i<length_pack;i++){
//              SerialUSB.println(data_in[i]);
//            }
//        SerialUSB.write((uint8_t*)data_in,length_pack*sizeof(unsigned int));
        substate = 0;
        state = 1;
//        SerialUSB.println(write_time);
//        SerialUSB.println("finished");
        break;}
      case 3:{
        int n=1;
        int x[]={1,2,3,4};
        while(n){
            if(substate){
  //            SerialUSB.println("reading");
              while(!SerialUSB.available()){}
              if(n>=input_count){
                n-=input_count;
                SerialUSB.readBytes((uint8_t*)data_in,input_count*4);
  
    
                MaxPool(data_in,input_count,rows_apack);
              }
              else{
                SerialUSB.readBytes((uint8_t*)data_in,n*4);
                MaxPool(data_in,input_count,rows_apack);
                n=0;
              }
   
  //            write_time += t0-millis();
            }
  
            else{
              SerialUSB.readBytes(buffer_int32,4);
              columns=recover_int32();
              SerialUSB.readBytes(buffer_int32,4);
              rows=recover_int32();
              input_count = length_pack-length_pack%columns;
              rows_apack = (int)(input_count/columns);
              n = rows;
              SerialUSB.println(columns);
              SerialUSB.println(rows);
              substate = 1;
            }
  
          }

          substate = 0;
          state = 1;

          break;}
      case 4:{
          int n=1;
          int x[]={1,2,3,4};
          while(n){
            if(substate){
  //            SerialUSB.println("reading");
              while(!SerialUSB.available()){}
              if(n>=input_count){
                n-=input_count;
                SerialUSB.readBytes((uint8_t*)data_in_long_3,input_count*8);
                SerialUSB.readBytes((uint8_t*)data_in_long_1,input_count*8);
                SerialUSB.readBytes((uint8_t*)data_in_long_2,input_count*8);
    
                Exp(data_in_long_3,data_in_long_1,data_in_long_2,input_count,columns);
              }
              else{
                SerialUSB.readBytes((uint8_t*)data_in,n*8);
                Exp(data_in_long_3,data_in_long_1,data_in_long_2,n,columns);
                n=0;
              }
  
  //            write_time += t0-millis();
            }
  
            else{
              SerialUSB.readBytes(buffer_int32,4);
              rows=recover_int32();
              SerialUSB.readBytes(buffer_int32,4);
              columns=recover_int32();
              input_count = length_pack-length_pack%columns;
              rows_apack = (int)(input_count/columns);
              n = rows;
              substate = 1;
            }
  
          }

          substate = 0;
          state = 1;
          break;}
      
    }
  }
  

}

void reset(){

  state = 0;
}

void initial(){
  state = 0;
}

unsigned int Add_modp(unsigned int a, unsigned int b){
  if(a>p-b){
    return (a+b)%p+intMaxModp;
  }
  else{
    return (a+b)%p;
  }
}


void ReLU(unsigned int a[length_pack],int count_input){
    randomSeed(11);
    for(int i=0;i<length_pack;i++){
        mask1[i]=random(2147483648);//+2147483648;
        mask2[i]=random(2147483648);//+2147483648;
        if(random(2)){
          mask1[i]+=2147483648;
        }
        if(random(2)){
          mask2[i]+=2147483648;
        }
        if(!(a[i]>>31)){
          temp_pack1[i] = a[i]+mask1[i];
          temp_pack2[i] = mask2[i];
          data_short1[i] = 0;
          data_short2[i] = 1;
        }
        else{
          temp_pack1[i] = mask1[i];
          temp_pack2[i] = mask2[i];
          data_short1[i] = 1;
          data_short2[i] = 1;
        }
     }

    SerialUSB.write((uint8_t*)temp_pack1,count_input*sizeof(unsigned int));
//    SerialUSB.readBytes(buffer_int32,1);
    SerialUSB.write((uint8_t*)temp_pack2,count_input*sizeof(unsigned int));
    SerialUSB.write((uint8_t*)data_short1,count_input*sizeof(unsigned short int));
    SerialUSB.write((uint8_t*)data_short2,count_input*sizeof(unsigned short int));
  
}

void MaxPool(unsigned int a[],int count_input, int rows_apack){

    unsigned int maxtmp=2147483648;
    bool maxtmp_sign = 1;
    bool a_sign = 0;
    randomSeed(11);
    for(int j = 0; j<rows_apack;j++){
      mask1[j]=random(2147483648);//+2147483648;
      mask2[j]=random(2147483648);//+2147483648;
      if(random(2)){
        mask1[j]+=2147483648;
      }
      if(random(2)){
        mask2[j]+=2147483648;
      }
      for(int k = 0; k<columns;k++){
        a_sign = a[j*columns+k]>2147483648;
        if(maxtmp_sign){
          if((a[j*columns+k]>maxtmp)||!a_sign){
            maxtmp = a[j*columns+k];
            maxtmp_sign = a_sign;
          }
            
        }
        else{
          if((a[j*columns+k]>maxtmp)&&!a_sign){
        
            maxtmp = a[j*columns+k];
            maxtmp_sign = a_sign;
          }
        }
      }
      temp_pack1[j] = mask1[j]+maxtmp;
      temp_pack2[j] = mask2[j];
      
    }


    SerialUSB.write((uint8_t*)temp_pack1,rows_apack*sizeof(unsigned int));
//    SerialUSB.readBytes(buffer_int32,1);
    SerialUSB.write((uint8_t*)temp_pack2,rows_apack*sizeof(unsigned int));

  
}

void Exp(unsigned long int data_in_long_3[],unsigned long int data_in_long_1[],
unsigned long int data_in_long_2[], int input_count, int columns){
    double m_e[input_count];
    unsigned int results1[input_count],results2[input_count];
    long int q_e[input_count],q_temp;
    int flag1;
    double pow52=pow(2,52);
    randomSeed(11);
    for(int i = 0;i<input_count*columns;i++){
      if(data_in_long_3[i]>q_L_3){
        flag1=2;
      }
      else if (data_in_long_3[i]>q_L){
        flag1=1;
      }
      else{flag1=0;}
      if(flag1==2)
        m_e[i] = (double)data_in_long_1[i]*(double)data_in_long_2[i]/m_L/m_L;
      else if (flag1)
        m_e[i] = (double)data_in_long_1[i]*(double)data_in_long_2[i]/m_L/pow52;
      else
        m_e[i] = (double)data_in_long_1[i]*(double)data_in_long_2[i]/pow52/pow52;
      q_e[i] = data_in_long_3[i] - q_L*flag1;
      for (int i = 0;i<input_count;i++){
        double sum = 0;
        double temp_m[columns];
        for (int j = 0;j<columns;j++){
          mask1[i*columns+j]=random(2147483648);//+2147483648;
          mask2[i*columns+j]=random(2147483648);//+2147483648;
          if(random(2)){
            mask1[i*columns+j]+=2147483648;
          }
          if(random(2)){
            mask2[i*columns+j]+=2147483648;
          }
          temp_m[i*columns+j]=m_e[i*columns+j]*pow(2,(q_e[i*columns+j]));
          sum+=temp_m[i*columns+j];
        }
        for (int j = 0;j<columns;j++){
          temp_m[i*columns+j] = temp_m[i*columns+j]/sum;
          results1[i*columns+j] = (unsigned int)(temp_m[i*columns+j]*pow(2,fp))+mask1[(i*columns+j)];
          results2[i*columns+j] = mask2[(i*columns+j)];
        }
      } 
      
    }
    
    SerialUSB.write((uint8_t*)results1,input_count*sizeof(unsigned int));
//    SerialUSB.readBytes(buffer_int32,1);
    SerialUSB.write((uint8_t*)results2,input_count*sizeof(unsigned int));
}

int recover_int32()
{
    int recover = int((unsigned char)(buffer_int32[3]) << 24 |
            (unsigned char)(buffer_int32[2]) << 16 |
            (unsigned char)(buffer_int32[1]) << 8 |
            (unsigned char)(buffer_int32[0]));
    // The buffer is filled from the left
//    SerialUSB.println(buffer_int32[0]);
//    SerialUSB.println(buffer_int32[1]);
//    SerialUSB.println(buffer_int32[2]);
//    SerialUSB.println(buffer_int32[3]);
//    SerialUSB.println(recover);
//    SerialUSB.write("----\n");
    return(recover);
}
