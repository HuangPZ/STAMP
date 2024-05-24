
#pragma once
#include "MHAConfig.h"
#include "Layer.h"
#include "tools.h"
#include "connect.h"
#include "globals.h"
#include "FCLayer.h"
#include "SoftmaxLayer.h"
using namespace std;

extern int partyNum;


class MHALayer : public Layer
{
private:
	MHAConfig conf;
	RSSVectorMyType activations;
	RSSVectorMyType deltas;
	RSSVectorMyType weights;
	RSSVectorMyType biases;
	vector<FCLayer*> FClayers;
	vector<SoftmaxLayer*> SoftmaxLayers;
	vector<RSSVectorMyType*> MatMulresults,Softmaxresults,Beforeconcat;
	RSSVectorMyType Afterconcat,Beforenorm;




public:
	//Constructor and initializer
	MHALayer(MHAConfig* conf, int _layerNum);
	void initialize();

	//Functions
	void printLayer(std::string fn) override;
	void forward(const vector<RSSVectorMyType>& inputActivation) override;
	void computeDelta(RSSVectorMyType& prevDelta) override;
	void updateEquations(const RSSVectorMyType& prevActivations) override;

	//Getters
	RSSVectorMyType* getActivation() {return &activations;};
	RSSVectorMyType* getDelta() {return &deltas;};
	RSSVectorMyType* getWeights() {return &weights;};
	RSSVectorMyType* getBias() {return &biases;};
};