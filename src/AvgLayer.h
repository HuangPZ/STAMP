
#pragma once
#include "AvgConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
using namespace std;

extern int partyNum;


class AvgLayer : public Layer
{
private:
	AvgConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;

public:
	//Constructor and initializer
	AvgLayer(AvgConfig* conf, int _layerNum);

	//Functions
	void printLayer(std::string fn) override;
	void forward(const RSSVectorMyType& inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
};