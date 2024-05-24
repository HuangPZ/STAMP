
#pragma once
#include "LayerConfig.h"
#include "globals.h"
using namespace std;

class FCConfig : public LayerConfig
{
public:
	size_t inputDim = 0;
	size_t batchSize = 0;
	size_t outputDim = 0;
	bool relu = 0;

	size_t poolSize = 0;
	size_t stride_M = 0;
	FCConfig(size_t _inputDim, size_t _batchSize, size_t _outputDim,
	bool _relu = 0)
	:inputDim(_inputDim),
	 batchSize(_batchSize), 
	 outputDim(_outputDim),
	 relu(_relu),
	 LayerConfig("FC")
	{};
};
