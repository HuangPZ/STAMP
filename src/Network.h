
#pragma once
#include "NetConfig.h"
#include "Layer.h"
#include "globals.h"
using namespace std;
class Network
{
public:
	RSSVectorMyType inputData,outputData;
	RSSVectorMyType outputProb;
	vector<Layer*> layers;
	Network() {};
	virtual ~Network() {}
	virtual void forward(){};
	virtual void backward(){};
	virtual void computeDelta(){};
	virtual void updateEquations(){};
	virtual void predict(vector<myType> &maxIndex){};
	virtual void getAccuracy(const vector<myType> &maxIndex, vector<size_t> &counter,string network){};
};