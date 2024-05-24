
#pragma once
#include "SoftmaxConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
using namespace std;

extern int partyNum;


class SoftmaxLayer : public Layer
{
private:
	SoftmaxConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;
    RSSVectorSmallType maxPrime;


public:
	//Constructor and initializer
	SoftmaxLayer(SoftmaxConfig* conf, int _layerNum);

	//Functions
	void printLayer(std::string fn) override;
	void forward(const RSSVectorMyType& inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
};