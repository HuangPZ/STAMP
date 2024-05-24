
#pragma once
#include "MHALayer.h"

#include "Functionalities.h"
using namespace std;

MHALayer::MHALayer(MHAConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf(conf->inputDim, conf->batchSize, conf->outputDim,conf->nhead, conf->masked),
 activations(conf->batchSize * conf->outputDim), 
 deltas(conf->batchSize * conf->outputDim),
 weights(conf->inputDim * conf->outputDim),
 biases(conf->outputDim)
{
	for(size_t i = 0; i < conf->nhead*3;i++){
		FCConfig *cfg = new FCConfig(conf->inputDim, MINI_BATCH_SIZE, conf->outputDim);
		FClayers.push_back(new FCLayer(cfg, i));
	}
	FCConfig *cfg = new FCConfig(conf->inputDim*conf->nhead, MINI_BATCH_SIZE, conf->outputDim);
	FClayers.push_back(new FCLayer(cfg, conf->nhead*3));
	for(size_t i = 0; i < conf->nhead;i++){
		SoftmaxConfig *cfg = new SoftmaxConfig(conf->inputDim, MINI_BATCH_SIZE);
		SoftmaxLayers.push_back(new SoftmaxLayer(cfg, i));
		MatMulresults.push_back(new RSSVectorMyType(conf->batchSize*conf->batchSize));
		Softmaxresults.push_back(new RSSVectorMyType(conf->batchSize*conf->batchSize));
		Beforeconcat.push_back(new RSSVectorMyType(conf->inputDim*conf->batchSize));
		
	}
	Afterconcat = RSSVectorMyType(conf->inputDim*conf->batchSize*conf->nhead);
	Beforenorm = RSSVectorMyType(conf->inputDim*conf->batchSize);

	initialize();
}


void MHALayer::initialize()
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
		
			

	

		
	
	// fill(biases.begin(), biases.end(), make_pair(0,0));
}


void MHALayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") MHA Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
		 << conf.batchSize << "\t\t (Batch Size)" << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") MHA Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
	// 	 << conf.batchSize << "\t\t (Batch Size)" << endl;
	// myfile.close();
}


