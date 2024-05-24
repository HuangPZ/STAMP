
#pragma once
#include "LayerConfig.h"

#include "globals.h"
using namespace std;

class MHAConfig : public LayerConfig
{
public:
	size_t inputDim = 0;
	size_t batchSize = 0;
	size_t outputDim = 0;
	size_t nhead = 0;
	bool masked = false;


	MHAConfig(size_t _inputDim, size_t _batchSize, size_t _outputDim, size_t _nhead, bool _masked )
	:inputDim(_inputDim),
	 batchSize(_batchSize), 
	 outputDim(_outputDim),
	 nhead(_nhead),
	 masked(_masked),
	 LayerConfig("MHA")
	{};
};
