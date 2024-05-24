#pragma once
#include "MaxpoolLayer.h"
#include "Functionalities.h"
using namespace std;


MaxpoolLayer::MaxpoolLayer(MaxpoolConfig* conf, int _layerNum)
:Layer(_layerNum),
 conf(conf->imageHeight, conf->imageWidth, conf->features, 
	  conf->poolSize, conf->stride, conf->batchSize),
 activations(conf->batchSize * conf->features * 
		    (((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
 		    (((conf->imageHeight - conf->poolSize)/conf->stride) + 1)),
 deltas(conf->batchSize * conf->features * 
	   (((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
	   (((conf->imageHeight - conf->poolSize)/conf->stride) + 1)),
 maxPrime((((conf->imageWidth - conf->poolSize)/conf->stride) + 1) * 
		 (((conf->imageHeight - conf->poolSize)/conf->stride) + 1) * 
		 conf->features * conf->batchSize * conf->poolSize * conf->poolSize)
{};


void MaxpoolLayer::printLayer(std::string fn)
{
	cout << "----------------------------------------------" << endl;  	
	cout << "(" << layerNum+1 << ") Maxpool Layer\t  " << conf.imageHeight << " x " << conf.imageWidth 
		 << " x " << conf.features << endl << "\t\t\t  " 
		 << conf.poolSize << "  \t\t(Pooling Size)" << endl << "\t\t\t  " 
		 << conf.stride << " \t\t(Stride)" << endl << "\t\t\t  " 
		 << conf.batchSize << "\t\t(Batch Size)" << endl;
	ofstream myfile;
	// myfile.open (fn.c_str(),fstream::app);
	// myfile << "----------------------------------------------" << endl;  	
	// myfile << "(" << layerNum+1 << ") Maxpool Layer\t  " << conf.imageHeight << " x " << conf.imageWidth 
	// 	 << " x " << conf.features << endl << "\t\t\t  " 
	// 	 << conf.poolSize << "  \t\t(Pooling Size)" << endl << "\t\t\t  " 
	// 	 << conf.stride << " \t\t(Stride)" << endl << "\t\t\t  " 
	// 	 << conf.batchSize << "\t\t(Batch Size)" << endl;
	// myfile.close();
}

void MaxpoolLayer::forward(const RSSVectorMyType& inputActivation)
{
	log_print("Maxpool.forward");

	size_t B 	= conf.batchSize;
	size_t iw 	= conf.imageWidth;
	size_t ih 	= conf.imageHeight;
	size_t f 	= conf.poolSize;
	size_t Din 	= conf.features;
	size_t S 	= conf.stride;
	size_t ow 	= (((iw-f)/S)+1);
	size_t oh	= (((ih-f)/S)+1);
	
	size_t pad_iw = S-iw%S;
	size_t pad_ih = S-ih%S;
	size_t P = pad_iw/2;
	RSSVectorMyType temp0((iw+pad_iw)*(ih+pad_ih)*Din*B, make_pair(0,0));
	zeroPad(inputActivation, temp0, iw, ih, P, Din, B);

	//Reshape for convolution
	RSSVectorMyType temp1(ow*oh*Din*B*f*f);
	{
		size_t sizeBeta = iw;
		size_t sizeD 	= sizeBeta*ih;
		size_t sizeB 	= sizeD*Din;
		size_t counter 	= 0;
		for (int b = 0; b < B; ++b)
			for (size_t r = 0; r < Din; ++r)
				for (size_t beta = 0; beta < ih-f+1; beta+=S) 
					for (size_t alpha = 0; alpha < iw-f+1; alpha+=S)
						for (int q = 0; q < f; ++q)
							for (int p = 0; p < f; ++p)
							{
								temp1[counter++] = 
									temp0[b*sizeB + r*sizeD + 
										(beta + q)*sizeBeta + (alpha + p)];
							}
	}

	//Pooling operation
	if (FUNCTION_TIME)
		cout << "funcMaxpool: " << funcTime(funcMaxpool, temp1, activations, maxPrime, ow*oh*Din*B, f*f) << endl;
	else
		funcMaxpool(temp1, activations, maxPrime, ow*oh*Din*B, f*f);
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcMaxpool(temp1, activations, maxPrime, ow*oh*Din*B, f*f);
}


void MaxpoolLayer::computeDelta(RSSVectorMyType& prevDelta)
{
	log_print("Maxpool.computeDelta");

	size_t B 	= conf.batchSize;
	size_t iw 	= conf.imageWidth;
	size_t ih 	= conf.imageHeight;
	size_t f 	= conf.poolSize;
	size_t Din 	= conf.features;
	size_t S 	= conf.stride;
	size_t ow 	= (((iw-f)/S)+1);
	size_t oh	= (((ih-f)/S)+1);

	RSSVectorSmallType temp1(iw*ih*Din*B);	//Contains maxPrime reordered
	RSSVectorMyType temp2(iw*ih*Din*B);		//Contains Delta reordered
	{
		size_t sizeY 	= iw;
		size_t sizeD 	= sizeY*ih;
		size_t sizeB 	= sizeD*Din;
		size_t counter1 = 0;
		size_t counter2 = 0;

		for (int b = 0; b < B; ++b)
			for (size_t r = 0; r < Din; ++r)
				for (int y = 0; y < oh; ++y)
					for (int x = 0; x < ow; ++x)
					{
						for (int q = 0; q < f; ++q)
						{
							for (int p = 0; p < f; ++p)
							{
								temp1[b*sizeB + r*sizeD + 
									(y*S + q)*sizeY + (x*S + p)] = 
								maxPrime[counter1++];

								temp2[b*sizeB + r*sizeD + 
									(y*S + q)*sizeY + (x*S + p)] = 
								deltas[counter2];
							}
						}
						counter2++;
					}
	}

	if (FUNCTION_TIME)
		cout << "funcSelectShares: " << funcTime(funcSelectShares, temp2, temp1, prevDelta, iw*ih*Din*B) << endl;
	else
		funcSelectShares(temp2, temp1, prevDelta, iw*ih*Din*B);
}

void MaxpoolLayer::updateEquations(const RSSVectorMyType& prevActivations)
{
	log_print("Maxpool.updateEquations");
}
