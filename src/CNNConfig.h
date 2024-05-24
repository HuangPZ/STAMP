
#pragma once
#include "LayerConfig.h"
#include "globals.h"
using namespace std;

class CNNConfig : public LayerConfig
{
public:
	size_t imageHeight = 0;
	size_t imageWidth = 0;

	size_t inputFeatures = 0;	//#Input feature maps
	size_t filters = 0;			//#Output feature maps
	size_t filterSize = 0;

	size_t stride = 0;
	size_t padding = 0;
	size_t batchSize = 0;

	size_t poolSizeX = 1;
	size_t poolSizeY = 1;

	bool maxpool = 0;
	bool relu = 0;
	bool BN = 0;

	size_t poolSize = 0;
	size_t stride_M = 0;

	CNNConfig(size_t _imageHeight, size_t _imageWidth, size_t _inputFeatures, size_t _filters, 
	size_t _filterSize, size_t _stride, size_t _padding, size_t _batchSize,
	bool _maxpool = 0, bool _relu = 0, bool _BN = 0, size_t _poolSize = 0, size_t _stride_M = 0)
	:imageHeight(_imageHeight),
	 imageWidth(_imageWidth),
	 inputFeatures(_inputFeatures),
	 filters(_filters),
	 filterSize(_filterSize),
	 stride(_stride),
	 padding(_padding),
	 batchSize(_batchSize),
	 maxpool(_maxpool),
	 relu(_relu),
	 BN(_BN),
	 poolSize(_poolSize),
	 stride_M(_stride_M),
	 LayerConfig("CNN")
	{
		assert(((imageWidth - filterSize + 2*padding)%stride == 0 || ((imageWidth - filterSize + 2*padding)%stride == 1 && imageWidth%2==0 ||filterSize%2 ==1))&& "CNN parameter check failed");
		assert(((imageHeight - filterSize + 2*padding)%stride == 0 || ((imageHeight - filterSize + 2*padding)%stride == 1 && imageHeight%2==0 ||filterSize%2 ==1))&& "CNN parameter check failed");
	};
};
