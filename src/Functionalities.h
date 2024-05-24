
#pragma once
#include "tools.h"
#include "tools_gpu.h"
#include "connect.h"
#include "globals.h"
#include "Chip.h"
#include "CPU.h"
using namespace std;

extern void start_time();
extern void start_communication();
extern void end_time(string str, string fn);
extern void end_communication(string str,string fn);



void funcTruncate(RSSVectorMyType &a, size_t power, size_t size);
void funcTruncatePublic(RSSVectorMyType &a, size_t divisor, size_t size);
void funcGetShares(RSSVectorMyType &a, const vector<myType> &data);
void funcGetShares(RSSVectorSmallType &a, const vector<smallType> &data);
int funcReconstructBit(const RSSVectorSmallType &a, vector<smallType> &b, size_t size, string str, bool print);
void funcReconstruct(const RSSVectorMyType &a, vector<myType> &b, size_t size, string str, bool print);
void funcReconstruct(const RSSVectorSmallType &a, vector<smallType> &b, size_t size, string str, bool print);
int funcReconstruct3out3(const vector<myType> &a, vector<myType> &b, size_t size, string str, bool print);
void funcMaxReLU(const vector<myType> &a, RSSVectorMyType &b, RSSVectorSmallType &prime, size_t B, size_t tempSize, size_t Dout, size_t truncation,
						bool maxpool = 0, bool ReLU = 0, bool BN = 0, const RSSVectorMyType &bias = RSSVectorMyType(1), size_t poolSize = 0, size_t stride_M = 0,
						bool Is_CNN = false, const RSSVectorMyType &gamma = RSSVectorMyType(1), const RSSVectorMyType &beta = RSSVectorMyType(1));
void funcMatMul(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &c, 
				size_t rows, size_t common_dim, size_t columns,
			 	size_t transpose_a, size_t transpose_b, size_t truncation);
void funcMatMul_mixed(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &c, RSSVectorSmallType &prime,
				size_t rows, size_t common_dim, size_t columns,
			 	size_t transpose_a, size_t transpose_b, size_t truncation,
				size_t B, size_t Dout, size_t tempSize,
				bool is_CNN=0, bool maxpool = 0, bool ReLU = 0, bool BN = 0, 
				const RSSVectorMyType &bias = RSSVectorMyType(1), size_t poolSize = 0, size_t stride_M = 0,
				const RSSVectorMyType &gamma = RSSVectorMyType(1),const RSSVectorMyType &beta = RSSVectorMyType(1));
int* funcDotProduct(const RSSVectorMyType &a, const RSSVectorMyType &b, 
					   RSSVectorMyType &c, size_t size, bool truncation, size_t precision);
int* funcDotProduct(const RSSVectorSmallType &a, const RSSVectorSmallType &b, 
							 RSSVectorSmallType &c, size_t size);
int* funcPrivateCompare(const RSSVectorSmallType &share_m, const vector<myType> &r, 
							  const RSSVectorSmallType &beta, vector<smallType> &betaPrime, 
							  size_t size);
int* funcWrap(const RSSVectorMyType &a, RSSVectorSmallType &theta, size_t size);
void funcSelectShares(const RSSVectorMyType &a, const RSSVectorSmallType &b, RSSVectorMyType &selected, size_t size);
int funcSelectBitShares(const RSSVectorSmallType &a0, const RSSVectorSmallType &a1, 
						 const RSSVectorSmallType &b, RSSVectorSmallType &answer, 
						 size_t rows, size_t columns, size_t loopCounter);
int* funcRELUPrime(const RSSVectorMyType &a, RSSVectorSmallType &b, size_t size);
int* funcRELU(const RSSVectorMyType &a, RSSVectorSmallType &temp, RSSVectorMyType &b, size_t size);
int* funcPow(const RSSVectorMyType &b, vector<smallType> &alpha, size_t size);
void funcDivision(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &quotient, 
							size_t size);
void funcBatchNorm(const RSSVectorMyType &inputActivation, 
				const RSSVectorMyType &gamma, const RSSVectorMyType &beta, RSSVectorMyType &b,
				 size_t EPSILON,  size_t m,  size_t B);
void funcMaxpool(RSSVectorMyType &a, RSSVectorMyType &max, RSSVectorSmallType &maxPrime, 
				size_t rows, size_t columns);
void funcAvgpool(const RSSVectorMyType &a, RSSVectorMyType &b, size_t rows, size_t columns);
void funcMaxpool2(const RSSVectorMyType &a, RSSVectorMyType &max, RSSVectorSmallType &maxPrime, 
				size_t rows, size_t columns);
int* funcSoftmax(const RSSVectorMyType &a,  RSSVectorMyType &b, size_t rows, size_t columns, bool masked = false  );
void funcVecMul(const RSSVectorMyType &a, RSSVectorMyType &c, 
					size_t rows, size_t columns,size_t truncation);
int* funcLayerNorm(const RSSVectorMyType &a, RSSVectorMyType &b, size_t rows, size_t columns);

void aggregateCommunication(std::string fn);
template <typename T>
void mask_Gen(T d, size_t size);
template <typename T>
void share_Gen(T d, size_t size);


//Debug
void debugMatMul();
void debugDotProd();
void debugPC();
void debugWrap();
void debugReLUPrime();
void debugReLU();
void debugDivision();
void debugBN();
void debugSSBits();
void debugSS();
void debugMaxpool();
void debugAvgpool();
void debugVecMul();
void debugSoftmax();
void debugLN();
void debugMatMulMix();

//Test
void testMatMul(size_t rows, size_t common_dim, size_t columns, size_t iter);
void testConvolution(size_t iw, size_t ih, size_t Din, size_t Dout, 
					size_t f, size_t S, size_t P, size_t B, size_t iter);
void testRelu(size_t r, size_t c, size_t iter);
void testReluPrime(size_t r, size_t c, size_t iter);
void testMaxpool(size_t ih, size_t iw, size_t Din, size_t f, size_t S, size_t B, size_t iter);
