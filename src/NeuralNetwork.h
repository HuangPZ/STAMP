
#pragma once
#include "NeuralNetConfig.h"
#include "Layer.h"
#include "globals.h"
using namespace std;
extern double diff(timespec start, timespec end);
class NeuralNetwork
{
public:
	RSSVectorMyType inputData,outputData;
	RSSVectorMyType outputProb;
	vector<Layer*> layers;
	size_t Nettype;
	double *time_cost;
	NeuralNetwork(NeuralNetConfig* config);
	~NeuralNetwork();
	void forward();
	void backward();
	void computeDelta();
	void updateEquations();
	void predict(vector<myType> &maxIndex);
	void getAccuracy(const vector<myType> &maxIndex, vector<size_t> &counter,string network);
};