
#pragma once
#include "FFLayer.h"
#include "Functionalities.h"
using namespace std;

FFLayer::FFLayer(FFConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf(conf->inputDim, conf->batchSize),
 activations(conf->batchSize * conf->inputDim), 
 deltas(conf->batchSize * conf->inputDim),
 weights(conf->inputDim * conf->inputDim),
 biases(conf->inputDim)
{
	initialize();
}


void FFLayer::initialize()
{
	// //Initialize weights and biases here.
	// //Ensure that initialization is correctly done.
	// size_t lower = 30;
	// size_t higher = 50;
	// size_t decimation = 10000;
	// size_t size = weights.size();

	// RSSVectorMyType temp(size);
	// srand(10);
	// if(partyNum==PARTY_A){
	// 	for (size_t i = 0; i < size; ++i){
	// 		weights[i].first = floatToMyType((float)(rand() % (higher - lower) + lower)/decimation);
	// 		weights[i].second = 0;
	// 	}
	// 	for (size_t i = 0; i < biases.size(); ++i){
	// 		biases[i].first = floatToMyType((float)(rand() % (higher - lower) + lower)/decimation);
	// 		biases[i].second = 0;
	// 	}
	// }
	// if(partyNum==PARTY_B){
	// 	for (size_t i = 0; i < size; ++i){
	// 		weights[i].first = 0;
	// 		weights[i].second = 0;
	// 	}
	// 	for (size_t i = 0; i < biases.size(); ++i){
	// 		biases[i].first = 0;
	// 		biases[i].second = 0;
	// 	}
	// }
	// if(partyNum==PARTY_C){
	// 	for (size_t i = 0; i < size; ++i){
	// 		weights[i].second = floatToMyType((float)(rand() % (higher - lower) + lower)/decimation);
	// 		weights[i].first = 0;
	// 	}
	// 	for (size_t i = 0; i < biases.size(); ++i){
	// 		biases[i].second = floatToMyType((float)(rand() % (higher - lower) + lower)/decimation);
	// 		biases[i].first = 0;
	// 	}
	// }
}


void FFLayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") FF Layer\t\t  " << conf.inputDim << " x " << conf.inputDim << endl << "\t\t\t  "
		 << conf.batchSize << "\t\t (Batch Size)" << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") FF Layer\t\t  " << conf.inputDim << " x " << conf.inputDim << endl << "\t\t\t  "
	// 	 << conf.batchSize << "\t\t (Batch Size)" << endl;
	// myfile.close();
}


void FFLayer::forward(const RSSVectorMyType &inputActivation)
{
	log_print("FF.forward");

	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t common_dim = conf.inputDim;
	size_t size = rows*columns;

	if (FUNCTION_TIME)
		cout << "funcMatMul: " << funcTime(funcMatMul, inputActivation, weights, activations, rows, common_dim, columns, 0, 0, FLOAT_PRECISION) << endl;
	else
		funcMatMul(inputActivation, weights, activations, rows, common_dim, columns, 0, 0, FLOAT_PRECISION);
	// if (SECURITY_TYPE.compare("Malicious") == 0)
	// 	funcMatMul(inputActivation, weights, activations, rows, common_dim, columns, 0, 0, FLOAT_PRECISION);
	for(size_t r = 0; r < rows; ++r)
		for(size_t c = 0; c < columns; ++c){
			activations[r*columns + c] = activations[r*columns + c] + biases[c] + inputActivation[r*columns + c];
		}
	if (FUNCTION_TIME){
		cout << "funcLayerNorm: " << funcTime(funcLayerNorm, activations,activations,
		rows,columns) << endl;
	}
	else{
		funcLayerNorm(activations,activations,rows,columns);
	}
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcLayerNorm(activations,activations,rows,columns);
	
}


void FFLayer::computeDelta(RSSVectorMyType& prevDelta)
{
	log_print("FF.computeDelta");

	//Back Propagate	
	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t common_dim = conf.inputDim;
	
	if (FUNCTION_TIME)
		cout << "funcMatMul: " << funcTime(funcMatMul, deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION) << endl;
	else
		funcMatMul(deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION);
	
}


void FFLayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	log_print("FF.updateEquations");

	size_t rows = conf.batchSize;
	size_t columns = conf.inputDim;
	size_t common_dim = conf.inputDim;
	size_t size = rows*columns;	
	RSSVectorMyType temp(columns, std::make_pair(0,0));

	//Update Biases
	for (size_t i = 0; i < rows; ++i)
		for (size_t j = 0; j < columns; ++j)
			temp[j] = temp[j] + deltas[i*columns + j];

	funcTruncate(temp, LOG_MINI_BATCH + LOG_LEARNING_RATE, columns);
	subtractVectors<RSSMyType>(biases, temp, biases, columns);

	//Update Weights 
	rows = conf.inputDim;
	columns = conf.inputDim;
	common_dim = conf.batchSize;
	size = rows*columns;
	RSSVectorMyType deltaWeight(size);

	if (FUNCTION_TIME)
		cout << "funcMatMul: " << funcTime(funcMatMul, prevActivations, deltas, deltaWeight, rows, common_dim, columns, 1, 0, FLOAT_PRECISION + LOG_LEARNING_RATE + LOG_MINI_BATCH) << endl;
	else
		funcMatMul(prevActivations, deltas, deltaWeight, rows, common_dim, columns, 1, 0, 
					FLOAT_PRECISION + LOG_LEARNING_RATE + LOG_MINI_BATCH);
	subtractVectors<RSSMyType>(weights, deltaWeight, weights, size);		
}
