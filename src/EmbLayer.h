
#pragma once
#include "EmbConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
using namespace std;

extern int partyNum;


class EmbLayer : public Layer
{
private:
	EmbConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;
	RSSVectorMyType weights;


public:
	//Constructor and initializer
	EmbLayer(EmbConfig* conf, int _layerNum);
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
};