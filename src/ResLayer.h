
#pragma once
#include "ResConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
#include "CNNLayer.h"
#include "ReLULayer.h"
#include "BNLayer.h"
using namespace std;

extern int partyNum;


class ResLayer : public Layer
{
public:
	ResConfig conf;


	RSSVectorMyType activations;
	RSSVectorMyType deltas;
	RSSVectorMyType weights;
	RSSVectorMyType biases;
	vector<CNNLayer*> CNNlayers;
	// vector<BNLayer*> BNlayers;
	vector<ReLULayer*> ReLULayers;





	//Constructor and initializer
	ResLayer(ResConfig* conf, int _layerNum);
	void initialize();

	//Functions
	void printLayer(std::string fn) override;
	void forward(const RSSVectorMyType &inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
	RSSVectorMyType* getWeights() {return &weights;};
	RSSVectorMyType* getBias() {return &biases;};
};