void MHALayer::forward(const vector<RSSVectorMyType> &inputActivation)
{
	log_print("MHA.forward");

	size_t batchSize = conf.batchSize;
	size_t outputDim = conf.outputDim;
	size_t inputDim = conf.inputDim;

	size_t nhead = conf.nhead;

	for(size_t h = 0; h < nhead; h++){
		for(size_t i = 0; i<3; i++){
			FClayers[i+h*nhead]->forward(inputActivation[i]);
		}
		if (FUNCTION_TIME){
			cout << "funcMatMul: " << funcTime(funcMatMul, *(FClayers[0+h*3]->getActivation()), 
			*(FClayers[1+h*3]->getActivation()), 
			*(MatMulresults[h]), batchSize, inputDim, batchSize, 0, 1, FLOAT_PRECISION) << endl;
			cout << "funcSoftmax: " << funcTime(funcSoftmax, *(MatMulresults[h]),*(Softmaxresults[h]),
			batchSize,batchSize,true) << endl;
			cout << "funcMatMul: " << funcTime(funcMatMul, 
			*(Softmaxresults[h]), *(FClayers[2+h*3]->getActivation()),
			*(Beforeconcat[h]), batchSize, batchSize, inputDim, 0, 0, FLOAT_PRECISION) << endl;

		}
			
		else{
			funcMatMul(*(FClayers[0+h*3]->getActivation()), *(FClayers[1+h*3]->getActivation()), 
			*(MatMulresults[h]), batchSize, inputDim, batchSize, 0, 1, FLOAT_PRECISION);
			funcSoftmax(*(MatMulresults[h]),*(Softmaxresults[h]),batchSize,batchSize,true);
			funcMatMul(*(Softmaxresults[h]), *(FClayers[2+h*3]->getActivation()),
			*(Beforeconcat[h]), batchSize, batchSize, inputDim, 0, 0, FLOAT_PRECISION);
			if (SECURITY_TYPE.compare("Malicious") == 0){
				funcMatMul(*(FClayers[0+h*3]->getActivation()), *(FClayers[1+h*3]->getActivation()), 
				*(MatMulresults[h]), batchSize, inputDim, batchSize, 0, 1, FLOAT_PRECISION);
				funcSoftmax(*(MatMulresults[h]),*(Softmaxresults[h]),batchSize,batchSize,true);
				funcMatMul(*(Softmaxresults[h]), *(FClayers[2+h*3]->getActivation()),
				*(Beforeconcat[h]), batchSize, batchSize, inputDim, 0, 0, FLOAT_PRECISION);
			}

		}
		
			
	}
	
	size_t size = inputDim*batchSize;
	for(size_t h = 0; h < nhead; h++){
		RSSVectorMyType temp = *(Beforeconcat[h]);
		for (size_t i = 0; i < size; i++){
			Afterconcat[h*size+i].first = temp[i].first;
			Afterconcat[h*size+i].second = temp[i].second;
		}
	}

	RSSVectorMyType temp = *(FClayers[nhead*3]->getActivation());
	FClayers[nhead*3]->forward(Afterconcat);	
	for(int i = 0; i< outputDim*batchSize;i++){
		Beforenorm[i].first = temp[i].first+(inputActivation[2])[i].first;
		Beforenorm[i].second = temp[i].first+(inputActivation[2])[i].second;
	}

	if (FUNCTION_TIME){
		cout << "funcLayerNorm: " << funcTime(funcLayerNorm, Beforenorm,activations,
		batchSize,outputDim) << endl;
	}
	else{
		funcLayerNorm(Beforenorm,activations,batchSize,outputDim);
	}
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcLayerNorm(Beforenorm,activations,batchSize,outputDim);

	

}


void MHALayer::computeDelta(RSSVectorMyType& prevDelta)
{
	// log_print("MHA.computeDelta");

	// //Back Propagate	
	// size_t rows = conf.batchSize;
	// size_t columns = conf.inputDim;
	// size_t common_dim = conf.outputDim;
	
	
	// if (FUNCTION_TIME)
	// 	cout << "funcMatMul: " << funcTime(funcMatMul, deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION) << endl;
	// else
	// 	funcMatMul(deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION);
	
}


void MHALayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	// log_print("MHA.updateEquations");

	// size_t rows = conf.batchSize;
	// size_t columns = conf.outputDim;
	// size_t common_dim = conf.inputDim;
	// size_t size = rows*columns;	
	// RSSVectorMyType temp(columns, std::make_pair(0,0));

	// //Update Biases
	// for (size_t i = 0; i < rows; ++i)
	// 	for (size_t j = 0; j < columns; ++j)
	// 		temp[j] = temp[j] + deltas[i*columns + j];

	// funcTruncate(temp, LOG_MINI_BATCH + LOG_LEARNING_RATE, columns);
	// subtractVectors<RSSMyType>(biases, temp, biases, columns);

	// //Update Weights 
	// rows = conf.inputDim;
	// columns = conf.outputDim;
	// common_dim = conf.batchSize;
	// size = rows*columns;
	// RSSVectorMyType deltaWeight(size);

	// if (FUNCTION_TIME)
	// 	cout << "funcMatMul: " << funcTime(funcMatMul, prevActivations, deltas, deltaWeight, rows, common_dim, columns, 1, 0, FLOAT_PRECISION + LOG_LEARNING_RATE + LOG_MINI_BATCH) << endl;
	// else
	// 	funcMatMul(prevActivations, deltas, deltaWeight, rows, common_dim, columns, 1, 0, 
	// 				FLOAT_PRECISION + LOG_LEARNING_RATE + LOG_MINI_BATCH);
	// subtractVectors<RSSMyType>(weights, deltaWeight, weights, size);		
}
