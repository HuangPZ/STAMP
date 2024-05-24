
#pragma once
#include "FFConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
using namespace std;

extern int partyNum;


class FFLayer : public Layer
{
private:
	FFConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;
	RSSVectorMyType weights;
	RSSVectorMyType biases;


public:
	//Constructor and initializer
	FFLayer(FFConfig* conf, int _layerNum);
	void initialize();

	//Functions
	void printLayer(std::string fn) override;
	void forward(const RSSVectorMyType& inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
	RSSVectorMyType* getWeights() {return &weights;};
	RSSVectorMyType* getBias() {return &biases;};
};