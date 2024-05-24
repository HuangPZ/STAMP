
#pragma once
#include "globals.h"

extern string SECURITY_TYPE;
class Layer
{
public: 
	int layerNum = 0;
	Layer(int _layerNum): layerNum(_layerNum) {};

//Virtual functions	
	virtual void printLayer(std::string fn) {};
	virtual void forward(const RSSVectorMyType& inputActivation) {};
	virtual void forward(const vector<RSSVectorMyType>& inputActivation) {};
	virtual void computeDelta(RSSVectorMyType& prevDelta) {};
	virtual void updateEquations(const RSSVectorMyType& prevActivations) {};

//Getters
	virtual RSSVectorMyType* getActivation() {};
	virtual RSSVectorMyType* getDelta() {};
	virtual RSSVectorMyType* getWeights() {};
};