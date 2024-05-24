
#pragma once
#include "NeuralNetConfig.h"
#include "Layer.h"
#include "Network.h"
#include "globals.h"
using namespace std;

class TransNetwork: public Network
{
public:
	RSSVectorMyType inputData,outputData;
	RSSVectorMyType outputProb;
	vector<Layer*> layers;
	//EmbIn, EmbOut, MHAIn, FFIn, MHAOut1, MHAOut2, FFOut, Linear, Softmax;
	TransNetwork(NeuralNetConfig* config);
	~TransNetwork();
	void forward() override;
	void backward() override;
	void computeDelta() override;
	void updateEquations() override;
	void predict(vector<myType> &maxIndex) override;
	void getAccuracy(const vector<myType> &maxIndex, vector<size_t> &counter,string network);
};