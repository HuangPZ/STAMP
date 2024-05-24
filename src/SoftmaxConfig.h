
#pragma once
#include "LayerConfig.h"
#include "globals.h"
using namespace std;

class SoftmaxConfig : public LayerConfig
{
public:
	size_t inputDim = 0;
	size_t batchSize = 0;
	bool masked = false;

	SoftmaxConfig(size_t _inputDim, size_t _batchSize, bool _masked = false)
	:inputDim(_inputDim),
	 batchSize(_batchSize), 
	 masked(_masked),
	 LayerConfig("Softmax")
	{};
};
