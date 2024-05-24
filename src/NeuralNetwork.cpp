
#pragma once
#include "tools.h"
#include "FCLayer.h"
#include "CNNLayer.h"
#include "MaxpoolLayer.h"
#include "ReLULayer.h"
#include "BNLayer.h"
#include "SoftmaxLayer.h"
#include "EmbLayer.h"
#include "MHALayer.h"
#include "FFLayer.h"
#include "ResLayer.h"
#include "AvgLayer.h"
#include "NeuralNetwork.h"
#include "Functionalities.h"
#include <string>
using namespace std;

extern size_t INPUT_SIZE;
extern size_t LAST_LAYER_SIZE;
extern bool WITH_NORMALIZATION;
extern bool LARGE_NETWORK;

NeuralNetwork::NeuralNetwork(NeuralNetConfig* config)
:Nettype(config->Nettype),
 inputData(INPUT_SIZE * MINI_BATCH_SIZE),
 outputData(LAST_LAYER_SIZE * MINI_BATCH_SIZE),
 outputProb(LAST_LAYER_SIZE * MINI_BATCH_SIZE)
{
	if(Nettype==0){
		for (size_t i = 0; i < NUM_LAYERS; ++i)
		{
			if (config->layerConf[i]->type.compare("FC") == 0) {
				FCConfig *cfg = static_cast<FCConfig *>(config->layerConf[i]);
				layers.push_back(new FCLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("CNN") == 0) {
				CNNConfig *cfg = static_cast<CNNConfig *>(config->layerConf[i]);
				layers.push_back(new CNNLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("Maxpool") == 0) {
				MaxpoolConfig *cfg = static_cast<MaxpoolConfig *>(config->layerConf[i]);
				layers.push_back(new MaxpoolLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("ReLU") == 0) {
				ReLUConfig *cfg = static_cast<ReLUConfig *>(config->layerConf[i]);
				layers.push_back(new ReLULayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("BN") == 0) {
				BNConfig *cfg = static_cast<BNConfig *>(config->layerConf[i]);
				layers.push_back(new BNLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("Softmax") == 0) {
				SoftmaxConfig *cfg = static_cast<SoftmaxConfig *>(config->layerConf[i]);
				layers.push_back(new SoftmaxLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("Res") == 0) {
				ResConfig *cfg = static_cast<ResConfig *>(config->layerConf[i]);
				layers.push_back(new ResLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("Avg") == 0) {
				AvgConfig *cfg = static_cast<AvgConfig *>(config->layerConf[i]);
				layers.push_back(new AvgLayer(cfg, i));
			}
			else if (config->layerConf[i]->type.compare("Emb") == 0) {
				EmbConfig *cfg = static_cast<EmbConfig *>(config->layerConf[i]);
				layers.push_back(new EmbLayer(cfg, i));
			}
			else{
				cout<<config->layerConf[i]->type;
				error(": Only FC, CNN, ReLU, Maxpool, AvgPool, BN, Softmax, Res layer types currently supported");
			}
		}
		time_cost = new double[NUM_LAYERS];

	}
	else if (Nettype==1){
		//EmbIn
		EmbConfig *cfg0 = static_cast<EmbConfig *>(config->layerConf[0]);
		layers.push_back(new EmbLayer(cfg0, 0));
		//MHAIn
		MHAConfig *cfg1 = static_cast<MHAConfig *>(config->layerConf[1]);
		layers.push_back(new MHALayer(cfg1, 1));
		//FFIn
		FFConfig *cfg2 = static_cast<FFConfig *>(config->layerConf[2]);
		layers.push_back(new FFLayer(cfg2, 2));
		//EmbOut
		EmbConfig *cfg3 = static_cast<EmbConfig *>(config->layerConf[3]);
		layers.push_back(new EmbLayer(cfg3, 3));
		//MHAOut1
		MHAConfig *cfg4 = static_cast<MHAConfig *>(config->layerConf[4]);
		layers.push_back(new MHALayer(cfg4, 4));
		//MHAOut2
		MHAConfig *cfg5 = static_cast<MHAConfig *>(config->layerConf[5]);
		layers.push_back(new MHALayer(cfg5, 5));
		//FFOut
		FFConfig *cfg6 = static_cast<FFConfig *>(config->layerConf[6]);
		layers.push_back(new FFLayer(cfg6, 6));
		//Linear
		FCConfig *cfg7 = static_cast<FCConfig *>(config->layerConf[7]);
		layers.push_back(new FCLayer(cfg7, 7));
		// //Softmax
		// SoftmaxConfig *cfg8 = static_cast<SoftmaxConfig *>(config->layerConf[8]);
		// layers.push_back(new SoftmaxLayer(cfg8, 8));
		time_cost = new double[9];
	}
	
}


NeuralNetwork::~NeuralNetwork()
{
	for (vector<Layer*>::iterator it = layers.begin() ; it != layers.end(); ++it)
		delete (*it);

	layers.clear();
}

void NeuralNetwork::forward()
{
	struct timespec requestStart, requestEnd;
	log_print("NN.forward");
	#if COUNTTIME
	if(Nettype==0){
		
		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[0]->forward(inputData);
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[0]=diff(requestStart, requestEnd);
		if (LARGE_NETWORK)
			cout << "Forward \t" << layers[0]->layerNum << " completed..." << endl;
		
		// print_vector(inputData, "FLOAT", "inputData:", 784);
		// print_vector(*((CNNLayer*)layers[0])->getWeights(), "FLOAT", "w0:", 20);
		// print_vector((*layers[0]->getActivation()), "FLOAT", "a0:", 3000);
		for (size_t i = 1; i < NUM_LAYERS; ++i)
		{
			clock_gettime(CLOCK_REALTIME, &requestStart);
			layers[i]->forward(*(layers[i-1]->getActivation()));
			clock_gettime(CLOCK_REALTIME, &requestEnd);
			time_cost[i]=diff(requestStart, requestEnd);
			if (LARGE_NETWORK)
				cout << "Forward \t" << layers[i]->layerNum << " completed..." << endl;

			// print_vector((*layers[i]->getActivation()), "FLOAT", "Activation Layer"+to_string(i), 
			// 			(*layers[i]->getActivation()).size());

			// print_vector((*layers[i]->getActivation()), "FLOAT", "Activation Layer "+to_string(i), 200);
		}
	}
	else if (Nettype==1){
		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[0]->forward(inputData);//EmbIn
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[0]=diff(requestStart, requestEnd);

		vector<RSSVectorMyType> BeforeMHAIn(3);
		for(int i = 0;i<3; i++){
			BeforeMHAIn[i] = *(layers[0]->getActivation());
		}
		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[1]->forward(BeforeMHAIn);//MHAIn
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[1]=diff(requestStart, requestEnd);

		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[2]->forward(*(layers[1]->getActivation()));//FFIn
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[2]=diff(requestStart, requestEnd);
		
		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[3]->forward(outputData);//EmbOut
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[3]=diff(requestStart, requestEnd);
		
		vector<RSSVectorMyType> BeforeMHAOut1(3);
		for(int i = 0;i<3; i++){
			BeforeMHAOut1[i] = *(layers[3]->getActivation());
		}

		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[4]->forward(BeforeMHAOut1);//MHAOut1
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[4]=diff(requestStart, requestEnd);
		

		vector<RSSVectorMyType> BeforeMHAOut2(3);
		for(int i = 0;i<2; i++){
			BeforeMHAOut1[i] = *(layers[2]->getActivation());
		}
		BeforeMHAOut1[2] = *(layers[4]->getActivation());

		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[5]->forward(BeforeMHAOut1);//MHAOut2
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[5]=diff(requestStart, requestEnd);
		

		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[6]->forward(*(layers[5]->getActivation()));//FFOut
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[6]=diff(requestStart, requestEnd);
		

		clock_gettime(CLOCK_REALTIME, &requestStart);
		layers[7]->forward(*(layers[6]->getActivation()));//Linear
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		time_cost[7]=diff(requestStart, requestEnd);
		

		// clock_gettime(CLOCK_REALTIME, &requestStart);
		// layers[8]->forward(*(layers[7]->getActivation()));//Softmax
		// clock_gettime(CLOCK_REALTIME, &requestEnd);
		// time_cost[8]=diff(requestStart, requestEnd);
		

	}
	#else
	if(Nettype==0){
		
		
		layers[0]->forward(inputData);
		

		if (LARGE_NETWORK)
			cout << "Forward \t" << layers[0]->layerNum << " completed..." << endl;

		// print_vector(inputData, "FLOAT", "inputData:", 784);
		// print_vector(*((CNNLayer*)layers[0])->getWeights(), "FLOAT", "w0:", 20);
		// print_vector((*layers[0]->getActivation()), "FLOAT", "a0:", 3000);
		for (size_t i = 1; i < NUM_LAYERS; ++i)
		{
			
			layers[i]->forward(*(layers[i-1]->getActivation()));
			

			if (LARGE_NETWORK)
				cout << "Forward \t" << layers[i]->layerNum << " completed..." << endl;

			// print_vector((*layers[i]->getActivation()), "FLOAT", "Activation Layer"+to_string(i), 
			// 			(*layers[i]->getActivation()).size());

			// print_vector((*layers[i]->getActivation()), "FLOAT", "Activation Layer "+to_string(i), 200);
		}
	}
	else if (Nettype==1){
		
		layers[0]->forward(inputData);//EmbIn
		


		vector<RSSVectorMyType> BeforeMHAIn(3);
		for(int i = 0;i<3; i++){
			BeforeMHAIn[i] = *(layers[0]->getActivation());
		}
		
		layers[1]->forward(BeforeMHAIn);//MHAIn
		


		
		layers[2]->forward(*(layers[1]->getActivation()));//FFIn
		

		
		
		layers[3]->forward(outputData);//EmbOut
		

		
		vector<RSSVectorMyType> BeforeMHAOut1(3);
		for(int i = 0;i<3; i++){
			BeforeMHAOut1[i] = *(layers[3]->getActivation());
		}

		
		layers[4]->forward(BeforeMHAOut1);//MHAOut1
		

		

		vector<RSSVectorMyType> BeforeMHAOut2(3);
		for(int i = 0;i<2; i++){
			BeforeMHAOut1[i] = *(layers[2]->getActivation());
		}
		BeforeMHAOut1[2] = *(layers[4]->getActivation());

		
		layers[5]->forward(BeforeMHAOut1);//MHAOut2
		

		

		
		layers[6]->forward(*(layers[5]->getActivation()));//FFOut

		
		layers[7]->forward(*(layers[6]->getActivation()));//Linear
		
		// layers[8]->forward(*(layers[7]->getActivation()));//Softmax
		
		

	}
	#endif
	
}

//TODO: backward for Trans
void NeuralNetwork::backward()
{
	log_print("NN.backward");

	computeDelta();
	updateEquations();
}

void NeuralNetwork::computeDelta()
{
	log_print("NN.computeDelta");

	size_t rows = MINI_BATCH_SIZE;
	size_t columns = LAST_LAYER_SIZE;
	size_t size = rows*columns;
	size_t index;

	if (WITH_NORMALIZATION)
	{
		RSSVectorMyType rowSum(size, make_pair(0,0));
		RSSVectorMyType quotient(size, make_pair(0,0));

		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < columns; ++j)
				rowSum[i*columns] = rowSum[i*columns] + 
									(*(layers[NUM_LAYERS-1]->getActivation()))[i * columns + j];

		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < columns; ++j)
				rowSum[i*columns + j] = rowSum[i*columns];

		funcDivision(*(layers[NUM_LAYERS-1]->getActivation()), rowSum, quotient, size);

		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < columns; ++j)
			{
				index = i * columns + j;
				(*(layers[NUM_LAYERS-1]->getDelta()))[index] = quotient[index] - outputData[index];
			}
	}
	else
	{
		for (size_t i = 0; i < rows; ++i)
			for (size_t j = 0; j < columns; ++j)
			{
				index = i * columns + j;
				(*(layers[NUM_LAYERS-1]->getDelta()))[index] = 
				(*(layers[NUM_LAYERS-1]->getActivation()))[index] - outputData[index];

			}
			
	}

	if (LARGE_NETWORK)		
		cout << "Delta last layer completed." << endl;

	for (size_t i = NUM_LAYERS-1; i > 0; --i)
	{
		layers[i]->computeDelta(*(layers[i-1]->getDelta()));
		if (LARGE_NETWORK)
			cout << "Delta \t\t" << layers[i]->layerNum << " completed..." << endl;

	}

}

void NeuralNetwork::updateEquations()
{
	log_print("NN.updateEquations");

	for (size_t i = NUM_LAYERS-1; i > 0; --i)
	{
		layers[i]->updateEquations(*(layers[i-1]->getActivation()));	
		if (LARGE_NETWORK)
			cout << "Update Eq. \t" << layers[i]->layerNum << " completed..." << endl;	
	}

	layers[0]->updateEquations(inputData);
	if (LARGE_NETWORK)
		cout << "First layer update Eq. completed." << endl;		
}

void NeuralNetwork::predict(vector<myType> &maxIndex)
{
	log_print("NN.predict");

	size_t rows = MINI_BATCH_SIZE;
	size_t columns = LAST_LAYER_SIZE;

	RSSVectorMyType max(rows);
	RSSVectorSmallType maxPrime(rows*columns);
	funcMaxpool(*(layers[NUM_LAYERS-1]->getActivation()), max, maxPrime, rows, columns);
	//Falcon Forgot about this....
	vector<smallType> reconst_maxPrime(maxPrime.size());
	vector<myType> reconst_max(max.size());
	// funcReconstruct(max, reconst_max, rows, "max", false);
		
	funcReconstructBit(maxPrime, reconst_maxPrime, rows*columns, "maxP", false);
    for (int i=0;i<rows;i++){
        for(int j=0;j<columns;j++){
					int a = reconst_maxPrime[i*columns+j];
					if(reconst_maxPrime[i*columns+j]){
						maxIndex[i]=j;
					}
        }
    }
}

void NeuralNetwork::getAccuracy(const vector<myType> &maxIndex, vector<size_t> &counter, string network)
{
	log_print("NN.getAccuracy");
	size_t rows = MINI_BATCH_SIZE;
	size_t columns = LAST_LAYER_SIZE;

	string path_input =  "files/preload/"+which_network(network)+"/label";
	ifstream f_input(path_input);
	vector<myType> label(MINI_BATCH_SIZE);
	string temp;
	for (int i = 0; i < MINI_BATCH_SIZE; ++i)
	{
		f_input >> temp; 
		label[i] = std::stof(temp);
	}
	
	// RSSVectorMyType max(rows);
	// RSSVectorSmallType maxPrime(rows*columns);

	// //Needed maxIndex here
	// funcMaxpool(outputData, max, maxPrime, rows, columns);
	
	// //Reconstruct things
/******************************** TODO ****************************************/
	RSSVectorMyType temp_max(rows), temp_groundTruth(rows);
	// if (partyNum == PARTY_B)
	// 	sendTwoVectors<RSSMyType>(max, groundTruth, PARTY_A, rows, rows);

	// if (partyNum == PARTY_A)
	// {
	// 	receiveTwoVectors<RSSMyType>(temp_max, temp_groundTruth, PARTY_B, rows, rows);
	// 	addVectors<RSSMyType>(temp_max, max, temp_max, rows);
//		dividePlain(temp_max, (1 << FLOAT_PRECISION));
	// 	addVectors<RSSMyType>(temp_groundTruth, groundTruth, temp_groundTruth, rows);	
	// }
/******************************** TODO ****************************************/

	for (size_t i = 0; i < MINI_BATCH_SIZE; ++i)
	{
		counter[1]++;
		if (label[i] == maxIndex[i])
			counter[0]++;
	}		

	cout << "Rolling accuracy: " << counter[0] << " out of " 
		 << counter[1] << " (" << (counter[0]*100/counter[1]) << " %)" << endl;
}


