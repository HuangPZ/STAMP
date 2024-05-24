
#pragma once
#include "LayerConfig.h"
#include "globals.h"
using namespace std;

class AvgConfig : public LayerConfig
{
public:
	size_t inputDim = 0;
	size_t batchSize = 0;

	AvgConfig(size_t _inputDim, size_t _batchSize)
	:inputDim(_inputDim),
	 batchSize(_batchSize), 
	 LayerConfig("Avg")
	{};
};
