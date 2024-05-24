
#pragma once
#include "BNConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
using namespace std;


class BNLayer : public Layer
{
public:
	BNConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;
	RSSVectorMyType gamma;
	RSSVectorMyType beta;
	RSSVectorMyType xhat;
	RSSVectorMyType sigma;


	//Constructor and initializer
	BNLayer(BNConfig* conf, int _layerNum);
	void initialize();

	//Functions
	void printLayer(std::string fn) override;
	void forward(const RSSVectorMyType& inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
};