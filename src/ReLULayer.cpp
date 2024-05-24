
#pragma once
#include "ReLULayer.h"
#include "Functionalities.h"
using namespace std;

ReLULayer::ReLULayer(ReLUConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf( conf->inputDim,conf->batchSize), //Falcon bug: inputdim batchsize reverted
 activations(conf->batchSize * conf->inputDim), 
 deltas(conf->batchSize * conf->inputDim),
 reluPrime(conf->batchSize * conf->inputDim)
{}


void ReLULayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") ReLU Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") ReLU Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	// myfile.close();
}


void ReLULayer::forward(const RSSVectorMyType &inputActivation)
{
	log_print("ReLU.forward");
	
	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t size = rows*columns;
	
	if (FUNCTION_TIME)
		cout << "funcRELU: " << funcTime(funcRELU, inputActivation, reluPrime, activations, size) << endl;
	else
		funcRELU(inputActivation, reluPrime, activations, size);
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcRELU(inputActivation, reluPrime, activations, size);
}


void ReLULayer::computeDelta(RSSVectorMyType& prevDelta)
{
	log_print("ReLU.computeDelta");

	//Back Propagate	
	
	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t size = rows*columns;

	if (FUNCTION_TIME)
		cout << "funcSelectShares: " << funcTime(funcSelectShares, deltas, reluPrime, prevDelta, size) << endl;
	else
		funcSelectShares(deltas, reluPrime, prevDelta, size);

}


void ReLULayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	log_print("ReLU.updateEquations");
}
