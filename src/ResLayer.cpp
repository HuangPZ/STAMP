
#pragma once
#include "ResLayer.h"

#include "Functionalities.h"
using namespace std;

ResLayer::ResLayer(ResConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf(conf->inputDim, conf->batchSize, conf->outputDim,conf->convNum, conf->groupNum, 
 conf->filterNum,conf->inputFeatures, conf->filterSize, conf-> half_out),
 activations(conf->batchSize * conf->outputDim), 
 deltas(conf->batchSize * conf->outputDim),
 weights(conf->inputDim * conf->outputDim),
 biases(conf->outputDim)
{
	CNNConfig *cfgC;
	BNConfig *cfgB;
	ReLUConfig *cfgR;
	size_t inputDim;
	if (conf->half_out){
		cfgC = new CNNConfig(conf->inputDim,conf->inputDim,conf->inputFeatures,
		conf->filterNum,conf->filterSize,2, floor((conf->filterSize-1)/2),MINI_BATCH_SIZE,0,1,1);
		// cfgB = new BNConfig((int)(conf->inputDim/2*conf->inputDim/2*conf->filterNum),MINI_BATCH_SIZE);
		// cfgR = new ReLUConfig((int)(conf->inputDim/2*conf->inputDim/2*conf->filterNum),MINI_BATCH_SIZE);
		inputDim = conf->inputDim/2;
	}
		
	else{
		cfgC = new CNNConfig(conf->inputDim,conf->inputDim,conf->inputFeatures,
		conf->filterNum,conf->filterSize,1, floor((conf->filterSize-1)/2),MINI_BATCH_SIZE,0,1,1);
		// cfgB = new BNConfig((int)(conf->inputDim*conf->inputDim*conf->filterNum),MINI_BATCH_SIZE);
		// cfgR = new ReLUConfig((int)(conf->inputDim*conf->inputDim*conf->filterNum),MINI_BATCH_SIZE);
		inputDim = conf->inputDim;
	}
	

	CNNlayers.push_back(new CNNLayer(cfgC, 0));
	// BNlayers.push_back(new BNLayer(cfgB, 1));
	// ReLULayers.push_back(new ReLULayer(cfgR, 2));
	delete cfgC,cfgB,cfgR;
	

	for(size_t i = 1; i < conf->convNum;i++){
		
		if(i%conf->groupNum==conf->groupNum-1){
			CNNConfig *cfg = new CNNConfig(inputDim,inputDim,conf->filterNum,
			conf->filterNum,conf->filterSize,1, floor((conf->filterSize-1)/2),MINI_BATCH_SIZE,0,0,1);
			CNNlayers.push_back(new CNNLayer(cfg, i*3));

			ReLUConfig* cfgR = new ReLUConfig((int)(inputDim*inputDim*conf->filterNum),MINI_BATCH_SIZE);
			ReLULayers.push_back(new ReLULayer(cfgR, i*3+2));
		}
		else{
			CNNConfig *cfg = new CNNConfig(inputDim,inputDim,conf->filterNum,
			conf->filterNum,conf->filterSize,1, floor((conf->filterSize-1)/2),MINI_BATCH_SIZE,0,1,1);
			CNNlayers.push_back(new CNNLayer(cfg, i*3));

		}
		delete cfgR,cfgB;
	}
	for (size_t i = 1; i < conf->convNum;i++){
		
	}
	initialize();
}


void ResLayer::initialize()
{
	//already run in each layer 
}


void ResLayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") Res Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
		 << conf.batchSize << "\t\t (Batch Size)" << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") Res Layer\t\t  " << conf.inputDim << " x " << conf.outputDim << endl << "\t\t\t  "
	// 	 << conf.batchSize << "\t\t (Batch Size)" << endl;
	// myfile.close();
}


void ResLayer::forward(const RSSVectorMyType &inputActivation)
{
	log_print("Res.forward");

	size_t convNum = conf.convNum;
	CNNlayers[0]->forward(inputActivation);
	// cout<<"forward1"<<endl;
	// BNlayers[0]->forward(*(CNNlayers[0]->getActivation()));
	// ReLULayers[0]->forward(*(CNNlayers[0]->getActivation()));
	int relucounter = 0;
	for(size_t h = 1; h < convNum; h++){
		if(h%conf.groupNum==0)
			CNNlayers[h]->forward(*(ReLULayers[relucounter-1]->getActivation()));
		else
			CNNlayers[h]->forward(*(CNNlayers[h-1]->getActivation()));

		// BNlayers[h]->forward(*(CNNlayers[h]->getActivation()));
		if(h%conf.groupNum==conf.groupNum-1){
			size_t size = (*(CNNlayers[h]->getActivation())).size();
			RSSVectorMyType temp_sum(size);
			if(h!=1){

				for(int i = 0; i<size;i++){
					temp_sum[i].first = (*(CNNlayers[h]->getActivation()))[i].first + (*(CNNlayers[h-conf.groupNum]->getActivation()))[i].first;
					temp_sum[i].second = (*(CNNlayers[h]->getActivation()))[i].second + (*(CNNlayers[h-conf.groupNum]->getActivation()))[i].second;
				}
				ReLULayers[relucounter]->forward(temp_sum);
				relucounter++;
			}
			else{//TODO: Sum of differrence dim

				ReLULayers[relucounter]->forward(*(CNNlayers[h]->getActivation()));
				relucounter++;
			}

			
		}
		// else{
		// 	// ReLULayers[h]->forward(*(BNlayers[h]->getActivation()));
		// }

			
		
	}

	activations = *(ReLULayers[relucounter-1]->getActivation());


}


void ResLayer::computeDelta(RSSVectorMyType& prevDelta)
{
	// log_print("Res.computeDelta");

	// //Back Propagate	
	// size_t rows = conf.batchSize;
	// size_t columns = conf.inputDim;
	// size_t common_dim = conf.outputDim;
	
	
	// if (FUNCTION_TIME)
	// 	cout << "funcMatMul: " << funcTime(funcMatMul, deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION) << endl;
	// else
	// 	funcMatMul(deltas, weights, prevDelta, rows, common_dim, columns, 0, 1, FLOAT_PRECISION);
	
}


void ResLayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	// log_print("Res.updateEquations");

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
