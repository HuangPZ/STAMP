
#pragma once
#include "LayerConfig.h"

#include "globals.h"
using namespace std;

class ResConfig : public LayerConfig
{
public:
	size_t inputDim = 0;
	size_t batchSize = 0;
	size_t outputDim = 0;
	size_t convNum = 0;
	size_t groupNum = 0;
	size_t filterNum = 0;
	size_t inputFeatures = 0;	//#Input feature maps
	size_t filterSize = 0;
	size_t half_out = 0;


	ResConfig(size_t _inputDim, size_t _batchSize, size_t _outputDim, size_t _convNum, size_t _groupNum, size_t _filterNum,
	size_t _inputFeatures,size_t _filterSize,size_t _half_out)
	:inputDim(_inputDim),
	 batchSize(_batchSize), 
	 outputDim(_outputDim),
	 convNum(_convNum),
	 groupNum(_groupNum),
	 filterNum(_filterNum),
	 inputFeatures(_inputFeatures),
	 filterSize(_filterSize),
	 half_out(_half_out),
	 LayerConfig("Res")
	{};
};
