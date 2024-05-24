#include <iostream>
#include <string>
#include "AESObject.h"
#include "Precompute.h"
#include "secondary.h"
#include "connect.h"
#include "NeuralNetConfig.h"
#include "NeuralNetwork.h"

#include "unitTests.h"
#include "Chip.h"
#include <fstream>



int partyNum;
float sendtime;
AESObject* aes_indep;
AESObject* aes_next;
AESObject* aes_prev;
Precompute PrecomputeObject;
extern int functype;
// extern float LAN_ping;
// extern float LAN_Com;
// extern float WAN_ping;
// extern float WAN_Com;
extern int MatMul_Com,ReLU_Com,MP_Com,MP_ReLU_Com,Div_Com,BN_Com,PP_Com,SS_Com;
extern int MatMul_rounds,ReLU_rounds,MP_rounds,Div_rounds,BN_rounds,PP_rounds,SS_rounds;
extern int MatMul_time,ReLU_time,MP_time,MP_ReLU_time,Div_time,BN_time;

extern CommunicationObject commObject;

int main(int argc, char** argv)
{	
	sendtime = 0;
	cout<<"functype"<<functype<<endl;

	RSSVectorMyType x(5);

	
	string Network_l[9] = {"SecureML", "Sarda", "MiniONN", "LeNet", "AlexNet", "VGG16",  "Trans", "ResNet", "Word2Vec"};
	string Dataset_l[2]={"MNIST", "CIFAR10", };//"ImageNet"};
	string network, dataset, security;
	security = "Semi-honest"; // "Semi-honest" or "Malicious"

	for (int networki = 0; networki < 9; networki++) {
		for (int dataseti = 0; dataseti < 2; dataseti++){

			network = Network_l[networki];
			dataset = Dataset_l[dataseti];

			if((network=="SecureML"||network=="Sarda"||network=="MiniONN"||network=="LeNet"
			||network=="New"||network=="Trans"||network=="ResNet"||network=="Word2Vec")&&(dataset!="MNIST")){
				continue;
			} 
			else if ((network=="AlexNet"||network=="VGG16")&&(dataset=="MNIST")){
				continue;
			}
			else if ((network!="SecureML")&&(network!="Word2Vec")){
				continue;
			}

			cout << network << dataset << security<< endl;

		/****************************** PREPROCESSING ******************************/ 
			parseInputs(argc, argv);

			extern Chip chip;
			chip.set_party(partyNum);

			int Nettype = ("Trans"==network);

			NeuralNetConfig* config = new NeuralNetConfig(NUM_ITERATIONS,Nettype);
			
			bool PRELOADING = false;

		/****************************** SELECT NETWORK ******************************/ 
			//Network {SecureML, Sarda, MiniONN, LeNet, AlexNet, and VGG16}
			//Dataset {MNIST, CIFAR10, and ImageNet}
			//Security {Semi-honest or Malicious}
			// if (argc == 9)
			// {network = argv[6]; dataset = argv[7]; security = argv[8];}
			// else
			// {
			// 	network = "AlexNet";//"VGG16";
			// 	dataset = "CIFAR10";//"CIFAR10";
			// 	security = "Semi-honest";
			// }
			selectNetwork(network, dataset, security, config);
			config->checkNetwork();
				cout<<"SSSSS";
			NeuralNetwork* net = new NeuralNetwork(config);
		/****************************** AES SETUP and SYNC ******************************/ 
			aes_indep = new AESObject(argv[3]);
			aes_next = new AESObject(argv[4]);
			aes_prev = new AESObject(argv[5]);
			struct timespec requestStart, requestEnd;
			initializeCommunication(argv[2], partyNum);
			clock_gettime(CLOCK_REALTIME, &requestStart);
			synchronize(200000000);
			clock_gettime(CLOCK_REALTIME, &requestEnd);
			cout << "----------------------------------------------" << endl;
			cout << "Wall Clock time for sync" << ": " << diff(requestStart, requestEnd) << " sec\n";
			cout << "----------------------------------------------" << endl;	
			// pause();
			

		/****************************** RUN NETWORK/UNIT TESTS ******************************/ 
			//Run these if you want a preloaded network to be tested
			// assert(NUM_ITERATIONS == 1 and "check if readMiniBatch is false in test(net)");
			//First argument {SecureML, Sarda, MiniONN, or LeNet}
			// network += " preloaded"; PRELOADING = true;
			// preload_network(PRELOADING, network, net);
			start_m();
			//Run unit tests in two modes: 
			//	1. Debug {Mat-Mul, VecMul, DotProd, PC, Wrap, ReLUPrime, ReLU, Softmax, Division, BN, SSBits, SS, Avg, and Maxpool}
			//	2. Test {Mat-Mul1, Mat-Mul2, Mat-Mul3 (and similarly) Conv*, ReLU*, ReLUPrime*, and Maxpool*} where * = {1,2,3}
			// runTest("Debug", "Maxpool", network);
			// runTest("Test", "ReLUPrime1", network);

			// Run forward/backward for single layers
			//  1. what {F, D, U}
			// 	2. l {0,1,....NUM_LAYERS-1}
			// size_t l = 0;
			// string what = "F";
			// runOnly(net, l, what, network);

			// //Run training
			// network += " train";
			// train(net, network);

			//Run inference (possibly with preloading a network)
			network += " test";
			test(PRELOADING, network, net);
			
			


			/****************************** OUTPUT ******************************/ 
			ofstream myfile;
			string root_dir = "output/";
			string filename;
			if(functype==2){
				filename = root_dir + "Model__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+".txt";	
			}
			else{
				filename = root_dir + "Og__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+".txt";	
			}
			myfile.open ((filename).c_str());
			myfile.close();
			myfile.open ((filename).c_str(), ios::app);
			myfile << "----------------------------------------------" << endl;  	
			myfile << "Run details: " << NUM_OF_PARTIES << "PC (P" << partyNum 
				<< "), " << NUM_ITERATIONS << " iterations, batch size " << MINI_BATCH_SIZE << endl 
				<< "Running " << security << " " << network << " on " << dataset << " dataset" << endl;
			myfile << "----------------------------------------------" << endl << endl; 
			myfile.close();	
			string filename1,filename2;
			if(functype==2){
				filename1 = root_dir + "Model__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+"_"+"1.txt";
				filename2 = root_dir + "Model__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+"_"+"T.txt";
			}
			else{
				filename1 = root_dir + "Og__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+"_"+"1.txt";	
				filename2 = root_dir + "Og__comm_"+security+"_"+network+"_"+dataset+"_"+to_string(partyNum)+"_"+"T.txt";	
			}
			myfile.open ((filename1).c_str());
			myfile.close();

			myfile.open ((filename1).c_str(), ios::app);
			myfile << endl;
			myfile.close();	
			end_m(network,filename1);
			cout << "ChipClockTime: " << chip.ClockTime <<endl;
			

			myfile.open ((filename1).c_str(), ios::app);
			myfile << "----------------------------------------------" << endl << endl; 
			for(int i = 0;i<NUM_LAYERS;i++){
				myfile<<"layer "<<i<<" : "<<net->time_cost[i]<<"sec"<<endl;
			}
			myfile << "----------------------------------------------" << endl << endl; 
			myfile << "sendtime: "<< sendtime << endl;
			myfile << "ChipClockTime: " << chip.ClockTime <<endl;
			myfile << "ChipMulTimes: " <<(float)chip.multimes/1000000 << endl;
			
			myfile << "TransInBits: " <<(float)(chip.TransInBits)/1000000 << endl;
			myfile << "TransInTimes: " <<chip.TransInTimes << endl;
			myfile << "TransOutBits: " <<(float)(chip.TransOutBits)/1000000 << endl;
			myfile << "TransOutTimes: " <<chip.TransOutTimes << endl;
			myfile << "P" << partyNum << endl;
			myfile << "MatMul_Com: "<< (float)(MatMul_Com)/1000000 << endl;
			myfile << "ReLU_Com: "<< (float)(ReLU_Com)/1000000<<   endl;
			myfile << "MP_Com: "<< (float)(MP_Com)/1000000 << endl;
			myfile << "MP_ReLU_Com: "<< (float)(MP_ReLU_Com)/1000000 << endl;
			myfile << "BN_Com: "<< (float)(BN_Com)/1000000 << endl;
			myfile << "Div_Com: "<< (float)(Div_Com)/1000000 << endl;
			myfile << "PP_Com: "<< (float)(PP_Com)/1000000 << endl;
			myfile << "SS_Com: "<< (float)(SS_Com)/1000000 << endl;
			myfile << "----------------------------------------------" << endl << endl; 
			myfile << "P" << partyNum << endl;
			myfile << "MatMul_time: "<< (float)(MatMul_time)/CLOCKS_PER_SEC << endl;
			myfile << "ReLU_time: "<< (float)(ReLU_time)/CLOCKS_PER_SEC << endl;
			myfile << "MP_time: "<< (float)(MP_time)/CLOCKS_PER_SEC << endl;
			myfile << "MP_ReLU_time: "<< (float)(MP_ReLU_time)/CLOCKS_PER_SEC << endl;
			myfile << "BN_time: "<< (float)(BN_time)/CLOCKS_PER_SEC << endl;
			myfile << "Div_time: "<< (float)(Div_time)/CLOCKS_PER_SEC << endl;
			myfile << "----------------------------------------------" << endl << endl; 
			myfile << "P" << partyNum << endl;
			myfile << "MatMul_rounds: "<< MatMul_rounds << endl;
			myfile << "ReLU_rounds: "<< ReLU_rounds << endl;
			myfile << "MP_rounds: "<< MP_rounds << endl;
			myfile << "BN_rounds: "<< BN_rounds << endl;
			myfile << "Div_rounds: "<< Div_rounds << endl;
			myfile << "PP_rounds: "<< PP_rounds << endl;
			myfile << "SS_rounds: "<< SS_rounds << endl;
			myfile << "----------------------------------------------" << endl << endl; 
			if(functype==2){

				myfile << "AddTimes: " <<(float)chip.AddTimes/1000000 << endl;
				myfile << "AddBits: " <<(float)(chip.AddBits)/1000000 << endl;
				myfile << "CompareTimes: " <<(float)chip.CompareTimes/1000000 << endl;
				myfile << "CompareBits: " <<(float)(chip.CompareBits)/1000000 << endl;
				myfile << "AESTimes: " <<(float)chip.AESTimes/1000000 << endl;
				myfile << "AESBits: " <<(float)(chip.AESBits)/1000000 << endl;
				myfile << "XORTimes: " <<(float)chip.XORTimes/1000000 << endl;
				myfile << "TransInBits: " <<(float)(chip.TransInBits)/1000000 << endl;
				myfile << "TransInTimes: " <<chip.TransInTimes << endl;
				myfile << "TransOutBits: " <<(float)(chip.TransOutBits)/1000000 << endl;
				myfile << "TransOutTimes: " <<chip.TransOutTimes << endl;
				myfile << "SimTime: " <<chip.Simtime << endl;
				myfile << "TotalSimtime:"<< chip.Simtime+(float)(chip.TransInBits+chip.TransOutBits)/(1e9)+\
				(float)(chip.TransInTimes+chip.TransOutTimes)/(1e6) << endl;
				myfile << "sendtime: "<< sendtime << endl;
				myfile << "----------------------------------------------" << endl; 	
				cout << "TransInBits: " <<(float)(chip.TransInBits)/1000000 << endl;
				cout << "TransInTimes: " <<chip.TransInTimes << endl;
				cout << "TransOutBits: " <<(float)(chip.TransOutBits)/1000000 << endl;
				cout << "TransOutTimes: " <<chip.TransOutTimes << endl;
				myfile << "ReluTimes: " <<chip.ReluTimes << endl;
				myfile << "Max_Comparetimes: " <<chip.Max_Comparetimes << endl;
				myfile << "SoftmaxTimes: " <<chip.SoftmaxTimes << endl;
				myfile << "LayerNormTimes: " <<chip.LayerNormTimes << endl;
				chip.Reset();
				
			}



			myfile.close();

			sendtime = 0;

			// printNetwork(net,filename1);
			
			cpu.Write(filename2);

		/****************************** CLEAN-UP ******************************/ 
			delete aes_indep;
			delete aes_next;
			delete aes_prev;
			delete config;
			delete net;
			commObject.reset();
			deleteObjects();
			MatMul_Com=ReLU_Com=MP_Com=MP_ReLU_Com=Div_Com=BN_Com=PP_Com=SS_Com=0;
			MatMul_rounds=ReLU_rounds=MP_rounds=Div_rounds=BN_rounds=PP_rounds=SS_rounds=0;
			MatMul_time=ReLU_time=MP_time=MP_ReLU_time=Div_time=BN_time=0;
			
			cpu.Reset();
	}
	}

	return 0;
}




