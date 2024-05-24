
#pragma once
#include "AvgLayer.h"
#include "Functionalities.h"
using namespace std;

AvgLayer::AvgLayer(AvgConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf( conf->inputDim,conf->batchSize), 
 activations(conf->batchSize,make_pair(0,0)), 
 deltas(conf->batchSize * conf->inputDim)
{}


void AvgLayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") Avg Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") Avg Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	// myfile.close();
}


void AvgLayer::forward(const RSSVectorMyType &inputActivation)
{

	size_t batchSize = conf.batchSize;
	size_t inputDim = conf.inputDim;
	funcAvgpool(inputActivation, activations, batchSize, inputDim);

}


void AvgLayer::computeDelta(RSSVectorMyType& prevDelta)
{
	log_print("Avg.computeDelta");


}


void AvgLayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	log_print("Avg.updateEquations");
}
