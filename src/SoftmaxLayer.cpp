
#pragma once
#include "SoftmaxLayer.h"
#include "Functionalities.h"
using namespace std;

SoftmaxLayer::SoftmaxLayer(SoftmaxConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf(conf->inputDim, conf->batchSize, conf->masked),
 activations(conf->batchSize * conf->inputDim), 
 deltas(conf->batchSize * conf->inputDim),
 maxPrime(conf->batchSize* conf->inputDim)
{}


void SoftmaxLayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") Softmax Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") ReLU Layer\t\t  " << conf.batchSize << " x " << conf.inputDim << endl;
	// myfile.close();
}


void SoftmaxLayer::forward(const RSSVectorMyType &inputActivation)
{
	log_print("SoftmaxLayer.forward");

	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t size = rows*columns;
	bool masked = conf.masked;
	RSSVectorMyType max(rows),temp(size);


	if (FUNCTION_TIME)
		cout << "funcSoftmax: " << funcTime(funcSoftmax, inputActivation, activations, rows, columns,masked) << endl;
		
	else
		funcSoftmax( inputActivation, activations, rows, columns, masked);
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcSoftmax( inputActivation, activations, rows, columns, masked);
	
}


void SoftmaxLayer::computeDelta(RSSVectorMyType& prevDelta)
{
	log_print("ReLU.computeDelta");

	//Back Propagate	
	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t size = rows*columns;
	RSSVectorMyType SiSj(rows*columns*columns); 

	if (FUNCTION_TIME)
		cout << "funcVecMul: " << funcTime(funcVecMul, deltas, SiSj, rows, columns, FLOAT_PRECISION) << endl;
	else
		funcVecMul( deltas, SiSj, rows, columns, FLOAT_PRECISION);
	for(int i=0;i<rows;i++){
		for(int j=0;j<columns;j++){
			for(int k=0;k<columns;k++){
				SiSj[i*columns*columns+j*columns+k].first=-SiSj[i*columns*columns+j*columns+k].first;
				SiSj[i*columns*columns+j*columns+k].second=-SiSj[i*columns*columns+j*columns+k].second;
			}
			SiSj[i*columns*columns+j*columns+j].first=SiSj[i*columns*columns+j*columns+j].first+SiSj[i*columns+j].first;
			SiSj[i*columns*columns+j*columns+j].second=SiSj[i*columns*columns+j*columns+j].second+SiSj[i*columns+j].second;	
		}
	}
	RSSVectorMyType* SiSj_temp,deltas_temp;
	size_t const SiSj_size = SiSj.size() / rows;
	size_t const deltas_size = deltas.size() / rows;
	size_t const prevDelta_size = prevDelta.size() / rows;
	for(int i=0;i<rows;i++){
		RSSVectorMyType SiSj_temp(SiSj.begin()+SiSj_size*i, SiSj.begin() + SiSj_size*(i+1));
		RSSVectorMyType deltas_temp(deltas.begin()+deltas_size*i, deltas.begin() + deltas_size*(i+1));
		RSSVectorMyType prevDelta_temp(prevDelta.begin()+prevDelta_size*i, prevDelta.begin() + prevDelta_size*(i+1));
		if (FUNCTION_TIME)
				cout << "funcMatMul: " << funcTime(funcMatMul, deltas_temp, SiSj_temp, prevDelta_temp, 1, columns, columns, 0, 0, FLOAT_PRECISION) << endl;
		else
			funcMatMul( deltas_temp, SiSj_temp, prevDelta_temp, 1, columns, columns, 0, 0, FLOAT_PRECISION) ;
	}
	
}


void SoftmaxLayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	log_print("Softmax.updateEquations");
}
