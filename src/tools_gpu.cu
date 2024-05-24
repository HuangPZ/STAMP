
#include "tools_gpu.h"

#if (USE_CUDA)
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>
#include <cuda.h>
#include <stdlib.h> 

#include "tools.h"

// using namespace std;


__global__ void matrixMulGPU(const myType *a, const myType *b, myType *c, size_t N, 
size_t m, size_t l, size_t transpose_a, size_t transpose_b) {
  // Compute each thread's global row and column index
  
	int row = (blockIdx.x * blockDim.x + threadIdx.x)%m;
	int col = (blockIdx.y * blockDim.y + threadIdx.y)%l;
	if (transpose_a){
		if (transpose_b){
			c[row * l + col] = 0;
			for (int k = 0; k < N; k++) {
				// Accumulate results for a single element
				c[row * l + col] += a[2*(row + k*m)] * b[2*(k + col*N)]+
									a[2*(row + k*m)+1] * b[2*(k + col*N)]+
									a[2*(row + k*m)] * b[2*(k + col*N)+1];
			}
		}
		else{
			c[row * l + col] = 0;
			for (int k = 0; k < N; k++) {
				// Accumulate results for a single element
				c[row * l + col] += a[2*(row + k*m)] * b[2*(k * l + col)]+
									a[2*(row + k*m)+1] * b[2*(k * l + col)]+
									a[2*(row + k*m)] * b[2*(k * l + col)+1];
			}
		}
	}
	else{
		if (transpose_b){
			c[row * l + col] = 0;
			for (int k = 0; k < N; k++) {
				// Accumulate results for a single element
				c[row * l + col] += a[2*(row * N + k)] * b[2*(k + col*N)]+
									a[2*(row * N + k)+1] * b[2*(k + col*N)]+
									a[2*(row * N + k)] * b[2*(k + col*N)+1];
			}
		}
		else{
			// Iterate over row, and down column
			c[row * l + col] = 0;
			for (int k = 0; k < N; k++) {
				// Accumulate results for a single element
				c[row * l + col] += a[2*(row * N + k)] * b[2*(k * l + col)]+
									a[2*(row * N + k)+1] * b[2*(k * l + col)]+
									a[2*(row * N + k)] * b[2*(k * l + col)+1];
			}
		}
	}
	
}

void matrixMultRSS_Cuda(const RSSVectorMyType &a, const RSSVectorMyType &b, vector<myType> &temp3, 
	size_t rows, size_t common_dim, size_t columns,size_t transpose_a, size_t transpose_b)
{

	// std::cout<<"!!!!!!!";
	myType *d_a, *d_b, *d_c;
	int myType_size = 4;
	int bytes = 1024;
	// std::cout<<myType_size*2*rows*common_dim<<std::endl;
	int size_a = myType_size*2*rows*common_dim;
	int size_b = myType_size*2*columns*common_dim;
	int size_c = myType_size*rows*columns;
	// std::cout<<size_a<<"??????????"<<size_b<<"??????????"<<size_c<<"??????????"<<std::endl;
	// std::cout<<rows<<"??????????"<<common_dim<<"??????????"<<columns<<"??????????"<<std::endl;
	cudaMalloc(&d_a, size_a);
	cudaMalloc(&d_b, size_b);
	cudaMalloc(&d_c, size_c);
	// for(int i =0;i<size_a;i++){
	// 	std::cout<<a[i].first<<a[i].second<<std::endl;
	// }
	// myType *d_a, *d_b, *d_c;

	// cudaMalloc(&d_a, myType_size*2*rows*common_dim);
	// cudaMalloc(&d_b, myType_size*2*columns*common_dim);
	// cudaMalloc(&d_c, myType_size*2*rows*columns);

	cudaMemcpy(d_a, &a[0], size_a, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, &b[0], size_b, cudaMemcpyHostToDevice);
	int THREADS = 32;
	dim3 threads(min(THREADS,(int)rows), min(THREADS,(int)columns));
	dim3 blocks((rows+THREADS-1)/THREADS, (columns+THREADS-1)/THREADS);
	matrixMulGPU<<<blocks, threads>>>(d_a, d_b, d_c, common_dim,rows,columns,transpose_a,transpose_b);

	cudaMemcpy(temp3.data(), d_c, size_c, cudaMemcpyDeviceToHost);


	cudaFree(d_a);
	cudaFree(d_b);
	cudaFree(d_c);

	// check multiplication correct
	// vector<myType> temp_send(rows*columns);
	// matrixMultRSS(a,b, temp_send, 
	// 	 rows,  common_dim,  columns,
	// 	  transpose_a,  transpose_b);
	// for(int i=0;i<columns*rows;i++){
	// 	// std::cout<<i<<" "<<temp_send[i]<<" "<<temp3[i]<<std::endl;
	// 	assert(temp_send[i] == temp3[i]);
	// }

}
__global__ void vectorMulGPU(const myType *a, myType *c, size_t rows, size_t columns ) {
	  // Compute each thread's global row and column index
	  
		int i = (blockIdx.x * blockDim.x + threadIdx.x)%rows;
		int j = (blockIdx.y * blockDim.y + threadIdx.y)%columns;
		
		for (int k = j; k < columns; ++k)
		{
			c[i*columns*columns + j*columns+k] = a[2*(i*columns + k)] * a[2*(i*columns + j)] +
									a[2*(i*columns + k)+1] * a[2*(i*columns + j)] +
									a[2*(i*columns + k)] * a[2*(i*columns + j)+1];
			c[i*columns*columns + j+k*columns] = c[i*columns*columns + j*columns+k];
		}
			

		
	}




void vectorMultRSS_Cuda(const RSSVectorMyType &a, vector<myType> &temp3, size_t rows, size_t columns)		
{
	myType *d_a, *d_b, *d_c;
	int myType_size = 4;
	int bytes = 1024;
	// std::cout<<myType_size*2*rows*common_dim<<std::endl;
	int size_a = myType_size*2*rows*columns;
	int size_c = myType_size*rows*columns*columns;
	// std::cout<<size_a<<"??????????"<<size_b<<"??????????"<<size_c<<"??????????"<<std::endl;
	// std::cout<<rows<<"??????????"<<common_dim<<"??????????"<<columns<<"??????????"<<std::endl;
	cudaMalloc(&d_a, size_a);
	cudaMalloc(&d_c, size_c);


	cudaMemcpy(d_a, &a[0], size_a, cudaMemcpyHostToDevice);

	int THREADS = 32;
	dim3 threads(min(THREADS,(int)rows), min(THREADS,(int)columns));
	dim3 blocks((rows+THREADS-1)/THREADS, (columns+THREADS-1)/THREADS);
	vectorMulGPU<<<blocks, threads>>>(d_a, d_c, rows,columns);

	cudaMemcpy(temp3.data(), d_c, size_c, cudaMemcpyDeviceToHost);


	cudaFree(d_a);

	cudaFree(d_c);



}

#endif