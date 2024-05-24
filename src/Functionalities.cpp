
#pragma once
#include "Functionalities.h"
#include "Precompute.h"
#include <thread>


using namespace std;
extern Precompute PrecomputeObject;
extern string SECURITY_TYPE;
int functype = 2;
extern CPU cpu;
extern float sendtime;
Chip chip;
struct timespec starttimespec,endtimespec;
/******************************** Functionalities 2PC ********************************/
// Share Truncation, truncate shares of a by power (in place) (power is logarithmic)
void funcTruncate(RSSVectorMyType &a, size_t power, size_t size)
{
	log_print("funcTruncate");

	RSSVectorMyType r(size), rPrime(size);
	vector<myType> reconst(size);
	PrecomputeObject.getDividedShares(r, rPrime, (1<<power), size);
	for (int i = 0; i < size; ++i)
		a[i] = a[i] - rPrime[i];
	cpu.CPU_Add(size);
	
	funcReconstruct(a, reconst, size, "Truncate reconst", false);
	dividePlain(reconst, (1 << power));
	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first + reconst[i];
			a[i].second = r[i].second;
		}
		cpu.CPU_Add(size);
	}

	if (partyNum == PARTY_B)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first;
			a[i].second = r[i].second;
		}
	}

	if (partyNum == PARTY_C)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first;
			a[i].second = r[i].second + reconst[i];
		}
		cpu.CPU_Add(size);
	}	
}

void funcTruncatePublic(RSSVectorMyType &a, size_t divisor, size_t size)
{
	log_print("funcTruncate");

	RSSVectorMyType r(size), rPrime(size);
	vector<myType> reconst(size);
	PrecomputeObject.getDividedShares(r, rPrime, divisor, size);
	for (int i = 0; i < size; ++i)
		a[i] = a[i] - rPrime[i];
	cpu.CPU_Add(size);
	
	funcReconstruct(a, reconst, size, "Truncate reconst", false);
	dividePlain(reconst, divisor);
	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first + reconst[i];
			a[i].second = r[i].second;
		}
		cpu.CPU_Add(size);
	}

	if (partyNum == PARTY_B)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first;
			a[i].second = r[i].second;
		}
		
	}

	if (partyNum == PARTY_C)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = r[i].first;
			a[i].second = r[i].second + reconst[i];
		}
		cpu.CPU_Add(size);
	}	
}


//Fixed-point data has to be processed outside this function.
void funcGetShares(RSSVectorMyType &a, const vector<myType> &data)
{
	size_t size = a.size();

	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = data[i];
			a[i].second = 0;
		}
	}
	else if (partyNum == PARTY_B)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = 0;
			a[i].second = 0;
		}
	}
	else if (partyNum == PARTY_C)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = 0;
			a[i].second = data[i];
		}
	}
}


void funcGetShares(RSSVectorSmallType &a, const vector<smallType> &data)
{
	size_t size = a.size();
	
	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = data[i];
			a[i].second = 0;
		}
	}
	else if (partyNum == PARTY_B)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = 0;
			a[i].second = 0;
		}
	}
	else if (partyNum == PARTY_C)
	{
		for (int i = 0; i < size; ++i)
		{
			a[i].first = 0;
			a[i].second = data[i];
		}
	}
}


int funcReconstructBit(const RSSVectorSmallType &a, vector<smallType> &b, size_t size, string str, bool print)
{
	int sent;
	log_print("Reconst: RSSSmallType (bits), smallType (bit)");

	if (SECURITY_TYPE.compare("Semi-honest") == 0)
	{
		vector<smallType> a_next(size), a_prev(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev[i] = 0;
			a_next[i] = a[i].first;
			b[i] = a[i].first;
			b[i] = b[i] ^ a[i].second;
		}
        cpu.CPU_Gate(size);

		thread *threads = new thread[2];
		threads[0] = thread(sendVector<smallType>, ref(a_next), nextParty(partyNum), size);
		sent+=sizeof(smallType)*size;
		threads[1] = thread(receiveVector<smallType>, ref(a_prev), prevParty(partyNum), size);
		for (int i = 0; i < 2; i++)
			threads[i].join();
		delete[] threads;

		for (int i = 0; i < size; ++i)
			b[i] = b[i] ^ a_prev[i];
        cpu.CPU_Gate(size);
		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				cout << (int)(b[i]) << " "; 
			std::cout << std::endl;
		}
	}

	if (SECURITY_TYPE.compare("Malicious") == 0)
	{
		vector<smallType> a_next_send(size), a_prev_send(size), a_next_recv(size), a_prev_recv(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev_send[i] = a[i].second;
			a_next_send[i] = a[i].first;
			b[i] = a[i].first;
			b[i] = b[i] ^ a[i].second;
		}
        cpu.CPU_Gate(size);

		thread *threads = new thread[4];
		threads[0] = thread(sendVector<smallType>, ref(a_next_send), nextParty(partyNum), size);
		sent+=sizeof(smallType)*size;
		threads[1] = thread(sendVector<smallType>, ref(a_prev_send), prevParty(partyNum), size);
		sent+=sizeof(smallType)*size;
		threads[2] = thread(receiveVector<smallType>, ref(a_next_recv), nextParty(partyNum), size);
		threads[3] = thread(receiveVector<smallType>, ref(a_prev_recv), prevParty(partyNum), size);
		sendRoundfix(partyNum,1);
		receiveRoundfix(partyNum,1);
		for (int i = 0; i < 4; i++)
			threads[i].join();
		delete[] threads;

		for (int i = 0; i < size; ++i)
		{
			if (a_next_recv[i] != a_prev_recv[i])
			{
				// error("Malicious behaviour detected");
			}
			b[i] = b[i] ^ a_prev_recv[i];
		}
        cpu.CPU_Gate(size);

		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				cout << (int)(b[i]) << " "; 
			std::cout << std::endl;
		}
	}
	return sent;
}


void funcReconstruct(const RSSVectorSmallType &a, vector<smallType> &b, size_t size, string str, bool print)
{
	log_print("Reconst: RSSSmallType, smallType");

	if (SECURITY_TYPE.compare("Semi-honest") == 0)
	{
		vector<smallType> a_next(size), a_prev(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev[i] = 0;
			a_next[i] = a[i].first;
			b[i] = a[i].first;
			b[i] = additionModPrime[b[i]][a[i].second];
		}
        cpu.CPU_Add(size);

		thread *threads = new thread[2];

		threads[0] = thread(sendVector<smallType>, ref(a_next), nextParty(partyNum), size);
		threads[1] = thread(receiveVector<smallType>, ref(a_prev), prevParty(partyNum), size);

		for (int i = 0; i < 2; i++)
			threads[i].join();

		delete[] threads;

		for (int i = 0; i < size; ++i)
			b[i] = additionModPrime[b[i]][a_prev[i]];
        cpu.CPU_Add(size);
		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				cout << (int)(b[i]) << " "; 
			std::cout << std::endl;
		}
	}

	if (SECURITY_TYPE.compare("Malicious") == 0)
	{
		vector<smallType> a_next_send(size), a_prev_send(size), a_next_recv(size), a_prev_recv(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev_send[i] = a[i].second;
			a_next_send[i] = a[i].first;
			b[i] = a[i].first;
			// b[i] = additionModPrime[b[i]][a[i].second];
		}

		thread *threads = new thread[4];
		threads[0] = thread(sendVector<smallType>, ref(a_next_send), nextParty(partyNum), size);
		threads[1] = thread(sendVector<smallType>, ref(a_prev_send), prevParty(partyNum), size);
		threads[2] = thread(receiveVector<smallType>, ref(a_next_recv), nextParty(partyNum), size);
		threads[3] = thread(receiveVector<smallType>, ref(a_prev_recv), prevParty(partyNum), size);
		sendRoundfix(partyNum,1);
		receiveRoundfix(partyNum,1);
		for (int i = 0; i < 4; i++)
			threads[i].join();
		delete[] threads;

		for (int i = 0; i < size; ++i)
		{
			if (a_next_recv[i] != a_prev_recv[i])
			{
				// error("Malicious behaviour detected");
			}
			// b[i] = additionModPrime[b[i]][a_prev_recv[i]];
		}

		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				cout << (int)(b[i]) << " "; 
			std::cout << std::endl;
		}
	}
}


void funcReconstruct(const RSSVectorMyType &a, vector<myType> &b, size_t size, string str, bool print)
{
	
	log_print("Reconst: RSSMyType, myType");
	assert(a.size() == size && "a.size mismatch for reconstruct function");

	if (SECURITY_TYPE.compare("Semi-honest") == 0)
	{
		vector<myType> a_next(size), a_prev(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev[i] = 0;
			a_next[i] = a[i].first;
			b[i] = a[i].first;
			b[i] = b[i] + a[i].second;
		}
        cpu.CPU_Add(size);

		thread *threads = new thread[2];

		threads[0] = thread(sendVector<myType>, ref(a_next), nextParty(partyNum), size);
		threads[1] = thread(receiveVector<myType>, ref(a_prev), prevParty(partyNum), size);

		for (int i = 0; i < 2; i++)
			threads[i].join();

		delete[] threads;

		for (int i = 0; i < size; ++i)
			b[i] = b[i] + a_prev[i];
        cpu.CPU_Add(size);
		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				print_linear(b[i], "SIGNED");
			std::cout << std::endl;
		}
	}

	if (SECURITY_TYPE.compare("Malicious") == 0)
	{
		vector<smallType> a_next_send(size), a_prev_send(size), a_next_recv(size), a_prev_recv(size);
		for (int i = 0; i < size; ++i)
		{
			a_prev_send[i] = a[i].second;
			a_next_send[i] = a[i].first;
			b[i] = a[i].first;
			b[i] = b[i] + a[i].second;
		}

		thread *threads = new thread[4];
		threads[0] = thread(sendVector<smallType>, ref(a_next_send), nextParty(partyNum), size);
		threads[1] = thread(sendVector<smallType>, ref(a_prev_send), prevParty(partyNum), size);
		
		threads[2] = thread(receiveVector<smallType>, ref(a_next_recv), nextParty(partyNum), size);
		threads[3] = thread(receiveVector<smallType>, ref(a_prev_recv), prevParty(partyNum), size);
		sendRoundfix(partyNum,1);
		receiveRoundfix(partyNum,1);
		for (int i = 0; i < 4; i++)
			threads[i].join();
		delete[] threads;

		for (int i = 0; i < size; ++i)
		{
			if (a_next_recv[i] != a_prev_recv[i])
			{
				// error("Malicious behaviour detected");
			}
			b[i] = b[i] + a_prev_recv[i];
		}

		if (print)
		{
			std::cout << str << ": \t\t";
			for (int i = 0; i < size; ++i)
				print_linear(b[i], "SIGNED");
			std::cout << std::endl;
		}
	}
}

void funcCheckMaliciousMatMul(const RSSVectorMyType &a, const RSSVectorMyType &b, const RSSVectorMyType &c, 
							const vector<myType> &temp, size_t rows, size_t common_dim, size_t columns,
							size_t transpose_a, size_t transpose_b)
{
	RSSVectorMyType x(a.size()), y(b.size()), z(c.size());
	PrecomputeObject.getTriplets(x, y, z, rows, common_dim, columns);

	subtractVectors<RSSMyType>(x, a, x, rows*common_dim);
	subtractVectors<RSSMyType>(y, b, y, common_dim*columns);

	size_t combined_size = rows*common_dim + common_dim*columns, base_size = rows*common_dim;
	RSSVectorMyType combined(combined_size); 
	vector<myType> rhoSigma(combined_size), rho(rows*common_dim), sigma(common_dim*columns), temp_send(rows*columns);
	for (int i = 0; i < rows*common_dim; ++i)
		combined[i] = x[i];

	for (int i = rows*common_dim; i < combined_size; ++i)
		combined[i] = y[i-base_size];

	funcReconstruct(combined, rhoSigma, combined_size, "rhoSigma", false);

	for (int i = 0; i < rows*common_dim; ++i)
		rho[i] = rhoSigma[i];

	for (int i = rows*common_dim; i < combined_size; ++i)
		sigma[i-base_size] = rhoSigma[i];

	//Doing x times sigma, rho times y, and rho times sigma
	#if (USE_CUDA)
	// cout<<"11111111111"<<endl;
	matrixMultRSS_Cuda(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	matrixMultRSS_Cuda(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	matrixMultRSS_Cuda(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	#else
	matrixMultRSS(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	matrixMultRSS(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	matrixMultRSS(x, y, temp_send, rows, common_dim, columns, transpose_a, transpose_b);
	#endif


	size_t size = rows*columns;
	vector<myType> temp_recv(size);

	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(temp_send), nextParty(partyNum), size);
	threads[1] = thread(receiveVector<myType>, ref(temp_recv), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		if (temp[i] == temp_recv[i])
		{
			//Do the abort thing	
		}
}

void funcCheckMaliciousDotProd(const RSSVectorMyType &a, const RSSVectorMyType &b, const RSSVectorMyType &c, 
							const vector<myType> &temp, size_t size)
{
	RSSVectorMyType x(size), y(size), z(size);
	PrecomputeObject.getTriplets(x, y, z, size);

	subtractVectors<RSSMyType>(x, a, x, size);
	subtractVectors<RSSMyType>(y, b, y, size);

	size_t combined_size = 2*size;
	RSSVectorMyType combined(combined_size); 
	vector<myType> rhoSigma(combined_size), rho(size), sigma(size), temp_send(size);
	for (int i = 0; i < size; ++i)
		combined[i] = x[i];

	for (int i = size; i < combined_size; ++i)
		combined[i] = y[i-size];

	funcReconstruct(combined, rhoSigma, combined_size, "rhoSigma", false);

	for (int i = 0; i < size; ++i)
		rho[i] = rhoSigma[i];

	for (int i = size; i < combined_size; ++i)
		sigma[i-size] = rhoSigma[i];


	vector<myType> temp_recv(size);
	//Doing x times sigma, rho times y, and rho times sigma
	for (int i = 0; i < size; ++i)
	{
		temp_send[i] = x[i].first + sigma[i];
		temp_send[i] = rho[i] + y[i].first;
		temp_send[i] = rho[i] + sigma[i];
	}


	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(temp_send), nextParty(partyNum), size);
	threads[1] = thread(receiveVector<myType>, ref(temp_recv), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		if (temp[i] == temp_recv[i])
		{
			//Do the abort thing	
		}
}


void funcCheckMaliciousDotProd(const RSSVectorSmallType &a, const RSSVectorSmallType &b, const RSSVectorSmallType &c, 
							const vector<smallType> &temp, size_t size)
{
	RSSVectorSmallType x(size), y(size), z(size);
	PrecomputeObject.getTriplets(x, y, z, size);

	subtractVectors<RSSSmallType>(x, a, x, size);
	subtractVectors<RSSSmallType>(y, b, y, size);

	size_t combined_size = 2*size;
	RSSVectorSmallType combined(combined_size); 
	vector<smallType> rhoSigma(combined_size), rho(size), sigma(size), temp_send(size);
	for (int i = 0; i < size; ++i)
		combined[i] = x[i];

	for (int i = size; i < combined_size; ++i)
		combined[i] = y[i-size];

	funcReconstruct(combined, rhoSigma, combined_size, "rhoSigma", false);

	for (int i = 0; i < size; ++i)
		rho[i] = rhoSigma[i];

	for (int i = size; i < combined_size; ++i)
		sigma[i-size] = rhoSigma[i];


	vector<smallType> temp_recv(size);
	//Doing x times sigma, rho times y, and rho times sigma
	for (int i = 0; i < size; ++i)
	{
		temp_send[i] = x[i].first + sigma[i];
		temp_send[i] = rho[i] + y[i].first;
		temp_send[i] = rho[i] + sigma[i];
	}


	thread *threads = new thread[2];
	threads[0] = thread(sendVector<smallType>, ref(temp_send), nextParty(partyNum), size);
	threads[1] = thread(receiveVector<smallType>, ref(temp_recv), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		if (temp[i] == temp_recv[i])
		{
			//Do the abort thing	
		}
}


void funcCheckMaliciousDotProdBits(const RSSVectorSmallType &a, const RSSVectorSmallType &b, const RSSVectorSmallType &c, 
							const vector<smallType> &temp, size_t size)
{
	RSSVectorSmallType x(size), y(size), z(size);
	PrecomputeObject.getTriplets(x, y, z, size);

	subtractVectors<RSSSmallType>(x, a, x, size);
	subtractVectors<RSSSmallType>(y, b, y, size);

	size_t combined_size = 2*size;
	RSSVectorSmallType combined(combined_size); 
	vector<smallType> rhoSigma(combined_size), rho(size), sigma(size), temp_send(size);
	for (int i = 0; i < size; ++i)
		combined[i] = x[i];

	for (int i = size; i < combined_size; ++i)
		combined[i] = y[i-size];

	funcReconstruct(combined, rhoSigma, combined_size, "rhoSigma", false);

	for (int i = 0; i < size; ++i)
		rho[i] = rhoSigma[i];

	for (int i = size; i < combined_size; ++i)
		sigma[i-size] = rhoSigma[i];


	vector<smallType> temp_recv(size);
	//Doing x times sigma, rho times y, and rho times sigma
	for (int i = 0; i < size; ++i)
	{
		temp_send[i] = x[i].first + sigma[i];
		temp_send[i] = rho[i] + y[i].first;
		temp_send[i] = rho[i] + sigma[i];
	}


	thread *threads = new thread[2];
	threads[0] = thread(sendVector<smallType>, ref(temp_send), nextParty(partyNum), size);
	threads[1] = thread(receiveVector<smallType>, ref(temp_recv), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		if (temp[i] == temp_recv[i])
		{
			//Do the abort thing	
		}
}



//Asymmetric protocol for semi-honest setting.
int funcReconstruct3out3(const vector<myType> &a, vector<myType> &b, size_t size, string str, bool print)
{
	int sent = 0;
	log_print("Reconst: myType, myType");
	assert(a.size() == size && "a.size mismatch for reconstruct function");

	vector<myType> temp_A(size,0), temp_B(size, 0);



	if (partyNum == PARTY_A or partyNum == PARTY_B)
		sent+=sendVector<myType>(a, PARTY_C, size);
	if (partyNum == PARTY_C)
	{
		receiveVector<myType>(temp_A, PARTY_A, size);
		receiveVector<myType>(temp_B, PARTY_B, size);
		receiveRoundfix(PARTY_C,1);

		addVectors<myType>(temp_A, a, temp_A, size);
		addVectors<myType>(temp_B, temp_A, b, size);
		sent+=sendVector<myType>(b, PARTY_A, size);
		sent+=sendVector<myType>(b, PARTY_B, size);
		sendRoundfix(PARTY_C,1);
	}

	if (partyNum == PARTY_A or partyNum == PARTY_B){
		receiveVector<myType>(b, PARTY_C, size);
	}

	if (print)
	{
		std::cout << str << ": \t\t";
		for (int i = 0; i < size; ++i)
			print_linear(b[i], "SIGNED");
		std::cout << std::endl;
	}


	if (SECURITY_TYPE.compare("Semi-honest") == 0)
	{}

	if (SECURITY_TYPE.compare("Malicious") == 0)
	{}
	return sent;
}

void funcMaxReLU(const vector<myType> &a, RSSVectorMyType &b, RSSVectorSmallType &prime, size_t B, size_t tempSize, size_t Dout, size_t truncation,
				bool maxpool , bool ReLU , bool BN, const RSSVectorMyType &bias, size_t poolSize, size_t stride_M, bool Is_CNN , 
				const RSSVectorMyType &gamma, const RSSVectorMyType &beta)
{
	
		size_t b_size = b.size()/B;
		size_t p_size = prime.size()/B;
		// if (SECURITY_TYPE.compare("Malicious") == 0)
		// 	B=2*B;//the resource usage of alpha
		
		
		size_t m = tempSize*Dout;
		size_t split_size = round((float)B/3);
		size_t ind_P[3] = {0,split_size,split_size*2};
		size_t length[3] = {split_size,split_size,B - split_size*2};
		std::vector<myType> share_a1(length[prevParty(partyNum)]*m,0),recv_a1(length[partyNum]*m,0);
		std::vector<myType> share_a2(length[nextParty(partyNum)]*m,0),recv_a2(length[partyNum]*m,0);
		std::vector<myType> share_b2(Dout,0),recv_b1(Dout,0);
		std::vector<myType> share_gamma(length[nextParty(partyNum)],0),share_beta(length[nextParty(partyNum)],0),
							recv_gamma(length[partyNum],0),recv_beta(length[partyNum],0);
		for(int i = 0; i < length[prevParty(partyNum)]*m; i++){
			share_a1[i]=a[i+ind_P[prevParty(partyNum)]*m];
		}
		for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
			share_a2[i]=a[i+ind_P[nextParty(partyNum)]*m];
		}
		for(int i = 0; i < Dout; i++){
			share_b2[i]=bias[i].first;
		}


		chip.ChipGenMask(share_a1,share_a1,length[prevParty(partyNum)]*m);
		chip.ChipGenMask(share_a2,share_a2,length[nextParty(partyNum)]*m);
		chip.ChipGenMask(share_b2,share_b2,Dout);
		

		std::vector<char> databuff[4];
		int sizebuff[4];

		sizebuff[0] = share_a1.size()*sizeof(myType);
		sizebuff[1] = share_a2.size()*sizeof(myType)+share_b2.size()*sizeof(myType);
		sizebuff[2] = recv_a1.size()*sizeof(myType)+recv_b1.size()*sizeof(myType);
		sizebuff[3] = recv_a2.size()*sizeof(myType);
		if(BN){
			for(int i = 0; i < length[nextParty(partyNum)]; i++){
				share_gamma[i]=gamma[i+ind_P[nextParty(partyNum)]].first;
				share_beta[i]=beta[i+ind_P[nextParty(partyNum)]].first;
			}
			chip.ChipGenMask(share_gamma,share_gamma,length[nextParty(partyNum)]);
			chip.ChipGenMask(share_beta,share_beta,length[nextParty(partyNum)]);
			sizebuff[1] += share_gamma.size()*sizeof(myType)+share_beta.size()*sizeof(myType);
			sizebuff[2] += recv_gamma.size()*sizeof(myType)+recv_beta.size()*sizeof(myType);
		}
		for(int i = 0;i<4;i++){
			databuff[i].resize(sizebuff[i]);

		}

		memcpy(&databuff[0][0],share_a1.data(),share_a1.size()*sizeof(myType));
		memcpy(&databuff[1][0],share_a2.data(),share_a2.size()*sizeof(myType));
		memcpy(&databuff[1][share_a2.size()*sizeof(myType)],share_b2.data(),share_b2.size()*sizeof(myType));
		if(BN){
			memcpy(&databuff[1][share_a2.size()*sizeof(myType)+share_b2.size()*sizeof(myType)],
			share_gamma.data(),share_gamma.size()*sizeof(myType));
			memcpy(&databuff[1][share_a2.size()*sizeof(myType)+share_b2.size()*sizeof(myType)+share_gamma.size()*sizeof(myType)],
			share_beta.data(),share_beta.size()*sizeof(myType));
		}
		thread *threads = new thread[4];
		threads[0] = thread(sendVector<char>, ref(databuff[0]), prevParty(partyNum), databuff[0].size());
		threads[1] = thread(sendVector<char>, ref(databuff[1]), nextParty(partyNum), databuff[1].size());
		threads[2] = thread(receiveVector<char>, ref(databuff[2]), prevParty(partyNum), databuff[2].size());
		threads[3] = thread(receiveVector<char>, ref(databuff[3]), nextParty(partyNum), databuff[3].size());
		
		for (int i = 0; i < 4; i++)
			threads[i].join();
		delete[] threads;
		// cout<<"endt"<<endl;
		memcpy(recv_a1.data(),&databuff[2][0],recv_a1.size()*sizeof(myType));
		memcpy(recv_b1.data(),&databuff[2][recv_a1.size()*sizeof(myType)],recv_b1.size()*sizeof(myType));
		memcpy(recv_a2.data(),&databuff[3][0],recv_a2.size()*sizeof(myType));
		if(BN){
			memcpy(recv_gamma.data(),&databuff[2][recv_a1.size()*sizeof(myType)+recv_b1.size()*sizeof(myType)],
			recv_gamma.size()*sizeof(myType));
			memcpy(recv_beta.data(),&databuff[2][recv_a1.size()*sizeof(myType)+recv_b1.size()*sizeof(myType)+
			recv_gamma.size()*sizeof(myType)],recv_beta.size()*sizeof(myType));
		}
		// cout<<"endcp"<<endl;
		int input_count = chip_package_len-chip_package_len%m;
		if (input_count == 0){
			input_count = chip_package_len;
		}
		
		RSSVectorMyType tempb(length[partyNum]*b_size);
		vector<myType> tempa(length[partyNum]*m),tempbias(Dout),tempgamma(length[partyNum]),tempbeta(length[partyNum]);
		RSSVectorSmallType tempp(length[partyNum]*p_size);
		
		for(int i = 0; i < length[partyNum]*m; i++){
			tempa[i]=a[i+ind_P[partyNum]*m]+recv_a1[i]+recv_a2[i];
		}
		for(int i = 0; i < Dout; i++){
			tempbias[i]=bias[i].first+bias[i].second+recv_b1[i];
		}
		if(BN){
			for(int i = 0; i < length[partyNum]; i++){
				tempgamma[i]=gamma[i+ind_P[partyNum]].first+gamma[i+ind_P[partyNum]].second+recv_gamma[i];
				tempbeta[i]=beta[i+ind_P[partyNum]].first+beta[i+ind_P[partyNum]].second+recv_beta[i];
			}
		}

	
		chip.ChipMaxReLU(tempa,tempbias, tempb,tempp,truncation,length[partyNum],tempSize, Dout, maxpool, ReLU, 
							BN, poolSize, stride_M, Is_CNN, tempgamma, tempbeta);

		std::vector<myType> share01(length[partyNum]*b_size,0),share02(length[partyNum]*b_size,0),
							recv01(length[prevParty(partyNum)]*b_size,0),recv02(length[nextParty(partyNum)]*b_size,0);
		std::vector<smallType> sharep1(length[partyNum]*p_size,0),sharep2(length[partyNum]*p_size,0),
							recvp1(length[prevParty(partyNum)]*p_size,0),recvp2(length[nextParty(partyNum)]*p_size,0);
		for(int i = 0; i < length[partyNum]*b_size; i++){
				share01[i]=tempb[i].first;
				share02[i]=tempb[i].second;
		}
		for(int i = 0; i < length[partyNum]*p_size; i++){
				sharep1[i]=tempp[i].first;
				sharep2[i]=tempp[i].second;
		}


		sizebuff[0] = share01.size()*sizeof(myType)+sharep1.size()*sizeof(smallType);
		sizebuff[1] = share02.size()*sizeof(myType)+sharep2.size()*sizeof(smallType);
		sizebuff[2] = recv01.size()*sizeof(myType)+recvp1.size()*sizeof(smallType);
		sizebuff[3] = recv02.size()*sizeof(myType)+recvp2.size()*sizeof(smallType);
		for(int i = 0;i<4;i++){
			databuff[i].resize(sizebuff[i]);
		}
		memcpy(&databuff[0][0],share01.data(),share01.size()*sizeof(myType));
		memcpy(&databuff[0][share01.size()*sizeof(myType)],sharep1.data(),sharep1.size()*sizeof(smallType));
		memcpy(&databuff[1][0],share02.data(),share02.size()*sizeof(myType));
		memcpy(&databuff[1][share02.size()*sizeof(myType)],sharep2.data(),sharep2.size()*sizeof(smallType));

		thread *threads2 = new thread[4];
		threads2[0] = thread(sendVector<char>, ref(databuff[0]), prevParty(partyNum), databuff[0].size());
		threads2[1] = thread(sendVector<char>, ref(databuff[1]), nextParty(partyNum), databuff[1].size());
		threads2[2] = thread(receiveVector<char>, ref(databuff[2]), prevParty(partyNum),databuff[2].size());
		threads2[3] = thread(receiveVector<char>, ref(databuff[3]), nextParty(partyNum),databuff[3].size());
		for (int i = 0; i < 4; i++)
			threads2[i].join();
		delete[] threads2;

		memcpy(recv01.data(),&((myType*)databuff[2].data())[0],recv01.size()*sizeof(myType));
		memcpy(recvp1.data(),&databuff[2][recv01.size()*sizeof(myType)],recvp1.size()*sizeof(smallType));
		memcpy(recv02.data(),&databuff[3][0],recv02.size()*sizeof(myType));
		memcpy(recvp2.data(),&databuff[3][recv02.size()*sizeof(myType)],recvp2 .size()*sizeof(smallType));
		
		// if (SECURITY_TYPE.compare("Malicious") == 0)
		// 	B=B/2;
		for(int i = 0; i < length[partyNum]*b_size; i++){

			b[i+ind_P[partyNum]*b_size].first=tempb[i].first;
			b[i+ind_P[partyNum]*b_size].second=tempb[i].second;
			assert(i+ind_P[partyNum]*b_size<b.size());
		}

		for(int i = 0; i < length[partyNum]*p_size; i++){

			prime[i+ind_P[partyNum]*p_size].first=tempp[i].first;
			prime[i+ind_P[partyNum]*p_size].second=tempp[i].second;
			// cout<<"i+ind_P[partyNum]*m<prime.size() "<<i+ind_P[partyNum]*m<<" "<<prime.size()<<endl;
			assert(i+ind_P[partyNum]*p_size<prime.size());
		}

		for(int i = 0; i < length[prevParty(partyNum)]*b_size; i++){

			b[i+ind_P[prevParty(partyNum)]*b_size].first=recv01[i];
			b[i+ind_P[prevParty(partyNum)]*b_size].second=chip.Chip_mask3[i%chip_package_len];
			// cout<<i<<" "<<ind_P[prevParty(partyNum)]*b_size<<" "<<b.size()<<" "<<B*b_size<<endl;

			assert(i+ind_P[prevParty(partyNum)]*b_size<b.size());
		}

		for(int i = 0; i < length[prevParty(partyNum)]*p_size; i++){

			prime[i+ind_P[prevParty(partyNum)]*p_size].first=recvp1[i];
			prime[i+ind_P[prevParty(partyNum)]*p_size].second=0;
			// cout<<"i+ind_P[prevParty(partyNum)]*m<prime.size() "<<i+ind_P[prevParty(partyNum)]*m<<" "<<prime.size()<<endl;
			
		}

		for(int i = 0; i < length[nextParty(partyNum)]*b_size; i++){

			b[i+ind_P[nextParty(partyNum)]*b_size].first=chip.Chip_mask3[i%chip_package_len];
			b[i+ind_P[nextParty(partyNum)]*b_size].second=recv02[i];
			
		}

		for(int i = 0; i < length[nextParty(partyNum)]*p_size; i++){
			
			prime[i+ind_P[nextParty(partyNum)]*p_size].first=0;
			prime[i+ind_P[nextParty(partyNum)]*p_size].second=recvp2[i];
			

		}
		chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
		

}




/******************************** Functionalities MPC ********************************/
// Matrix Multiplication of a*b = c with transpose flags for a,b.
// Output is a share between PARTY_A and PARTY_B.
// a^transpose_a is rows*common_dim and b^transpose_b is common_dim*columns
int MatMul_Com = 0;
int *pt_MatMul_Com = &MatMul_Com;
int MatMul_time = 0;
int *pt_MatMul_time = &MatMul_time; 
int MatMul_rounds = 0;
int *pt_MatMul_rounds = &MatMul_rounds; 
void funcMatMul(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &c, 
					size_t rows, size_t common_dim, size_t columns,
				 	size_t transpose_a, size_t transpose_b, size_t truncation)
{
	int clock_begin = clock();
	log_print("funcMatMul");
	assert(a.size() == rows*common_dim && "Matrix a incorrect for Mat-Mul");
	assert(b.size() == common_dim*columns && "Matrix b incorrect for Mat-Mul");
	assert(c.size() == rows*columns && "Matrix c incorrect for Mat-Mul");

#if (LOG_DEBUG)
	cout << "Rows, Common_dim, Columns: " << rows << "x" << common_dim << "x" << columns << endl;
#endif

	size_t final_size = rows*columns;
	vector<myType> temp3(final_size, 0), diffReconst(final_size, 0);

	// matrixMultRSS(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#if (USE_CUDA)
	// cout<<"11111111111"<<endl;
	matrixMultRSS_Cuda(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#else
	matrixMultRSS(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#endif
	RSSVectorMyType r(final_size), rPrime(final_size);
	PrecomputeObject.getDividedShares(r, rPrime, (1<<truncation), final_size);
	for (int i = 0; i < final_size; ++i)
		temp3[i] = temp3[i] - rPrime[i].first;
    cpu.CPU_Add(final_size);

	*pt_MatMul_Com+=funcReconstruct3out3(temp3, diffReconst, final_size, "Mat-Mul diff reconst", false);
	*pt_MatMul_rounds+=1;
	// =*pt_MatMul_Com+final_size*sizeof(myType);

	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousMatMul(a, b, c, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	dividePlain(diffReconst, (1 << truncation));

	// for (int i = 0; i < 128; ++i)
	// 	print_linear(diffReconst[i], "FLOAT");
	// cout << endl;

	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first + diffReconst[i];
			c[i].second = r[i].second;
		}
        cpu.CPU_Add(final_size);
	}

	if (partyNum == PARTY_B)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first;
			c[i].second = r[i].second;
		}
	}

	if (partyNum == PARTY_C)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first;
			c[i].second = r[i].second + diffReconst[i];
		}
        cpu.CPU_Add(final_size);
	}	
	*pt_MatMul_time+=clock()-clock_begin;
}
void funcMatMul_mixed(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &c, RSSVectorSmallType &prime,
					size_t rows, size_t common_dim, size_t columns,
				 	size_t transpose_a, size_t transpose_b, size_t truncation,
					size_t B, size_t Dout, size_t tempSize, 
					bool is_CNN, bool maxpool, bool ReLU, bool BN, 
					const RSSVectorMyType &bias, size_t poolSize, size_t stride_M,
					const RSSVectorMyType &gamma, const RSSVectorMyType &beta)
{
	int clock_begin = clock();
	log_print("funcMatMul");
	assert(a.size() == rows*common_dim && "Matrix a incorrect for Mat-Mul");
	assert(b.size() == common_dim*columns && "Matrix b incorrect for Mat-Mul");
	// assert(c.size() == rows*columns && "Matrix c incorrect for Mat-Mul");

#if (LOG_DEBUG)
	cout << "Rows, Common_dim, Columns: " << rows << "x" << common_dim << "x" << columns << endl;
#endif

	size_t final_size = rows*columns;
	vector<myType> temp3(final_size, 0);

	// matrixMultRSS(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#if (USE_CUDA)
	// cout<<"11111111111"<<endl;
	matrixMultRSS_Cuda(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#else
	matrixMultRSS(a, b, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	#endif
	RSSVectorMyType c_temp(final_size,make_pair(0,0));
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousMatMul(a, b, c_temp, temp3, rows, common_dim, columns, transpose_a, transpose_b);
	// RSSVectorMyType r(final_size), rPrime(final_size);
	// PrecomputeObject.getDividedShares(r, rPrime, (1<<truncation), final_size);
	// for (int i = 0; i < final_size; ++i)
	// 	temp3[i] = temp3[i] - rPrime[i].first;
    cpu.CPU_Add(final_size);
	vector<myType> temp4(final_size);
	// RSSVectorSmallType tempprime(final_size);
	if (is_CNN){
		for (size_t i = 0; i < B; ++i)
			for (size_t j = 0; j < Dout; ++j) 
				for (size_t k = 0; k < tempSize; ++k)
					{
						temp4[i*Dout*tempSize + j*tempSize + k] = temp3[j*B*tempSize + i*tempSize + k];
						// tempprime[i*Dout*tempSize + j*tempSize + k] = tempprime[j*B*tempSize + i*tempSize + k];
					}
		
		funcMaxReLU(temp4, c, prime, B, tempSize, Dout, truncation, maxpool, ReLU, BN, bias, poolSize, stride_M, true,gamma, beta);
		
	}
	else{
		funcMaxReLU(temp3, c, prime, rows, 1, columns, truncation, maxpool, ReLU, BN, bias, poolSize, stride_M, false,gamma, beta);

	}

	
	

	


	*pt_MatMul_time+=clock()-clock_begin;
}

void funcVecMul(const RSSVectorMyType &a, RSSVectorMyType &c, 
					size_t rows, size_t columns,size_t truncation)
{
	int clock_begin = clock();
	log_print("funcVecMul");
	size_t final_size = rows*columns*columns;
	vector<myType> temp3(final_size, 0), diffReconst(final_size, 0);

#if (LOG_DEBUG)
	cout << "Rows, Columns: " << rows << "x" << columns << endl;
#endif
	
	#if (USE_CUDA)
	// cout<<"11111111111"<<endl;
	vectorMultRSS_Cuda(a, temp3, rows, columns);
	#else // TODO: DEBUG i issue
	vectorMultRSS(a, temp3, rows, columns);
	#endif
	
	
	RSSVectorMyType r(final_size), rPrime(final_size);
	PrecomputeObject.getDividedShares(r, rPrime, (1<<truncation), final_size);
	for (int i = 0; i < final_size; ++i)
		temp3[i] = temp3[i] - rPrime[i].first;
    cpu.CPU_Add(final_size);
	*pt_MatMul_Com+=funcReconstruct3out3(temp3, diffReconst, final_size, "Mat-Mul diff reconst", false);
	*pt_MatMul_rounds+=1;
	// =*pt_MatMul_Com+final_size*sizeof(myType);
	
	dividePlain(diffReconst, (1 << truncation));


	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first + diffReconst[i];
			c[i].second = r[i].second;
		}
        cpu.CPU_Add(final_size);
	}

	if (partyNum == PARTY_B)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first;
			c[i].second = r[i].second;
		}
	}

	if (partyNum == PARTY_C)
	{
		for (int i = 0; i < final_size; ++i)
		{
			c[i].first = r[i].first;
			c[i].second = r[i].second + diffReconst[i];
		}
        cpu.CPU_Add(final_size);
	}	
	*pt_MatMul_time+=clock()-clock_begin;
}


// Term by term multiplication of 64-bit vectors overriding precision
int* funcDotProduct(const RSSVectorMyType &a, const RSSVectorMyType &b, 
						   RSSVectorMyType &c, size_t size, bool truncation, size_t precision) 
{
	static int sent[2]={0,0};
	log_print("funcDotProduct");
	assert(a.size() == size && "Matrix a incorrect for Mat-Mul");
	assert(b.size() == size && "Matrix b incorrect for Mat-Mul");
	assert(c.size() == size && "Matrix c incorrect for Mat-Mul");

	vector<myType> temp3(size, 0);

	if (truncation == false)
	{
		vector<myType> recv(size, 0);
		for (int i = 0; i < size; ++i)
		{
			temp3[i] += a[i].first * b[i].first +
					    a[i].first * b[i].second +
					    a[i].second * b[i].first;
		}
        cpu.CPU_Add(size*2);
        cpu.CPU_Mul(size*3);
		thread *threads = new thread[2];

		threads[0] = thread(sendVector<myType>, ref(temp3), prevParty(partyNum), size);
		sent[0]+=sizeof(myType)*size;
		sent[1]+=1;
		threads[1] = thread(receiveVector<myType>, ref(recv), nextParty(partyNum), size);
		
		for (int i = 0; i < 2; i++)
			threads[i].join();
		delete[] threads; 

		for (int i = 0; i < size; ++i)
		{
			c[i].first = temp3[i];
			c[i].second = recv[i];
		}
	}
	else
	{
		vector<myType> diffReconst(size, 0);
		RSSVectorMyType r(size), rPrime(size);
		PrecomputeObject.getDividedShares(r, rPrime, (1<<precision), size);

		for (int i = 0; i < size; ++i)
		{
			temp3[i] += a[i].first * b[i].first +
					    a[i].first * b[i].second +
					    a[i].second * b[i].first -
					    rPrime[i].first;
		}
        cpu.CPU_Add(size*3);
        cpu.CPU_Mul(size*3);

		sent[0]+=funcReconstruct3out3(temp3, diffReconst, size, "Dot-product diff reconst", false);
		sent[1]+=1;
		dividePlain(diffReconst, (1 << precision));
		if (partyNum == PARTY_A)
		{
			for (int i = 0; i < size; ++i)
			{
				c[i].first = r[i].first + diffReconst[i];
				c[i].second = r[i].second;
			}
            cpu.CPU_Add(size);
		}

		if (partyNum == PARTY_B)
		{
			for (int i = 0; i < size; ++i)
			{
				c[i].first = r[i].first;
				c[i].second = r[i].second;
			}
		}

		if (partyNum == PARTY_C)
		{
			for (int i = 0; i < size; ++i)
			{
				c[i].first = r[i].first;
				c[i].second = r[i].second + diffReconst[i];
			}
            cpu.CPU_Add(size);
		}
	}
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousDotProd(a, b, c, temp3, size);
	return sent;
}


// Term by term multiplication of mod 67 vectors 
int* funcDotProduct(const RSSVectorSmallType &a, const RSSVectorSmallType &b, 
							 RSSVectorSmallType &c, size_t size) 
{
	static int sent[2]={0,0};
	log_print("funcDotProduct");
	assert(a.size() == size && "Matrix a incorrect for Mat-Mul");
	assert(b.size() == size && "Matrix b incorrect for Mat-Mul");
	assert(c.size() == size && "Matrix c incorrect for Mat-Mul");


	vector<smallType> temp3(size, 0), recv(size, 0);
	for (int i = 0; i < size; ++i)
	{
		temp3[i] = multiplicationModPrime[a[i].first][b[i].first];
		temp3[i] = additionModPrime[temp3[i]][multiplicationModPrime[a[i].first][b[i].second]];
		temp3[i] = additionModPrime[temp3[i]][multiplicationModPrime[a[i].second][b[i].first]];
	}
    cpu.CPU_Add(size*2);
	//Add random shares of 0 locally
	thread *threads = new thread[2];

	threads[0] = thread(sendVector<smallType>, ref(temp3), prevParty(partyNum), size);
	sent[0] += sizeof(smallType)*size;
	sent[1]+= 1;
	threads[1] = thread(receiveVector<smallType>, ref(recv), nextParty(partyNum), size);

	for (int i = 0; i < 2; i++)
		threads[i].join();

	delete[] threads; 

	for (int i = 0; i < size; ++i)
	{
		c[i].first = temp3[i];
		c[i].second = recv[i];
	}

	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousDotProd(a, b, c, temp3, size);
	return sent;
}


// Term by term multiplication boolean shares
int funcDotProductBits(const RSSVectorSmallType &a, const RSSVectorSmallType &b, 
							 RSSVectorSmallType &c, size_t size) 
{
	int sent;
	log_print("funcDotProductBits");
	assert(a.size() == size && "Matrix a incorrect for Mat-Mul");
	assert(b.size() == size && "Matrix b incorrect for Mat-Mul");
	assert(c.size() == size && "Matrix c incorrect for Mat-Mul");

	vector<smallType> temp3(size, 0), recv(size, 0);
	for (int i = 0; i < size; ++i)
	{
		temp3[i] = (a[i].first and b[i].first) ^ 
				   (a[i].first and b[i].second) ^ 
				   (a[i].second and b[i].first);
	}
    cpu.CPU_Gate(size*5);

	//Add random shares of 0 locally
	thread *threads = new thread[2];
	threads[0] = thread(sendVector<smallType>, ref(temp3), prevParty(partyNum), size);
	sent+=sizeof(smallType)*size;
	threads[1] = thread(receiveVector<smallType>, ref(recv), nextParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads; 

	for (int i = 0; i < size; ++i)
	{
		c[i].first = temp3[i];
		c[i].second = recv[i];
	}

	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousDotProdBits(a, b, c, temp3, size);
	return sent;
}


//Multiply index 2i, 2i+1 of the first vector into the second one. The second vector is half the size.
int funcMultiplyNeighbours(const RSSVectorSmallType &c_1, RSSVectorSmallType &c_2, size_t size)
{
	int sent;
	assert (size % 2 == 0 && "Size should be 'half'able");
	vector<smallType> temp3(size/2, 0), recv(size/2, 0);
	for (int i = 0; i < size/2; ++i)
	{
		temp3[i] = additionModPrime[temp3[i]][multiplicationModPrime[c_1[2*i].first][c_1[2*i+1].first]];
		temp3[i] = additionModPrime[temp3[i]][multiplicationModPrime[c_1[2*i].first][c_1[2*i+1].second]];
		temp3[i] = additionModPrime[temp3[i]][multiplicationModPrime[c_1[2*i].second][c_1[2*i+1].first]];
	}
    cpu.CPU_Add(size*3);

	//Add random shares of 0 locally
	thread *threads = new thread[2];

	threads[0] = thread(sendVector<smallType>, ref(temp3), nextParty(partyNum), size/2);
	sent += sizeof(smallType)*size/2;
	threads[1] = thread(receiveVector<smallType>, ref(recv), prevParty(partyNum), size/2);

	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size/2; ++i)
	{
		c_2[i].first = temp3[i];
		c_2[i].second = recv[i];
	}


	RSSVectorSmallType temp_a(size/2), temp_b(size/2), temp_c(size/2);
	if (SECURITY_TYPE.compare("Malicious") == 0)
		funcCheckMaliciousDotProd(temp_a, temp_b, temp_c, temp3, size/2);
	return sent;
}

//Multiply each group of 64 with a random number in Z_p* and reconstruct output in betaPrime.
int* funcCrunchMultiply(const RSSVectorSmallType &c, vector<smallType> &betaPrime, size_t size)
{
	static int sent[2];
	size_t sizeLong = size*BIT_SIZE;
	RSSVectorSmallType c_0(sizeLong/2, make_pair(0,0)), c_1(sizeLong/4, make_pair(0,0)), 
					   c_2(sizeLong/8, make_pair(0,0)), c_3(sizeLong/16, make_pair(0,0)), 
					   c_4(sizeLong/32, make_pair(0,0)); 
	RSSVectorSmallType c_5(sizeLong/64, make_pair(0,0));

	vector<smallType> reconst(size, 0);

	sent[0] += funcMultiplyNeighbours(c, c_0, sizeLong);
	sent[0]+=funcMultiplyNeighbours(c_0, c_1, sizeLong/2);
	sent[0] +=funcMultiplyNeighbours(c_1, c_2, sizeLong/4);
	sent[0]+=funcMultiplyNeighbours(c_2, c_3, sizeLong/8);
	sent[0] +=funcMultiplyNeighbours(c_3, c_4, sizeLong/16);
	sent[1]+=5;
	if (BIT_SIZE == 64){
		sent[0] +=funcMultiplyNeighbours(c_4, c_5, sizeLong/32);
		sent[1]+=1;
	}

	vector<smallType> a_next(size), a_prev(size);
	if (BIT_SIZE == 64){
		for (int i = 0; i < size; ++i)
		{
			a_prev[i] = 0;
			a_next[i] = c_5[i].first;
			reconst[i] = c_5[i].first;
			reconst[i] = additionModPrime[reconst[i]][c_5[i].second];
		}
        cpu.CPU_Add(size);
    }
	else if (BIT_SIZE == 32){
		for (int i = 0; i < size; ++i)
		{
			a_prev[i] = 0;
			a_next[i] = c_4[i].first;
			reconst[i] = c_4[i].first;
			reconst[i] = additionModPrime[reconst[i]][c_4[i].second];
		}
        cpu.CPU_Add(size);
    }

	thread *threads = new thread[2];

	threads[0] = thread(sendVector<smallType>, ref(a_next), nextParty(partyNum), size);
	sent[0] += sizeof(smallType)*size;
	sent[1]+=1;
	threads[1] = thread(receiveVector<smallType>, ref(a_prev), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		reconst[i] = additionModPrime[reconst[i]][a_prev[i]];
    cpu.CPU_Add(size);
	for (int i = 0; i < size; ++i)
	{
		if (reconst[i] == 0)
			betaPrime[i] = 1;
	}
	return sent;
}

//Thread function for parallel private compare
void parallelFirst(smallType* temp3, const RSSSmallType* beta, const myType* r, 
					const RSSSmallType* share_m, size_t start, size_t end, int t)
{
	size_t index3, index2;
	smallType bit_r;
	RSSSmallType twoBetaMinusOne, diff;


	for (int index2 = start; index2 < end; ++index2)
	{
		//Computing 2Beta-1
		twoBetaMinusOne = subConstModPrime(beta[index2], 1);
		twoBetaMinusOne = addModPrime(twoBetaMinusOne, beta[index2]);

		for (size_t k = 0; k < BIT_SIZE; ++k)
		{
			index3 = index2*BIT_SIZE + k;
			bit_r = (smallType)((r[index2] >> (BIT_SIZE-1-k)) & 1);
			diff = share_m[index3];
					
			if (bit_r == 1)
				diff = subConstModPrime(diff, 1);

			//Dot Product
			temp3[index3] = multiplicationModPrime[diff.first][twoBetaMinusOne.first];
			temp3[index3] = additionModPrime[temp3[index3]][multiplicationModPrime[diff.first][twoBetaMinusOne.second]];
			temp3[index3] = additionModPrime[temp3[index3]][multiplicationModPrime[diff.second][twoBetaMinusOne.first]];
		}

	}
    cpu.CPU_Add((end-start)*BIT_SIZE*2);
    cpu.CPU_Mul((end-start)*BIT_SIZE);
}

void parallelSecond(RSSSmallType* c, const smallType* temp3, const smallType* recv, const myType* r, 
					const RSSSmallType* share_m, size_t start, size_t end, int t)
{
	size_t index3, index2;
	smallType bit_r;
	RSSSmallType a, tempM, tempN, xMinusR;

	if (partyNum == PARTY_A)
	{
		for (int index2 = start; index2 < end; ++index2)
		{
			a = make_pair(0, 0);
			for (size_t k = 0; k < BIT_SIZE; ++k)
			{
				index3 = index2*BIT_SIZE + k;
				//Complete Dot Product
				xMinusR.first = temp3[index3];
				xMinusR.second = recv[index3];

				//Resume rest of the loop
				c[index3] = a;	
				tempM = share_m[index3];
				bit_r = (smallType)((r[index2] >> (BIT_SIZE-1-k)) & 1);

				tempN = XORPublicModPrime(tempM, bit_r);
				a = addModPrime(a, tempN);

				c[index3].first = additionModPrime[c[index3].first][xMinusR.first];
				c[index3].first = additionModPrime[c[index3].first][1];
				c[index3].second = additionModPrime[c[index3].second][xMinusR.second];
			}
		}
        cpu.CPU_Add((end-start)*BIT_SIZE*2);
	}


	if (partyNum == PARTY_B)
	{
		for (int index2 = start; index2 < end; ++index2)
		{
			a = make_pair(0, 0);
			for (size_t k = 0; k < BIT_SIZE; ++k)
			{
				index3 = index2*BIT_SIZE + k;
				//Complete Dot Product
				xMinusR.first = temp3[index3];
				xMinusR.second = recv[index3];

				//Resume rest of the loop
				c[index3] = a;	
				tempM = share_m[index3];
				bit_r = (smallType)((r[index2] >> (BIT_SIZE-1-k)) & 1);

				tempN = XORPublicModPrime(tempM, bit_r);
				a = addModPrime(a, tempN);

				c[index3].first = additionModPrime[c[index3].first][xMinusR.first];
				c[index3].second = additionModPrime[c[index3].second][xMinusR.second];
			}
		}
        cpu.CPU_Add((end-start)*BIT_SIZE*2);
	}


	if (partyNum == PARTY_C)
	{
		for (int index2 = start; index2 < end; ++index2)
		{
			a = make_pair(0, 0);
			for (size_t k = 0; k < BIT_SIZE; ++k)
			{
				index3 = index2*BIT_SIZE + k;
				//Complete Dot Product
				xMinusR.first = temp3[index3];
				xMinusR.second = recv[index3];

				//Resume rest of the loop
				c[index3] = a;	
				tempM = share_m[index3];
				bit_r = (smallType)((r[index2] >> (BIT_SIZE-1-k)) & 1);

				tempN = XORPublicModPrime(tempM, bit_r);
				a = addModPrime(a, tempN);

				c[index3].first = additionModPrime[c[index3].first][xMinusR.first];
				c[index3].second = additionModPrime[c[index3].second][xMinusR.second];
				c[index3].second = additionModPrime[c[index3].second][1];
			}
		}
        cpu.CPU_Add((end-start)*BIT_SIZE*2);
	}	
}


// Private Compare functionality
int PP_Com = 0;
int *pt_PP_Com = &PP_Com;
int PP_rounds = 0;
int *pt_PP_rounds = &PP_Com;
int* funcPrivateCompare(const RSSVectorSmallType &share_m, const vector<myType> &r, 
							const RSSVectorSmallType &beta, vector<smallType> &betaPrime, 
							size_t size)
{
	error("No use protocol");
}



//Wrap functionality.
int* funcWrap(const RSSVectorMyType &a, RSSVectorSmallType &theta, size_t size)
{
	static int sent[2] ={0,0};
	log_print("funcWrap");
	
	size_t sizeLong = size*BIT_SIZE;
	RSSVectorMyType x(size), r(size); 
	RSSVectorSmallType shares_r(sizeLong), alpha(size), beta(size), eta(size); 
	vector<smallType> delta(size), etaPrime(size); 
	vector<myType> reconst_x(size);

	PrecomputeObject.getShareConvertObjects(r, shares_r, alpha, size);
	addVectors<RSSMyType>(a, r, x, size);
	for (int i = 0; i < size; ++i)
	{
		beta[i].first = wrapAround(a[i].first, r[i].first);
		x[i].first = a[i].first + r[i].first;
		beta[i].second = wrapAround(a[i].second, r[i].second);
		x[i].second = a[i].second + r[i].second;
	}
	cpu.CPU_Add(size*2);
	cpu.CPU_Comp(size*2);

	vector<myType> x_next(size), x_prev(size);
	for (int i = 0; i < size; ++i)
	{
		x_prev[i] = 0;
		x_next[i] = x[i].first;
		reconst_x[i] = x[i].first;
		reconst_x[i] = reconst_x[i] + x[i].second;
	}
    cpu.CPU_Add(size);

	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(x_next), nextParty(partyNum), size);
	sent[0] += sizeof(myType)*size;
	sent[1] += 1;
	threads[1] = thread(receiveVector<myType>, ref(x_prev), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	for (int i = 0; i < size; ++i)
		reconst_x[i] = reconst_x[i] + x_prev[i];

	wrap3(x, x_prev, delta, size); // All parties have delta
	PrecomputeObject.getRandomBitShares(eta, size);

	// cout << "PC: \t\t" << funcTime(funcPrivateCompare, shares_r, reconst_x, eta, etaPrime, size, BIT_SIZE) << endl;
	int* tempsent2;
	tempsent2 = funcPrivateCompare(shares_r, reconst_x, eta, etaPrime, size);
	sent[0]+=tempsent2[0];
	sent[1]+=tempsent2[1];

	if (partyNum == PARTY_A)
	{
		for (int i = 0; i < size; ++i)
		{
			theta[i].first = beta[i].first ^ delta[i] ^ alpha[i].first ^ eta[i].first ^ etaPrime[i];
			theta[i].second = beta[i].second ^ alpha[i].second ^ eta[i].second;
		}
        cpu.CPU_Gate(size*6);
	}
	else if (partyNum == PARTY_B)
	{
		for (int i = 0; i < size; ++i)
		{
			theta[i].first = beta[i].first ^ delta[i] ^ alpha[i].first ^ eta[i].first;
			theta[i].second = beta[i].second ^ alpha[i].second ^ eta[i].second;
		}
        cpu.CPU_Gate(size*5);
	}
	else if (partyNum == PARTY_C)
	{
		for (int i = 0; i < size; ++i)
		{
			theta[i].first = beta[i].first ^ alpha[i].first ^ eta[i].first;
			theta[i].second = beta[i].second ^ delta[i] ^ alpha[i].second ^ eta[i].second ^ etaPrime[i];
		}	
        cpu.CPU_Gate(size*6);
	}
	return sent;
}


// Set c[i] = a[i] if b[i] = 0
// Set c[i] = 0    if b[i] = 1
int SS_Com = 0;
int *pt_SS_Com = &SS_Com;
int SS_rounds = 0;
int *pt_SS_rounds = &SS_rounds;
void funcSelectShares(const RSSVectorMyType &a, const RSSVectorSmallType &b, 
								RSSVectorMyType &selected, size_t size)
{
	int* tempsent2;
	log_print("funcSelectShares");

	vector<smallType> recvb(size),mask_sent_b(size),receiv_b(size),sent_b(size),mask_recv_b(size);
	vector<myType> shareprev(size),sharenext(size);
	// share_Gen(shareprev,size);
	// share_Gen(sharenext,size);
	// mask_Gen(mask_sent_b,size);
	for(int i=0;i<size;i++)
		sent_b[i]=b[i].first^mask_sent_b[i];
    cpu.CPU_Gate(size);
	thread* threads = new thread[2];
	threads[0] = thread(sendVector<smallType>, ref(sent_b), nextParty(partyNum), size);
	*pt_SS_Com+=sizeof(smallType)*size;
	*pt_SS_rounds+=1;
	threads[1] = thread(receiveVector<smallType>, ref(receiv_b), prevParty(partyNum), size);
	for (int i = 0; i < 2; i++)
			threads[i].join();
	delete[] threads;
	for(int i=0;i<size;i++){
		recvb[i]=b[i].first^b[i].second^mask_recv_b[i]^receiv_b[i];
		selected[i].first=recvb[i]?a[i].first:0+shareprev[i];
		selected[i].second=recvb[i]?a[i].second:0+sharenext[i];
	}
    cpu.CPU_Gate(size*5);
	
	

}

//Within each group of columns, select a0 or a1 depending on value of bit b into answer.
//loopCounter is used to offset a1 by loopCounter*rows*columns
//answer = ((a0 \oplus a1) b ) \oplus a0
int funcSelectBitShares(const RSSVectorSmallType &a0, const RSSVectorSmallType &a1, 
						 const RSSVectorSmallType &b, RSSVectorSmallType &answer, 
						 size_t rows, size_t columns, size_t loopCounter)
{
	int sent=0;
	int temp=0;
	log_print("funcSelectBitShares");
	size_t size = rows*columns;
	assert(a0.size() == rows*columns && "a0 size incorrect");
	assert(a1.size() == (columns)*rows*columns && "a1 size incorrect");
	assert(b.size() == rows && "b size incorrect");
	assert(answer.size() == rows*columns && "answers size incorrect");
	
	RSSVectorSmallType bRepeated(size), tempXOR(size);
	for (int i = 0; i < rows; ++i)
		for (size_t j = 0; j < columns; ++j)
			bRepeated[i*columns + j] = b[i];

	for (size_t i = 0; i < rows; ++i)
		for (size_t j = 0; j < columns; ++j)
			tempXOR[i*columns+j] = a0[i*columns+j] ^
								   a1[loopCounter*rows*columns+i*columns+j];
    cpu.CPU_Gate(rows*columns);
	temp=funcDotProductBits(tempXOR, bRepeated, answer, size);
	sent+=temp;
	*pt_SS_Com+=temp;

	for (int i = 0; i < size; ++i)
		answer[i] = answer[i] ^ a0[i];
    cpu.CPU_Gate(size);
	return sent;
}


// b holds bits of ReLU' of a TODO
int* funcRELUPrime(const RSSVectorMyType &a, RSSVectorSmallType &b, size_t size)
{
	// cout<<"startRELUP"<<endl;
    int clock_begin=clock();
	static int sent[2] = {0,0};
	int tempsent = 0;
	int* tempsent2;
	log_print("funcRELUP");

	
	#if SPLIT3
	size_t split_size = round((float)size/3);
	std::vector<myType> d,share1,recv;
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,size - split_size*2};
	share1.resize(length[nextParty(partyNum)]);
	recv.resize(length[partyNum]);
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		share1[i]=a[i+ind_P[nextParty(partyNum)]].first;
	}
	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(share1), nextParty(partyNum), share1.size());
	sent[0]+=sizeof(myType)*share1.size();
	threads[1] = thread(receiveVector<myType>, ref(recv), prevParty(partyNum), share1.size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;
	RSSVectorSmallType tempb(length[partyNum]);
	std::vector<smallType> shareb_next(length[partyNum]),shareb_prev(length[partyNum]);
	std::vector<smallType> recvb_prev(length[prevParty(partyNum)],0),recvb_next(length[nextParty(partyNum)],0);
	RSSVectorMyType tempa(length[partyNum]);
	for(int i = 0; i < length[partyNum]; i++){
		tempa[i].first=a[i+ind_P[partyNum]].first;
		tempa[i].second=a[i+ind_P[partyNum]].second;
	}
	chip.ChipReLUP(recv,tempa,tempb,recv.size());
	for(int i = 0; i < length[partyNum]; i++){
		shareb_prev[i]=tempb[i].first;
		shareb_next[i]=tempb[i].second;
	}
	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<smallType>, ref(shareb_prev), prevParty(partyNum), shareb_prev.size());
	threads2[1] = thread(sendVector<smallType>, ref(shareb_next), nextParty(partyNum), shareb_next.size());
	threads2[2] = thread(receiveVector<smallType>, ref(recvb_prev), prevParty(partyNum), recvb_prev.size());
	threads2[3] = thread(receiveVector<smallType>, ref(recvb_next), nextParty(partyNum), recvb_next.size());
	sent[0]+=sizeof(myType)*tempb.size()*2;
	for (int i = 0; i < 4; i++)
		threads[i].join();
	delete[] threads2;
	for(int i = 0; i < length[partyNum]; i++){
		b[i+ind_P[partyNum]].first=tempb[i].first;
		b[i+ind_P[partyNum]].second=tempb[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]; i++){
		b[i+ind_P[prevParty(partyNum)]].first=recvb_prev[i];
		b[i+ind_P[prevParty(partyNum)]].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		b[i+ind_P[nextParty(partyNum)]].first=chip.Chip_mask3[i%chip_package_len];
		b[i+ind_P[nextParty(partyNum)]].second=recvb_next[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> d(size,0),share1(size,0),recv(size,0);
	for(int i = 0; i < size; i++){
		share1[i]=a[i].first;
	}
	if (partyNum == PARTY_C){
		chip.ChipGenMask(share1,d,size);
		sent[0]+=sendVector<myType>(d, PARTY_A, size);
		sent[1]+=1;
	}
		

	if (partyNum == PARTY_A){
		receiveVector<myType>(recv, PARTY_C, size);
		chip.ChipReLUP(recv,a,b,size);
		std::vector<myType> tob(size,0),toc(size,0);
		for(int i = 0; i < size; i++){
			tob[i]=b[i].first;
			toc[i]=b[i].second;
		}
		sent[0]+=sendVector<myType>(tob, PARTY_B, size);
		sent[1]+=1;
		sent[0]+=sendVector<myType>(toc, PARTY_C, size);
		sent[1]+=1;
		sendRoundfix(PARTY_A,1);
	}
	
	if (partyNum == PARTY_B){
		
		receiveVector<myType>(recv, PARTY_A, size);
		for(int i = 0; i < size; i++){
			b[i].first=recv[i];
			b[i].second=Mask_BC;
		}
	}
	if (partyNum == PARTY_C){

		receiveVector<myType>(recv, PARTY_A, size);
		for(int i = 0; i < size; i++){
			b[i].first=Mask_BC;
			b[i].second=recv[i];
		}
	}
	#endif

	return sent;
}


int ReLU_Com = 0;
int *pt_ReLU_Com = &ReLU_Com;
int ReLU_time = 0;
int *pt_ReLU_time = &ReLU_time;
int ReLU_rounds = 0;
int *pt_ReLU_rounds = &ReLU_rounds;


//Input is a, outputs are temp = ReLU'(a) and b = RELU(a).
int* funcRELU(const RSSVectorMyType &a, RSSVectorSmallType &temp, RSSVectorMyType &b, size_t size)
{
	// cout<<a.size()<<" "<<temp.size()<<" "<<b.size()<<" "<<endl;
	int clock_begin=clock();
	static int sent[2] = {0,0};
	int tempsent = 0;
	int* tempsent2;
	log_print("funcRELU");

	#if SPLIT3
	size_t split_size = round((float)size/3);
	std::vector<myType> d,share1,recv;
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,size - split_size*2};
	share1.resize(length[nextParty(partyNum)]);
	recv.resize(length[partyNum]);
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		share1[i]=a[i+ind_P[nextParty(partyNum)]].first;
	}
	chip.ChipGenMask(share1,share1,length[nextParty(partyNum)]);
	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(share1), nextParty(partyNum), share1.size());
	sent[0]+=sizeof(myType)*share1.size();
	threads[1] = thread(receiveVector<myType>, ref(recv), prevParty(partyNum), share1.size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;
	RSSVectorMyType tempb(length[partyNum]);
	std::vector<myType> shareb_next(length[partyNum]),shareb_prev(length[partyNum]);
	RSSVectorSmallType temptemp(length[partyNum]);
	std::vector<smallType> sharetemp_next(length[partyNum]),sharetemp_prev(length[partyNum]);
	std::vector<smallType> recvtemp_prev(length[prevParty(partyNum)],0),recvtemp_next(length[nextParty(partyNum)],0);
	std::vector<myType> recvb_prev(length[prevParty(partyNum)],0),recvb_next(length[nextParty(partyNum)],0);
	RSSVectorMyType tempa(length[partyNum]);
	for(int i = 0; i < length[partyNum]; i++){
		tempa[i].first=a[i+ind_P[partyNum]].first;
		tempa[i].second=a[i+ind_P[partyNum]].second;
	}

	chip.ChipReLU(recv,tempa,tempb,temptemp,length[partyNum]);
	for(int i = 0; i < length[partyNum]; i++){
		shareb_prev[i]=tempb[i].first;
		shareb_next[i]=tempb[i].second;
		sharetemp_prev[i]=temptemp[i].first;
		sharetemp_next[i]=temptemp[i].second;
	}
	
	std::vector<char> databuff[4];
	int sizebuff[4];
	sizebuff[0] = shareb_prev.size()*sizeof(myType)+sharetemp_prev.size()*sizeof(smallType);
	sizebuff[1] = shareb_next.size()*sizeof(myType)+sharetemp_next.size()*sizeof(smallType);
	sizebuff[2] = recvb_prev.size()*sizeof(myType)+recvtemp_prev.size()*sizeof(smallType);
	sizebuff[3] = recvb_next.size()*sizeof(myType)+recvtemp_next.size()*sizeof(smallType);
	for(int i = 0;i<4;i++){
		databuff[i].resize(sizebuff[i]);
	}
	memcpy(&databuff[0][0],shareb_prev.data(),shareb_prev.size()*sizeof(myType));
	memcpy(&databuff[0][shareb_prev.size()*sizeof(myType)],sharetemp_prev.data(),sharetemp_prev.size()*sizeof(smallType));
	memcpy(&databuff[1][0],shareb_next.data(),shareb_next.size()*sizeof(myType));
	memcpy(&databuff[1][shareb_next.size()*sizeof(myType)],sharetemp_next.data(),sharetemp_next.size()*sizeof(smallType));
	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<char>, ref(databuff[0]), prevParty(partyNum), databuff[0].size());
	threads2[1] = thread(sendVector<char>, ref(databuff[1]), nextParty(partyNum), databuff[1].size());
	threads2[2] = thread(receiveVector<char>, ref(databuff[2]), prevParty(partyNum),databuff[2].size());
	threads2[3] = thread(receiveVector<char>, ref(databuff[3]), nextParty(partyNum),databuff[3].size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;
	memcpy(recvb_prev.data(),&((myType*)databuff[2].data())[0],recvb_prev.size()*sizeof(myType));
	memcpy(recvtemp_prev.data(),&databuff[2][recvb_prev.size()*sizeof(myType)],recvtemp_prev.size()*sizeof(smallType));
	memcpy(recvb_next.data(),&databuff[3][0],recvb_next.size()*sizeof(myType));
	memcpy(recvtemp_next.data(),&databuff[3][recvb_next.size()*sizeof(myType)],recvtemp_next .size()*sizeof(smallType));
	sent[0]+=sizeof(myType)*tempb.size()*2;


	for(int i = 0; i < length[partyNum]; i++){
		b[i+ind_P[partyNum]].first=tempb[i].first;
		b[i+ind_P[partyNum]].second=tempb[i].second;
		temp[i+ind_P[partyNum]].first=temptemp[i].first;
		temp[i+ind_P[partyNum]].second=temptemp[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]; i++){
		b[i+ind_P[prevParty(partyNum)]].first=recvb_prev[i];
		b[i+ind_P[prevParty(partyNum)]].second=chip.Chip_mask3[i%chip_package_len];
		temp[i+ind_P[prevParty(partyNum)]].first=recvtemp_prev[i];
		temp[i+ind_P[prevParty(partyNum)]].second=chip.Chip_mask3[i%chip_package_len]%2;
	}
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		b[i+ind_P[nextParty(partyNum)]].first=chip.Chip_mask3[i%chip_package_len];
		b[i+ind_P[nextParty(partyNum)]].second=recvb_next[i];
		temp[i+ind_P[nextParty(partyNum)]].first=chip.Chip_mask3[i%chip_package_len]%2;
		temp[i+ind_P[nextParty(partyNum)]].second=recvtemp_next[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> d(size,0),share1(size,0),recv(size,0);
	for(int i = 0; i < size; i++){
		share1[i]=a[i].first;
	}
	if (partyNum == PARTY_C){
		chip.ChipGenMask(share1,d,size);
		sent[0]+=sendVector<myType>(d, PARTY_A, size);
		sent[1]+=1;
	}
	
	std::vector<myType> share01(size,0),share02(size,0),recv01(size,0);
	std::vector<smallType> sharep01(size,0),sharep02(size,0),recv02(size,0);

	// if(ARDUINO){
		if(partyNum==PARTY_A){
			receiveVector<myType>(recv, PARTY_C, size);
			chip.ChipReLU(recv,a,b,temp,size);
			
			for(int i = 0; i < size; i++){
				share01[i]=b[i].first;
				share02[i]=b[i].second;
				sharep01[i]=temp[i].first;
				sharep02[i]=temp[i].second;
			}
		}

		if(partyNum==PARTY_A){

			sent[0]+=sendVector<myType>(share01, PARTY_C, size);
			sent[0]+=sendVector<myType>(share02, PARTY_B, size);
			sent[0]+=sendVector<smallType>(sharep01, PARTY_C, size);
			sent[0]+=sendVector<smallType>(sharep02, PARTY_B, size);
			sendRoundfix(PARTY_A,3);
			// 1 use thread?
			// 2 use a buffer, use memcpy() to convert data to a buffer.
			// 3 Zeromp 
			sent[1]+=2;
		}
		if(partyNum==PARTY_B or partyNum==PARTY_C){

			receiveVector<myType>(recv01, PARTY_A, size);
			receiveVector<smallType>(recv02, PARTY_A, size);
			receiveRoundfix(partyNum,1);
			if(partyNum==PARTY_B){
			for(int i = 0; i < size; i++){
				b[i].first=recv01[i];
				b[i].second=chip.Chip_mask3[i%chip_package_len];
				temp[i].first=recv02[i];
				temp[i].second = chip.Chip_mask3[i%chip_package_len]%2;
			}
			}
			else{
				for(int i = 0; i < size; i++){
				b[i].first=chip.Chip_mask3[i%chip_package_len];
				b[i].second=recv01[i];
				temp[i].first=chip.Chip_mask3[i%chip_package_len]%2;
				temp[i].second=recv02[i];
			}
			}

		}
	// }
	// else{
	// 	chip.ChipReLU(recv,a,b,temp,size);
	// }
	// if(partyNum==PARTY_C){
	// 	cout<<"party 2"<<endl;
	// 	receiveVector<myType>(recv02, PARTY_A, size);
	// 	for(int i = 0; i < size; i++){
	// 		b[i].first=chip.Chip_mask3[i];
	// 		b[i].second=recv02[i];
	// 	}
	// }
	#endif

	*pt_ReLU_Com+=sent[0];
	*pt_ReLU_rounds+=sent[1];
	*pt_ReLU_time+=clock()-clock_begin;

	return sent;
}


int* funcPow(const RSSVectorMyType &b, vector<smallType> &alpha, size_t size)
{
	static int sent[2]={0,0};
	
	int *tempsent2;
	size_t ell = 5;
	if (BIT_SIZE == 64)
		ell = 6;

	RSSVectorMyType x(size), d(size), temp(size);
	copyVectors<RSSMyType>(b, x, size);

	RSSVectorSmallType c(size);
	for (int i = 0; i < size; ++i)
		alpha[i] = 0;

	vector<smallType> r_c(size);
	//TODO vecrorize this, right now only accepts the first argument
	for (int j = ell-1; j > -1; --j)
	{
		vector<myType> temp_1(size, (1 << ((1 << j) + (int)alpha[0])));
		funcGetShares(temp, temp_1);
		subtractVectors<RSSMyType>(x, temp, d, size);
		tempsent2=funcRELUPrime(d, c, size);

		sent[0]+=tempsent2[0];
		sent[1]+=tempsent2[1];

		sent[0]+=funcReconstructBit(c, r_c, size, "null", false);
		sent[1]+=1;

		if (r_c[0] == 0)
		{
			for (int i = 0; i < size; ++i)
				alpha[i] += (1 << j);
		}
	}
	
	return sent;
}


//All parties start with shares of a number in a and b and the quotient is in quotient.
//alpha is the order of divisiors, 2^alpha =< b < 2^{alpha+1}.
int Div_Com = 0;
int *pt_Div_Com = &Div_Com;
int Div_time = 0;
int *pt_Div_time = &Div_time;
int Div_rounds = 0;
int *pt_Div_rounds = &Div_rounds;
void funcDivision(const RSSVectorMyType &a, const RSSVectorMyType &b, RSSVectorMyType &quotient, 
							size_t size)
{

	int *tempsent2;
	int clock_begin=clock();
	log_print("funcDivision");

	//TODO incorporate funcPow
	//TODO Scale up and complete this computation with fixed-point precision
	vector<smallType> alpha_temp(size);
	tempsent2=funcPow(b, alpha_temp, size);

	*pt_Div_Com+=tempsent2[0];

	*pt_Div_rounds+=tempsent2[1];
	size_t alpha = alpha_temp[0];
	size_t precision = alpha + 1;
	const myType constTwoPointNine = ((myType)(2.9142 * (1 << precision)));
	const myType constOne = ((myType)(1 * (1 << precision)));

	vector<myType> data_twoPointNine(size, constTwoPointNine), data_one(size, constOne), reconst(size);
	RSSVectorMyType ones(size), twoPointNine(size), twoX(size), w0(size), xw0(size), 
					epsilon0(size), epsilon1(size), termOne(size), termTwo(size), answer(size);

	funcGetShares(twoPointNine, data_twoPointNine);

	funcGetShares(ones, data_one);


	multiplyByScalar(b, 2, twoX);
	subtractVectors<RSSMyType>(twoPointNine, twoX, w0, size);
	funcDotProduct(b, w0, xw0, size, true, precision); 
	subtractVectors<RSSMyType>(ones, xw0, epsilon0, size);
	if (PRECISE_DIVISION){
		tempsent2=funcDotProduct(epsilon0, epsilon0, epsilon1, size, true, precision);
		*pt_Div_Com+=tempsent2[0];
		*pt_Div_rounds+=tempsent2[1];
	}
	addVectors(ones, epsilon0, termOne, size);
	if (PRECISE_DIVISION)
		addVectors(ones, epsilon1, termTwo, size);
	tempsent2=funcDotProduct(w0, termOne, answer, size, true, precision);
	*pt_Div_Com+=tempsent2[0];
	*pt_Div_rounds+=tempsent2[1];
	if (PRECISE_DIVISION){
		tempsent2=funcDotProduct(answer, termTwo, answer, size, true, precision);
		*pt_Div_Com+=tempsent2[0];
		*pt_Div_rounds+=tempsent2[1];
	}
	// RSSVectorMyType scaledA(size);
	// multiplyByScalar(a, (1 << (alpha + 1)), scaledA);
	tempsent2=funcDotProduct(answer, a, quotient, size, true, ((2*precision-FLOAT_PRECISION)));	
	*pt_Div_Com+=tempsent2[0];
	*pt_Div_rounds+=tempsent2[1];
	*pt_Div_time+=clock()-clock_begin;
}

// a is of size batchSize*B, b is of size B and quotient is a/b (b from each group).
int BN_Com = 0;
int *pt_BN_Com = &BN_Com;
int BN_time = 0;
int *pt_BN_time = &BN_time;
int BN_rounds = 0;
int *pt_BN_rounds = &BN_rounds;
// int BN_ReLU_Com = 0;
// int *pt_BN_ReLU_Com = &BN_ReLU_Com;
void funcBatchNorm(const RSSVectorMyType &a, 
				const RSSVectorMyType &gamma, const RSSVectorMyType &beta, RSSVectorMyType &b,
				 size_t EPSILON,  size_t m,  size_t B)
{

	int clock_begin=clock();
	static int sent[2] = {0,0};
	log_print("funcBatchNorm");
	//TODO Scale up and complete this computation with higher fixed-point precision

	int size = m*B;
	#if SPLIT3
	size_t split_size = round((float)B/3);
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,B - split_size*2};
	std::vector<myType> share_a(length[nextParty(partyNum)]*m,0),share_gamma(length[nextParty(partyNum)],0),share_beta(length[nextParty(partyNum)],0),
	recv_a(length[partyNum]*m,0),recv_gamma(length[partyNum],0),recv_beta(length[partyNum],0);
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		share_a[i]=a[i+ind_P[nextParty(partyNum)]*m].first;
	}
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		share_gamma[i]=gamma[i+ind_P[nextParty(partyNum)]].first;
		share_beta[i]=beta[i+ind_P[nextParty(partyNum)]].first;
	}
	std::vector<char> databuff[4];
	int sizebuff[4];
	
	chip.ChipGenMask(share_a,share_a,length[nextParty(partyNum)]*m);
	chip.ChipGenMask(share_gamma,share_gamma,length[nextParty(partyNum)]);
	chip.ChipGenMask(share_beta,share_beta,length[nextParty(partyNum)]);
	sizebuff[0] = share_a.size()*sizeof(myType)+share_gamma.size()*sizeof(myType)+share_beta.size()*sizeof(myType);
	sizebuff[1] = recv_a.size()*sizeof(myType)+recv_gamma.size()*sizeof(myType)+recv_beta.size()*sizeof(myType);
	for(int i = 0;i<2;i++){
		databuff[i].resize(sizebuff[i]);
	}
	memcpy(&databuff[0][0],share_a.data(),share_a.size()*sizeof(myType));
	memcpy(&databuff[0][share_a.size()*sizeof(myType)],share_gamma.data(),share_gamma.size()*sizeof(myType));
	memcpy(&databuff[0][share_a.size()*sizeof(myType)+share_gamma.size()*sizeof(myType)],share_beta.data(),share_beta.size()*sizeof(myType));
	thread *threads = new thread[2];
	threads[0] = thread(sendVector<char>, ref(databuff[0]), nextParty(partyNum), databuff[0].size());
	sent[0]+=sizeof(char)*databuff[0].size();
	threads[1] = thread(receiveVector<char>, ref(databuff[1]), prevParty(partyNum), databuff[1].size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;

	memcpy(recv_a.data(),&databuff[1][0],recv_a.size()*sizeof(myType));
	memcpy(recv_gamma.data(),&databuff[1][recv_a.size()*sizeof(myType)],recv_gamma.size()*sizeof(myType));
	memcpy(recv_beta.data(),&databuff[1][recv_a.size()*sizeof(myType)+recv_gamma.size()*sizeof(myType)],recv_beta.size()*sizeof(myType));
	sent[0]+=sizebuff[0];
	sent[1]+=1;

	int input_count = chip_package_len-chip_package_len%m;
	if (input_count == 0){
		input_count = chip_package_len;
	}
	RSSVectorMyType tempb(length[partyNum]*m);
	RSSVectorMyType tempa(length[partyNum]*m),tempgamma(length[partyNum]),tempbeta(length[partyNum]);

	for(int i = 0; i < length[partyNum]*m; i++){
		tempa[i].first=a[i+ind_P[partyNum]*m].first;
		tempa[i].second=a[i+ind_P[partyNum]*m].second;
	}
	for(int i = 0; i < length[partyNum]; i++){
		tempgamma[i].first=gamma[i+ind_P[partyNum]].first;
		tempgamma[i].second=gamma[i+ind_P[partyNum]].second;
		tempbeta[i].first=beta[i+ind_P[partyNum]].first;
		tempbeta[i].second=beta[i+ind_P[partyNum]].second;
	}

	chip.ChipBN(tempa,recv_a,gamma,recv_gamma,beta, recv_beta, tempb, EPSILON, m ,length[partyNum]);
	std::vector<myType> share01(length[partyNum]*m,0),share02(length[partyNum]*m,0),
						recv01(length[prevParty(partyNum)]*m,0),recv02(length[nextParty(partyNum)]*m,0);

	for(int i = 0; i < length[partyNum]*m; i++){
			share01[i]=tempb[i].first;
			share02[i]=tempb[i].second;
	}


	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<myType>, ref(share01), prevParty(partyNum), share01.size());
	threads2[1] = thread(sendVector<myType>, ref(share02), nextParty(partyNum), share02.size());
	threads2[2] = thread(receiveVector<myType>, ref(recv01), prevParty(partyNum),recv01.size());
	threads2[3] = thread(receiveVector<myType>, ref(recv02), nextParty(partyNum),recv02.size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;

	for(int i = 0; i < length[partyNum]*m; i++){
		
		b[i+ind_P[partyNum]*m].first=tempb[i].first;
		b[i+ind_P[partyNum]*m].second=tempb[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]*m; i++){
		
		b[i+ind_P[prevParty(partyNum)]*m].first=recv01[i];
		b[i+ind_P[prevParty(partyNum)]*m].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		
		b[i+ind_P[nextParty(partyNum)]*m].first=chip.Chip_mask3[i%chip_package_len];
		b[i+ind_P[nextParty(partyNum)]*m].second=recv02[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> share_a(size,0),share_gamma(B,0),share_beta(B,0),
	recv_a(size,0),recv_gamma(B,0),recv_beta(B,0);
	
	if (partyNum == PARTY_C){
		for(int i = 0; i < size; i++){
			share_a[i]=a[i].first;
		}
		for(int i = 0; i < B; i++){

			share_gamma[i]=gamma[i].first;
			share_beta[i]=beta[i].first;
		}
		chip.ChipGenMask(share_a,share_a,size);

		chip.ChipGenMask(share_gamma,share_gamma,B);
		chip.ChipGenMask(share_beta,share_beta,B);
		
		sent[0]+=sendVector<myType>(share_a, PARTY_A, size);

		sent[0]+=sendVector<myType>(share_gamma, PARTY_A, B);
		sent[0]+=sendVector<myType>(share_beta, PARTY_A, B);
		sent[1]+=1;
	}

	if(partyNum==PARTY_A){
		receiveVector<myType>(recv_a, PARTY_C, size);
		receiveVector<myType>(recv_gamma, PARTY_C, B);
		receiveVector<myType>(recv_beta, PARTY_C, B);
	}

	sent[0]+=sizeof(myType)*size;
	sent[1]+=1;

	int input_count = chip_package_len-chip_package_len%m;
	if (input_count == 0){
		input_count = chip_package_len;
	}

	// cout<<"fin trans 2"<<endl;
	std::vector<myType> share01(size,0),share02(size,0),recv01(size,0),recv02(size,0);
	if(partyNum==PARTY_A){

		chip.ChipBN(a,recv_a,gamma,recv_gamma,
					beta, recv_beta, b, EPSILON, m ,B);

		for(int i = 0; i < size; i++){
			share01[i]=b[i].first;
			share02[i]=b[i].second;
		}
	}

	// cout<<"fin trans 2"<<endl;
	if(partyNum==PARTY_A){


		sent[0]+=sendVector<myType>(share01, PARTY_C, size);
		sent[0]+=sendVector<myType>(share02, PARTY_B, size);
		sendRoundfix(PARTY_A,1);

		sent[1]+=2;
	}

	if(partyNum==PARTY_B or partyNum==PARTY_C){

		receiveVector<myType>(recv01, PARTY_A, size);

		if(partyNum==PARTY_B){
			for(int i = 0; i < size; i++){
				b[i].first=recv01[i];
				b[i].second=chip.Chip_mask3[i%input_count];
			}
		}
		else{
			for(int i = 0; i < size; i++){
				b[i].first=chip.Chip_mask3[i%input_count];
				b[i].second=recv01[i];
			}
		}
	// cout<<"fin trans 3"<<endl;

	}
	#endif
	*pt_BN_Com+=sent[0];
	*pt_BN_rounds+=sent[1];
	*pt_BN_time+=clock()-clock_begin;
}


int MP_Com = 0;
int *pt_MP_Com = &MP_Com;
int MP_ReLU_Com = 0;
int *pt_MP_ReLU_Com = &MP_ReLU_Com;
int MP_time = 0;
int *pt_MP_time = &MP_time;
int MP_ReLU_time = 0;
int *pt_MP_ReLU_time = &MP_ReLU_time;
int MP_rounds = 0;
int *pt_MP_rounds = &MP_rounds;
//Chunk wise maximum of a vector of size rows*columns and maximum is caclulated of every 
//column number of elements. max is a vector of size rows, maxPrime, of rows*columns*columns; 
void funcMaxpool(RSSVectorMyType &a, RSSVectorMyType &max, RSSVectorSmallType &maxPrime,
						 size_t rows, size_t columns)
{

	int clock_begin=clock();
	static int sent[2]={0,0};
	int clock_relu=clock();
	log_print("funcMaxpool");
	// assert(columns < 256 && "Pooling size has to be smaller than 8-bits");
	int size = rows*columns;


	#if SPLIT3
	size_t B = rows;
	size_t m = columns;
	size_t split_size = round((float)B/3);
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,B - split_size*2};
	std::vector<myType> share_a(length[nextParty(partyNum)]*m,0),recv_a(length[partyNum]*m,0);
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		share_a[i]=a[i+ind_P[nextParty(partyNum)]*m].first;
	}

	chip.ChipGenMask(share_a,share_a,length[nextParty(partyNum)]*m);

	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(share_a), nextParty(partyNum), share_a.size());
	sent[0]+=sizeof(myType)*share_a.size();
	threads[1] = thread(receiveVector<myType>, ref(recv_a), prevParty(partyNum), recv_a.size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;
	int input_count = chip_package_len-chip_package_len%m;
	if (input_count == 0){
		input_count = chip_package_len;
	}
	RSSVectorMyType tempmax(length[partyNum]),tempa(length[partyNum]*m);
	RSSVectorSmallType tempmaxp(length[partyNum]*m);
	for(int i = 0; i < length[partyNum]*m; i++){
		tempa[i].first=a[i+ind_P[partyNum]*m].first;
		tempa[i].second=a[i+ind_P[partyNum]*m].second;
	}


	chip.ChipMax(recv_a,tempa,tempmax,tempmaxp,length[partyNum],m);

	std::vector<myType> share01(length[partyNum],0),share02(length[partyNum],0),
						recv01(length[prevParty(partyNum)],0),recv02(length[nextParty(partyNum)],0);
	std::vector<smallType> sharep1(length[partyNum]*m,0),sharep2(length[partyNum]*m,0),
						recvp1(length[prevParty(partyNum)]*m,0),recvp2(length[nextParty(partyNum)]*m,0);
	for(int i = 0; i < length[partyNum]; i++){
			share01[i]=tempmax[i].first;
			share02[i]=tempmax[i].second;
	}
	for(int i = 0; i < length[partyNum]*m; i++){
			sharep1[i]=tempmaxp[i].first;
			sharep2[i]=tempmaxp[i].second;
	}

	std::vector<char> databuff[4];
	int sizebuff[4];
	sizebuff[0] = share01.size()*sizeof(myType)+sharep1.size()*sizeof(smallType);
	sizebuff[1] = share02.size()*sizeof(myType)+sharep2.size()*sizeof(smallType);
	sizebuff[2] = recv01.size()*sizeof(myType)+recvp1.size()*sizeof(smallType);
	sizebuff[3] = recv02.size()*sizeof(myType)+recvp2.size()*sizeof(smallType);
	for(int i = 0;i<4;i++){
		databuff[i].resize(sizebuff[i]);
	}
	memcpy(&databuff[0][0],share01.data(),share01.size()*sizeof(myType));
	memcpy(&databuff[0][share01.size()*sizeof(myType)],sharep1.data(),sharep1.size()*sizeof(smallType));
	memcpy(&databuff[1][0],share02.data(),share02.size()*sizeof(myType));
	memcpy(&databuff[1][share02.size()*sizeof(myType)],sharep2.data(),sharep2.size()*sizeof(smallType));

	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<char>, ref(databuff[0]), prevParty(partyNum), databuff[0].size());
	threads2[1] = thread(sendVector<char>, ref(databuff[1]), nextParty(partyNum), databuff[1].size());
	threads2[2] = thread(receiveVector<char>, ref(databuff[2]), prevParty(partyNum),databuff[2].size());
	threads2[3] = thread(receiveVector<char>, ref(databuff[3]), nextParty(partyNum),databuff[3].size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;
	memcpy(recv01.data(),&((myType*)databuff[2].data())[0],recv01.size()*sizeof(myType));
	memcpy(recvp1.data(),&databuff[2][recv01.size()*sizeof(myType)],recvp1.size()*sizeof(smallType));
	memcpy(recv02.data(),&databuff[3][0],recv02.size()*sizeof(myType));
	memcpy(recvp2.data(),&databuff[3][recv02.size()*sizeof(myType)],recvp2 .size()*sizeof(smallType));

	for(int i = 0; i < length[partyNum]; i++){
		max[i+ind_P[partyNum]].first=tempmax[i].first;
		max[i+ind_P[partyNum]].second=tempmax[i].second;
	}
	for(int i = 0; i < length[partyNum]*m; i++){
		maxPrime[i+ind_P[partyNum]*m].first=tempmaxp[i].first;
		maxPrime[i+ind_P[partyNum]*m].second=tempmaxp[i].second;
	}

	for(int i = 0; i < length[prevParty(partyNum)]; i++){
		
		max[i+ind_P[prevParty(partyNum)]].first=recv01[i];
		max[i+ind_P[prevParty(partyNum)]].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[prevParty(partyNum)]*m; i++){
		maxPrime[i+ind_P[prevParty(partyNum)]*m].first=recvp1[i];
		maxPrime[i+ind_P[prevParty(partyNum)]*m].second=0;
	}
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		
		max[i+ind_P[nextParty(partyNum)]].first=chip.Chip_mask3[i%chip_package_len];
		max[i+ind_P[nextParty(partyNum)]].second=recv02[i];
	}
	
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		
		maxPrime[i+ind_P[nextParty(partyNum)]*m].first=0;
		maxPrime[i+ind_P[nextParty(partyNum)]*m].second=recvp2[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> d(size,0),share1(size,0),recv(size,0);
	if (partyNum == PARTY_C){
		for(int i = 0; i < size; i++){
			share1[i]=a[i].first;
		}
		chip.ChipGenMask(share1,d,size);
		sent[0]+=sendVector<myType>(d, PARTY_A, size);
		sent[1]+=1;
	}

	if(partyNum==PARTY_A){
		receiveVector<myType>(recv, PARTY_C, size);
	}

	sent[0]+=sizeof(myType)*size;
	sent[1]+=1;
	// if(ARDUINO){
		int input_count = chip_package_len-chip_package_len%columns;
		if (input_count == 0){
			input_count = chip_package_len;
		}
		int rows_apack = (int)(input_count/columns);
		// cout<<"fin trans 2"<<endl;
		std::vector<myType> share01(rows,0),share02(rows,0),recv01(rows,0),recv02(rows,0);
		if(partyNum==PARTY_A){
			chip.ChipMax(recv,a,max,maxPrime,rows,columns);

			for(int i = 0; i < rows; i++){
				share01[i]=max[i].first;
				share02[i]=max[i].second;
			}
		}
		// cout<<"fin trans 2"<<endl;
		if(partyNum==PARTY_A){


			sent[0]+=sendVector<myType>(share01, PARTY_C, rows);
			sent[0]+=sendVector<myType>(share02, PARTY_B, rows);
			sendRoundfix(PARTY_A,1);

			sent[1]+=2;
		}
		if(partyNum==PARTY_B or partyNum==PARTY_C){

			receiveVector<myType>(recv01, PARTY_A, rows);

			if(partyNum==PARTY_B){
			for(int i = 0; i < rows; i++){
				max[i].first=recv01[i];
				max[i].second=chip.Chip_mask3[i%rows_apack];
			}
			}
			else{
				for(int i = 0; i < rows; i++){
				max[i].first=chip.Chip_mask3[i%rows_apack];
				max[i].second=recv01[i];
			}
			}
		// cout<<"fin trans 3"<<endl;

		}

	#endif
	*pt_MP_Com+=sent[0];
	*pt_MP_rounds+=sent[1];
	
	*pt_MP_time+=clock()-clock_begin;
}

void funcMaxpool2(const RSSVectorMyType &a, RSSVectorMyType &max, RSSVectorSmallType &maxPrime,
						 size_t rows, size_t columns)
{

	int clock_begin=clock();
	static int sent[2]={0,0};
	int clock_relu=clock();
	log_print("funcMaxpool");
	// assert(columns < 256 && "Pooling size has to be smaller than 8-bits");
	int size = rows*columns;


	#if SPLIT3
	size_t B = rows;
	size_t m = columns;
	size_t split_size = round((float)B/3);
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,B - split_size*2};
	std::vector<myType> share_a(length[nextParty(partyNum)]*m,0),recv_a(length[partyNum]*m,0);
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		share_a[i]=a[i+ind_P[nextParty(partyNum)]*m].first;
	}

	chip.ChipGenMask(share_a,share_a,length[nextParty(partyNum)]*m);

	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(share_a), nextParty(partyNum), share_a.size());
	sent[0]+=sizeof(myType)*share_a.size();
	threads[1] = thread(receiveVector<myType>, ref(recv_a), prevParty(partyNum), recv_a.size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;
	int input_count = chip_package_len-chip_package_len%m;
	if (input_count == 0){
		input_count = chip_package_len;
	}
	RSSVectorMyType tempmax(length[partyNum]),tempa(length[partyNum]*m);
	RSSVectorSmallType tempmaxp(length[partyNum]);
	for(int i = 0; i < length[partyNum]*m; i++){
		tempa[i].first=a[i+ind_P[partyNum]*m].first;
		tempa[i].second=a[i+ind_P[partyNum]*m].second;
	}cout<<endl;
	chip.ChipMax(recv_a,tempa,tempmax,tempmaxp,length[partyNum],m);
	std::vector<myType> share01(length[partyNum],0),share02(length[partyNum],0),
						recv01(length[prevParty(partyNum)],0),recv02(length[nextParty(partyNum)],0);

	for(int i = 0; i < length[partyNum]; i++){
			share01[i]=tempmax[i].first;
			share02[i]=tempmax[i].second;
	}cout<<endl;


	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<myType>, ref(share01), prevParty(partyNum), share01.size());
	threads2[1] = thread(sendVector<myType>, ref(share02), nextParty(partyNum), share02.size());
	threads2[2] = thread(receiveVector<myType>, ref(recv01), prevParty(partyNum),recv01.size());
	threads2[3] = thread(receiveVector<myType>, ref(recv02), nextParty(partyNum),recv02.size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;

	for(int i = 0; i < length[partyNum]; i++){
		
		max[i+ind_P[partyNum]].first=tempmax[i].first;
		max[i+ind_P[partyNum]].second=tempmax[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]; i++){
		
		max[i+ind_P[prevParty(partyNum)]].first=recv01[i];
		max[i+ind_P[prevParty(partyNum)]].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[nextParty(partyNum)]; i++){
		
		max[i+ind_P[nextParty(partyNum)]].first=chip.Chip_mask3[i%chip_package_len];
		max[i+ind_P[nextParty(partyNum)]].second=recv02[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> d(size,0),share1(size,0),recv(size,0);
	if (partyNum == PARTY_C){
		for(int i = 0; i < size; i++){
			share1[i]=a[i].first;
		}
		chip.ChipGenMask(share1,d,size);
		sent[0]+=sendVector<myType>(d, PARTY_A, size);
		sent[1]+=1;
	}

	if(partyNum==PARTY_A){
		receiveVector<myType>(recv, PARTY_C, size);
	}

	sent[0]+=sizeof(myType)*size;
	sent[1]+=1;
	// if(ARDUINO){
		int input_count = chip_package_len-chip_package_len%columns;
		if (input_count == 0){
			input_count = chip_package_len;
		}
		int rows_apack = (int)(input_count/columns);
		// cout<<"fin trans 2"<<endl;
		std::vector<myType> share01(rows,0),share02(rows,0),recv01(rows,0),recv02(rows,0);
		if(partyNum==PARTY_A){
			chip.ChipMax(recv,a,max,maxPrime,rows,columns);

			for(int i = 0; i < rows; i++){
				share01[i]=max[i].first;
				share02[i]=max[i].second;
			}
		}
		// cout<<"fin trans 2"<<endl;
		if(partyNum==PARTY_A){


			sent[0]+=sendVector<myType>(share01, PARTY_C, rows);
			sent[0]+=sendVector<myType>(share02, PARTY_B, rows);
			sendRoundfix(PARTY_A,1);

			sent[1]+=2;
		}
		if(partyNum==PARTY_B or partyNum==PARTY_C){

			receiveVector<myType>(recv01, PARTY_A, rows);

			if(partyNum==PARTY_B){
			for(int i = 0; i < rows; i++){
				max[i].first=recv01[i];
				max[i].second=chip.Chip_mask3[i%rows_apack];
			}
			}
			else{
				for(int i = 0; i < rows; i++){
				max[i].first=chip.Chip_mask3[i%rows_apack];
				max[i].second=recv01[i];
			}
			}
		// cout<<"fin trans 3"<<endl;

		}

	#endif
	*pt_MP_Com+=sent[0];
	*pt_MP_rounds+=sent[1];
	
	*pt_MP_time+=clock()-clock_begin;
}


void funcAvgpool(const RSSVectorMyType &a, RSSVectorMyType &b, size_t rows, size_t columns){
	size_t batchSize = rows;
	size_t inputDim = columns;
	vector<myType> div_plain(batchSize,floatToMyType(1.0/inputDim));
	RSSVectorMyType div(batchSize);
	funcGetShares(div,div_plain);
	for (int i = 0; i < batchSize; i++){
		for (int j = 0; j < inputDim; j++){
			b[i].first += a[i*inputDim+j].first;
			b[i].second += a[i*inputDim+j].second;
		}

	}
	funcDotProduct(b,div,b,batchSize,true,FLOAT_PRECISION);
}

int Softmax_Com = 0;
int *pt_Softmax_Com = &Softmax_Com;
int Softmax_time = 0;
int *pt_Softmax_time = &Softmax_time;
int Softmax_rounds = 0;
int *pt_Softmax_rounds = &Softmax_rounds;
int* funcSoftmax(const RSSVectorMyType &a,  RSSVectorMyType &b, size_t rows, size_t columns, bool masked ){

	if(masked){
		assert(rows == columns && "Masking needs square matrix");
	}
	int clock_begin=clock();
	static int sent[2] = {0,0};
	int tempsent = 0;
	int* tempsent2;
	log_print("funcSoftmax");
	
	#if SPLIT3
	

	size_t B = rows;
	size_t m = columns;
	size_t split_size = round((float)B/3);
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,B - split_size*2};
	std::vector<myType> d(length[nextParty(partyNum)]*m,0);

	std::vector<longType> mm(length[nextParty(partyNum)]*m,0),share2(length[nextParty(partyNum)]*m,0),recv2(length[partyNum]*m,0);
	std::vector<longType> q(length[nextParty(partyNum)]*m,0),share1(length[nextParty(partyNum)]*m,0),recv1(length[partyNum]*m,0);

	int input_count = chip_package_len-chip_package_len%columns;
	if (input_count == 0){
		input_count = chip_package_len;
	}

	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		d[i]=a[i+ind_P[nextParty(partyNum)]*m].first+a[i+ind_P[nextParty(partyNum)]*m].second;
	}
	computeExp(d,mm,q,length[nextParty(partyNum)]*m);
	chip.ChipGenMask(q,share1,length[nextParty(partyNum)]*m);
	chip.ChipGenMask(mm,share2,length[nextParty(partyNum)]*m);

	std::vector<char> databuff[4];
	int sizebuff[4];

	sizebuff[0] = share1.size()*sizeof(longType)+share2.size()*sizeof(longType);
	sizebuff[1] = recv1.size()*sizeof(longType)+recv2.size()*sizeof(longType);
	for(int i = 0;i<2;i++){
		databuff[i].resize(sizebuff[i]);
	}
	memcpy(&databuff[0][0],share1.data(),share1.size()*sizeof(longType));
	memcpy(&databuff[0][share1.size()*sizeof(longType)],share2.data(),share2.size()*sizeof(longType));
	thread *threads = new thread[2];
	threads[0] = thread(sendVector<char>, ref(databuff[0]), nextParty(partyNum), databuff[0].size());
	sent[0]+=sizeof(char)*databuff[0].size();
	threads[1] = thread(receiveVector<char>, ref(databuff[1]), prevParty(partyNum), databuff[1].size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;
	memcpy(recv1.data(),&databuff[1][0],recv1.size()*sizeof(longType));
	memcpy(recv2.data(),&databuff[1][recv1.size()*sizeof(longType)],recv2.size()*sizeof(longType));

	RSSVectorMyType tempb(length[partyNum]*m);

	std::vector<myType> d2(length[partyNum]*m,0);
	std::vector<longType> mm2(length[partyNum]*m,0),qq2(length[(partyNum)]*m,0);
	for(int i = 0; i < length[partyNum]*m; i++){
		d2[i]=a[i+ind_P[partyNum]*m].second;
	}
	computeExp(d2,mm2,qq2,length[partyNum]*m);

	for(int i = 0; i < length[partyNum]*m; i++){
		qq2[i]=qq2[i]+recv1[i];
	}
	chip.ChipSoftmax(qq2,recv2,mm2,tempb,length[partyNum],m,masked,ind_P[partyNum]);
	std::vector<myType> share01(length[partyNum]*m,0),share02(length[partyNum]*m,0),
						recv01(length[prevParty(partyNum)]*m,0),recv02(length[nextParty(partyNum)]*m,0);

	for(int i = 0; i < length[partyNum]*m; i++){
			share01[i]=tempb[i].first;
			share02[i]=tempb[i].second;
	}

	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<myType>, ref(share01), prevParty(partyNum), share01.size());
	threads2[1] = thread(sendVector<myType>, ref(share02), nextParty(partyNum), share02.size());
	threads2[2] = thread(receiveVector<myType>, ref(recv01), prevParty(partyNum),recv01.size());
	threads2[3] = thread(receiveVector<myType>, ref(recv02), nextParty(partyNum),recv02.size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;

	for(int i = 0; i < length[partyNum]*m; i++){
		
		b[i+ind_P[partyNum]*m].first=tempb[i].first;
		b[i+ind_P[partyNum]*m].second=tempb[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]*m; i++){
		
		b[i+ind_P[prevParty(partyNum)]*m].first=recv01[i];
		b[i+ind_P[prevParty(partyNum)]*m].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		
		b[i+ind_P[nextParty(partyNum)]*m].first=chip.Chip_mask3[i%chip_package_len];
		b[i+ind_P[nextParty(partyNum)]*m].second=recv02[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	int size = rows*columns;
	std::vector<myType> d(size,0);

	std::vector<longType> m(size,0),share2(size,0),recv2(size,0);
	std::vector<longType> q(size,0),share1(size,0),recv1(size,0);

	int input_count = chip_package_len-chip_package_len%columns;
	if (input_count == 0){
		input_count = chip_package_len;
	}
	if (partyNum == PARTY_C){
		for(int i = 0; i < size; i++){
			d[i]=a[i].first+a[i].second;
		}
		computeExp(d,m,q,size);
		chip.ChipGenMask(q,share1,size);
		chip.ChipGenMask(m,share2,size);
		sent[0]+=sendVector<longType>(share1, PARTY_A, size);
		sent[0]+=sendVector<longType>(share2, PARTY_A, size);
		sendRoundfix(PARTY_C,1);
		sent[1]+=1;
	}

	std::vector<myType> share01(size,0),share02(size,0),recv01(size,0),recv02(size,0);

	// if(ARDUINO){
	if(partyNum==PARTY_A){
		receiveVector<longType>(recv1, PARTY_C, size);
		receiveVector<longType>(recv2, PARTY_C, size);
		receiveRoundfix(partyNum,1);
		for(int i = 0; i < size; i++){
			d[i]=a[i].second;
			
		}
		computeExp(d,m,q,size);
		for(int i = 0; i < size; i++){
			
			q[i]=q[i]+recv1[i];
			
		}
		chip.ChipSoftmax(q,recv2,m,b,rows,columns,masked);
		for(int i = 0; i < size; i++){
			share01[i]=b[i].first;
			share02[i]=b[i].second;
		}


		sent[0]+=sendVector<myType>(share01, PARTY_C, size);
		sent[0]+=sendVector<myType>(share02, PARTY_B, size);
		sendRoundfix(partyNum,1);
		sent[1]+=2;
	}
	if(partyNum==PARTY_B){
		receiveVector<myType>(recv01, PARTY_A, size);

		for(int i = 0; i < size; i++){
			b[i].first=recv01[i];
			b[i].second=chip.Chip_mask3[i%input_count];
		}
	}
		
	if(partyNum==PARTY_C){
		receiveVector<myType>(recv02, PARTY_A, size);

		for(int i = 0; i < size; i++){
		b[i].first=chip.Chip_mask3[i%input_count];
		b[i].second=recv02[i];
	}
	

	}
	#endif
	// cin.get();
	*pt_Softmax_Com+=sent[0];
	*pt_Softmax_rounds+=sent[1];
	*pt_Softmax_time+=clock()-clock_begin;

	return sent;
}

int LayerNorm_Com = 0;
int *pt_LayerNorm_Com = &LayerNorm_Com;
int LayerNorm_time = 0;
int *pt_LayerNorm_time = &LayerNorm_time;
int LayerNorm_rounds = 0;
int *pt_LayerNorm_rounds = &LayerNorm_rounds;
int* funcLayerNorm(const RSSVectorMyType &a, RSSVectorMyType &b, size_t rows, size_t columns){
	int clock_begin=clock();
	static int sent[2] = {0,0};
	int tempsent = 0;
	int* tempsent2;
	log_print("funcLayerNorm");
	size_t size  = rows*columns;

	#if SPLIT3
	size_t B = rows;
	size_t m = columns;
	size_t split_size = round((float)B/3);
	size_t ind_P[3] = {0,split_size,split_size*2};
	size_t length[3] = {split_size,split_size,B - split_size*2};
	std::vector<myType> share_a(length[nextParty(partyNum)]*m,0),recv_a(length[partyNum]*m,0);
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		share_a[i]=a[i+ind_P[nextParty(partyNum)]*m].first;
	}

	std::vector<char> databuff[4];
	int sizebuff[4];
	
	chip.ChipGenMask(share_a,share_a,length[nextParty(partyNum)]*m);

	thread *threads = new thread[2];
	threads[0] = thread(sendVector<myType>, ref(share_a), nextParty(partyNum), share_a.size());
	sent[0]+=sizeof(char)*databuff[0].size();
	threads[1] = thread(receiveVector<myType>, ref(recv_a), prevParty(partyNum), recv_a.size());
	for (int i = 0; i < 2; i++)
		threads[i].join();
	delete[] threads;


	int input_count = chip_package_len-chip_package_len%m;
	if (input_count == 0){
		input_count = chip_package_len;
	}
	RSSVectorMyType tempb(length[partyNum]*m);
	RSSVectorMyType tempa(length[partyNum]*m),tempgamma(length[partyNum]),tempbeta(length[partyNum]);

	for(int i = 0; i < length[partyNum]*m; i++){
		tempa[i].first=a[i+ind_P[partyNum]*m].first;
		tempa[i].second=a[i+ind_P[partyNum]*m].second;
	}


	chip.ChipLayerNorm(recv_a,tempa,tempb,length[partyNum],m);
	std::vector<myType> share01(length[partyNum]*m,0),share02(length[partyNum]*m,0),
						recv01(length[prevParty(partyNum)]*m,0),recv02(length[nextParty(partyNum)]*m,0);

	for(int i = 0; i < length[partyNum]*m; i++){
			share01[i]=tempb[i].first;
			share02[i]=tempb[i].second;
	}


	thread *threads2 = new thread[4];
	threads2[0] = thread(sendVector<myType>, ref(share01), prevParty(partyNum), share01.size());
	threads2[1] = thread(sendVector<myType>, ref(share02), nextParty(partyNum), share02.size());
	threads2[2] = thread(receiveVector<myType>, ref(recv01), prevParty(partyNum),recv01.size());
	threads2[3] = thread(receiveVector<myType>, ref(recv02), nextParty(partyNum),recv02.size());
	for (int i = 0; i < 4; i++)
		threads2[i].join();
	delete[] threads2;

	for(int i = 0; i < length[partyNum]*m; i++){
		b[i+ind_P[partyNum]*m].first=tempb[i].first;
		b[i+ind_P[partyNum]*m].second=tempb[i].second;
	}
	for(int i = 0; i < length[prevParty(partyNum)]*m; i++){
		
		b[i+ind_P[prevParty(partyNum)]*m].first=recv01[i];
		b[i+ind_P[prevParty(partyNum)]*m].second=chip.Chip_mask3[i%chip_package_len];
	}
	for(int i = 0; i < length[nextParty(partyNum)]*m; i++){
		
		b[i+ind_P[nextParty(partyNum)]*m].first=chip.Chip_mask3[i%chip_package_len];
		b[i+ind_P[nextParty(partyNum)]*m].second=recv02[i];
	}
	chip.ChipGenMask(length[nextParty(partyNum)]+length[prevParty(partyNum)]);
	#else
	std::vector<myType> d(size,0),share1(size,0),recv(size,0);
	for(int i = 0; i < size; i++){
		share1[i]=a[i].first;
	}

	if (partyNum == PARTY_C){
		chip.ChipGenMask(share1,d,size);
		sent[0]+=sendVector<myType>(d, PARTY_A, size);
		sent[1]+=1;
	}

	std::vector<myType> share01(size,0),share02(size,0),recv01(size,0);
	std::vector<smallType> sharep01(size,0),sharep02(size,0),recv02(size,0);

	// if(ARDUINO){
		if(partyNum==PARTY_A){
			receiveVector<myType>(recv, PARTY_C, size);
			chip.ChipLayerNorm(recv,a,b,rows,columns);
			for(int i = 0; i < size; i++){
				share01[i]=b[i].first;
				share02[i]=b[i].second;
			}
		}

		if(partyNum==PARTY_A){

			sent[0]+=sendVector<myType>(share01, PARTY_C, size);
			sent[0]+=sendVector<myType>(share02, PARTY_B, size);
			sendRoundfix(partyNum,1);

			sent[1]+=2;
		}
		if(partyNum==PARTY_B or partyNum==PARTY_C){

			receiveVector<myType>(recv01, PARTY_A, size);
			// receiveVector<smallType>(recv02, PARTY_A, size);

			if(partyNum==PARTY_B){
			for(int i = 0; i < size; i++){
				b[i].first=recv01[i];
				b[i].second=chip.Chip_mask3[i%chip_package_len];
			}
			}
			else{
				for(int i = 0; i < size; i++){
				b[i].first=chip.Chip_mask3[i%chip_package_len];
				b[i].second=recv01[i];

			}
			}

		}

	#endif
	*pt_LayerNorm_Com+=sent[0];
	*pt_LayerNorm_rounds+=sent[1];
	*pt_LayerNorm_time+=clock()-clock_begin;
	return sent;
}

/****************************************************************/
/* 							DEBUG 								*/
/****************************************************************/

/* This is quite interesting, negatives move more towards the exact
computation. So it might be the case that the approximations 
introduced by negative and positive numbers in effect cancel out to
preserve overall NN accuracy. */
void debugMatMul()
{
	// size_t rows = 1000; 
	// size_t common_dim = 1000;
	// size_t columns = 1000;
	// size_t transpose_a = 0, transpose_b = 0;

	// RSSVectorMyType a(rows*common_dim, make_pair(1,1)), 
	// 				b(common_dim*columns, make_pair(1,1)), c(rows*columns);

	// funcMatMul(a, b, c, rows, common_dim, columns, transpose_a, transpose_b, FLOAT_PRECISION);

/******************************** Attempt 1 ****************************************/	
// 	size_t rows = 3; 
// 	size_t common_dim = 2;
// 	size_t columns = 3;
// 	size_t transpose_a = 0, transpose_b = 0;

// 	RSSVectorMyType a(rows*common_dim), b(common_dim*columns), c(rows*columns);
// 	vector<myType> a_reconst(rows*columns), b_reconst(common_dim*columns), c_reconst(rows*columns); 

// 	vector<myType> data_a = {floatToMyType(3),floatToMyType(4),
// 							 floatToMyType(5),floatToMyType(6),
// 							 floatToMyType(7),floatToMyType(8)};
// 	vector<myType> data_b = {floatToMyType(4),floatToMyType(5),floatToMyType(6),
// 							 floatToMyType(7),floatToMyType(8),floatToMyType(9)};
// 	funcGetShares(a, data_a);
// 	funcGetShares(b, data_b);

// 	funcMatMul(a, b, c, rows, common_dim, columns, transpose_a, transpose_b, FLOAT_PRECISION);

// #if (!LOG_DEBUG)
// 	funcReconstruct(a, a_reconst, rows*common_dim, "a", true);
// 	funcReconstruct(b, b_reconst, common_dim*columns, "b", true);
// 	funcReconstruct(c, c_reconst, rows*columns, "c", true);
// #endif

/******************************** Attemp2 ****************************************/	
	size_t rows = 2; 
	size_t common_dim = 2;
	size_t columns = 2;
	size_t transpose_a = 0, transpose_b = 0;

	RSSVectorMyType a(rows*common_dim), b(common_dim*columns), c(rows*columns);
	// vector<myType> a_reconst(rows*columns), b_reconst(common_dim*columns), c_reconst(rows*columns); 

// A, B 		: 0001011001110, 1000011000101, 0110001100111, 0110101000001
// 				: 718, 4293, 3175, 3393
// C(exact)  	: 14145799/67108864, 17648523/67108864, 13052425/67108864, 6285681/16777216
// C(MPC)		: 1726, 2154, 1593, 3069
//Float(exact)	: [0.21069336, 0.26293945, 0.19445801, 0.37463379]
//Float(MPC)	: [0.21078883, 0.26298349, 0.19449629, 0.37465578]
//Rel error (%)	: [0.04531312, 0.01674641, 0.01968604, 0.00587083] --> 0.022% error
	vector<myType> data_a = {-718,-4293,-3175,-3393};
	vector<myType> data_b = {718,4293,3175,3393};
	funcGetShares(a, data_a);
	funcGetShares(b, data_b);

	funcMatMul(a, b, c, rows, common_dim, columns, transpose_a, transpose_b, FLOAT_PRECISION);

	print_vector(a, "FLOAT", "a", a.size());	
	print_vector(b, "FLOAT", "b", b.size());	
	print_vector(c, "FLOAT", "c", c.size());	
// #if (!LOG_DEBUG)
// 	funcReconstruct(a, a_reconst, rows*common_dim, "a", true);
// 	funcReconstruct(b, b_reconst, common_dim*columns, "b", true);
// 	funcReconstruct(c, c_reconst, rows*columns, "c", true);
// #endif

/******************************** Attemp3 ****************************************/	
// 	size_t rows = 1; 
// 	size_t common_dim = 1;
// 	size_t columns = 1;
// 	size_t transpose_a = 0, transpose_b = 0;

// 	RSSVectorMyType a(rows*common_dim), b(common_dim*columns), c(rows*columns);

// // A, B 		: 0001011001110, 1000011000101, 0110001100111, 0110101000001
// // 				: 718, 4293, 3175, 3393
// // C(exact)  	: 14145799/67108864, 17648523/67108864, 13052425/67108864, 6285681/16777216
// // C(MPC)		: 1726, 2154, 1593, 3069
// //Float(exact)	: [0.21069336, 0.26293945, 0.19445801, 0.37463379]
// //Float(MPC)	: [0.21078883, 0.26298349, 0.19449629, 0.37465578]
// //Rel error (%)	: [0.04531312, 0.01674641, 0.01968604, 0.00587083] --> 0.022% error
// 	vector<myType> data_a = {-718};
// 	vector<myType> data_b = {718};
// 	funcGetShares(a, data_a);
// 	funcGetShares(b, data_b);

// 	funcMatMul(a, b, c, rows, common_dim, columns, transpose_a, transpose_b, FLOAT_PRECISION);

// 	print_vector(a, "FLOAT", "a", a.size());	
// 	print_vector(b, "FLOAT", "b", b.size());	
// 	print_vector(c, "FLOAT", "c", c.size());

/******************************** Attemp4 ****************************************/	
	// size_t rows = 1; 
	// size_t common_dim = 1;
	// size_t columns = 1;
	// size_t transpose_a = 0, transpose_b = 0;

	// RSSVectorMyType a(rows*common_dim), b(common_dim*columns), c(rows*columns);

	// vector<myType> data_a = {-718};
	// vector<myType> data_b = {718};
	// funcGetShares(a, data_a);
	// funcGetShares(b, data_b);

	// myType result = a[0].first * b[0].first +
 //                    a[0].first * b[0].second +
 //                    a[0].second * b[0].first;

	// // print_linear(data_a[0], "SIGNED");
	// // print_linear(result, "SIGNED");
	// print_linear(dividePlain(result, (1 << FLOAT_PRECISION)), "SIGNED");
}

void debugMatMulMix()
{
	size_t rows = 2; 
	size_t common_dim = 2;
	size_t columns = 2;
	size_t transpose_a = 0, transpose_b = 0;

	RSSVectorMyType a(rows*common_dim), b(common_dim*columns), c(rows*columns);
	RSSVectorSmallType d(rows*common_dim);

	vector<myType> data_a = {718,4293,3175,-3393};
	vector<myType> data_b = {718,4293,3175,3393};
	funcGetShares(a, data_a);
	funcGetShares(b, data_b);

	funcMatMul_mixed(a, b, c, d, rows, common_dim, columns, transpose_a, transpose_b, FLOAT_PRECISION, 0, 0, 0,
	0, 0, 1 );

	print_vector(a, "FLOAT", "a", a.size());	
	print_vector(b, "FLOAT", "b", b.size());	
	print_vector(c, "FLOAT", "c", c.size());	

}

void debugVecMul()
{
	
	size_t rows = 1; 
	size_t columns = 4;
	size_t transpose_a = 0, transpose_b = 0;

	RSSVectorMyType a(rows*columns), b(rows*columns), c(rows*columns*columns);
	vector<myType> data_a = {-718,-4293,-3175,-3393};
	// vector<myType> data_b = {718,4293,3175,3393};
	funcGetShares(a, data_a);
	// funcGetShares(b, data_b);

	funcVecMul(a, c, rows, columns, FLOAT_PRECISION);
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(c, "FLOAT", "c", c.size());	

}


void debugDotProd()
{
	/****************************** myType ***************************/
	// size_t rows = 3; 
	// size_t columns = 3;

	// RSSVectorMyType a(rows*columns, make_pair(0,0)), 
	// 				b(rows*columns, make_pair(0,0)), 
	// 				c(rows*columns);
	// vector<myType> a_reconst(rows*columns), b_reconst(rows*columns), c_reconst(rows*columns); 

	// vector<myType> data = {floatToMyType(3),floatToMyType(4),floatToMyType(5),
	// 						 floatToMyType(6),floatToMyType(7),floatToMyType(8), 
	// 						 floatToMyType(7),floatToMyType(8),floatToMyType(9)};
	// funcAddConstant(a, data);
	// funcAddConstant(b, data);

	// funcReconstruct(a, a_reconst, rows*columns, "a", true);
	// funcReconstruct(b, b_reconst, rows*columns, "b", true);
	// funcDotProduct(a, b, c, rows*columns, true, FLOAT_PRECISION);
	// funcReconstruct(c, c_reconst, rows*columns, "c", true);

	/****************************** smallType ***************************/
	size_t size = 9; 

	RSSVectorSmallType a(size, make_pair(1,1)), 
					   b(size, make_pair(1,1)), 
					   c(size);

	funcDotProduct(a, b, c, size);
}


void debugPC()
{
	vector<myType> plain_m{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	vector<myType> plain_r{ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}; 
	vector<smallType> plain_beta{ 1, 0, 1, 0, 0, 0, 1, 1, 0, 1};
	size_t size = plain_m.size();
	size_t sizeLong = size*BIT_SIZE;
	assert(plain_r.size() == plain_m.size() && "Error in debugPC");

	RSSVectorSmallType beta(size), shares_m(sizeLong);
	vector<smallType> reconst_betaP(size), betaPrime(size);
	funcGetShares(beta, plain_beta);

	vector<smallType> bits_of_m(sizeLong);
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < BIT_SIZE; ++j)
			bits_of_m[i*BIT_SIZE + j] = (smallType)((plain_m[i] >> (BIT_SIZE-1-j)) & 1);

	funcGetShares(shares_m, bits_of_m);
	funcPrivateCompare(shares_m, plain_r, beta, betaPrime, size);
	
#if (!LOG_DEBUG)
	cout << "BetaPrime: \t ";
	for (int i = 0; i < size; ++i)
		cout << (int)betaPrime[i] << " ";
	cout << endl;
	cout << "Beta: \t\t";
	for (int i = 0; i < size; ++i)
		cout << (int)plain_beta[i] << " ";
	cout << endl;
	cout << "m: \t\t";
	for (int i = 0; i < size; ++i)
		cout << (int)plain_m[i] << " ";
	cout << endl;
	cout << "r: \t\t";
	for (int i = 0; i < size; ++i)
		cout << (int)plain_r[i] << " ";
	cout << endl;
	cout << "m-r: \t\t";
	for (int i = 0; i < size; ++i)
		cout << (int)plain_m[i] - (int)plain_r[i] << " ";
	cout << endl;
#endif
}

void debugWrap()
{
	size_t size = 5;
	RSSVectorMyType a(size);
	RSSVectorSmallType theta(size);
	vector<smallType> reconst(size);

	myType interesting = MINUS_ONE/3;
	a[0] = make_pair(0,0);
	a[1] = make_pair(interesting, interesting);
	interesting += 1;	
	a[2] = make_pair(interesting, interesting);
	interesting = ((MINUS_ONE/3) << 1);
	a[3] = make_pair(interesting, interesting);
	interesting += 1;	
	a[4] = make_pair(interesting, interesting);

	funcWrap(a, theta, size);

#if (!LOG_DEBUG)
	cout << "a: \t\t ";
	for (int i = 0; i < size; ++i)
		cout << (int)3*a[i].first << "(" << (int) a[i].first << ", " << (int)a[i].second << ") ";
	cout << endl; 
	funcReconstruct(theta, reconst, size, "Theta", true);
#endif
}


void debugReLUPrime()
{
	vector<myType> data_a = {1, 2, -1, -2, 3};
	size_t size = data_a.size();
	RSSVectorMyType a(size);
	RSSVectorSmallType b(size);
	vector<myType> reconst_a(size);
	vector<smallType> reconst_b(size);

	funcGetShares(a, data_a);
	funcRELUPrime(a, b, size);

#if (!LOG_DEBUG)
	funcReconstruct(a, reconst_a, size, "a", true);
	funcReconstructBit(b, reconst_b, size, "b", true);
#endif
}


void debugReLU()
{
	vector<myType> data_a {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
	size_t size = data_a.size();
	RSSVectorMyType a(size), b(size);
	RSSVectorSmallType aPrime(size);

	vector<smallType> reconst_ap(size);
	vector<myType> reconst_b(size);

	funcGetShares(a, data_a);
	for (int i = size/2; i < size; ++i)
	{
		a[i].first = -a[i].first;
		a[i].second = -a[i].second ;
	}

	funcRELU(a, aPrime, b, size);

#if (!LOG_DEBUG)
	funcReconstruct(a, data_a, size, "a", true);
	funcReconstruct(b, reconst_b, size, "ReLU", true);
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(b, "FLOAT", "b", b.size());		
	funcReconstructBit(aPrime, reconst_ap, size, "ap", true);
#endif
}


void debugDivision()
{
	vector<myType> data_a = {floatToMyType(300)}, data_b = {floatToMyType(900)};
	size_t size = data_a.size();
	RSSVectorMyType a(size), b(size), quotient(size);
	vector<myType> reconst(size);

	funcGetShares(a, data_a);
	funcGetShares(b, data_b);
	funcDivision(a, b, quotient, size);

#if (!LOG_DEBUG)
	funcReconstruct(a, reconst, size, "a", true);
	funcReconstruct(b, reconst, size, "b", true);
	funcReconstruct(quotient, reconst, size, "Quot", true);
	print_myType(reconst[0], "Quotient[0]", "FLOAT");
#endif	
}


//TODO This needs thorough debugging. Many issues with this function.
void debugBN()
{
	// vector<myType> data_a = {1<<13}, data_b = {(1<<14) + (1<<13)};
	//Values over and above 10, use lower fixed-point precision. 13 bits + another 2-3 powers of 2 is too much.
	vector<myType> data_a = {floatToMyType(-2),floatToMyType(2),floatToMyType(-4),floatToMyType(4),
	floatToMyType(-2),floatToMyType(2),floatToMyType(-4),floatToMyType(4),
	floatToMyType(-2),floatToMyType(2),floatToMyType(-4),floatToMyType(4)};
	vector<myType> data_gamma = {floatToMyType(2),floatToMyType(2),floatToMyType(2),};
	vector<myType> data_beta = {floatToMyType(1),floatToMyType(1),floatToMyType(1),};
	size_t size = data_a.size();
	size_t B = data_gamma.size();
	size_t m = size / B;
	RSSVectorMyType a(size), b(size), gamma(B),beta(B);
	vector<myType> reconst(size);

	funcGetShares(a, data_a);
	funcGetShares(gamma, data_gamma);
	funcGetShares(beta, data_beta);
	funcBatchNorm(a, gamma, beta, b, floatToMyType(0), m, B);

#if (!LOG_DEBUG)
	funcReconstruct(a, reconst, size, "a", true);
	funcReconstruct(b, reconst, size, "b", true);
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(b, "FLOAT", "b", b.size());	
#endif	
}

void debugSoftmax()
{
	
	// size_t rows = 2; 
	// size_t columns = 4;
	// RSSVectorMyType a(rows*columns), b(rows*columns), c(rows*columns);
	// vector<myType> data_a = {-718,4293,-3175,3393, 21938,32839,-13414,14511};
	// // vector<myType> data_b = {718,4293,3175,3393};
	// funcGetShares(a, data_a);
	// // funcGetShares(b, data_b);
	size_t rows = 3; 
	size_t columns = 3;
	RSSVectorMyType a(rows*columns), b(rows*columns), c(rows*columns);
	int p2=pow(2,FLOAT_PRECISION);
	// vector<myType> data_a= {-3.96313*p2,2.62659*p2,-3.03662*p2,8.7168*p2,-6.46106*p2,
	// 9.04565*p2,-0.949707*p2,-8.29395*p2,2.47485*p2,2.80188*p2};
	vector<myType> data_a= {-32466,21517,-24876,71408,-52929,74102,-7780,-7944,20274};//,22953};
	// vector<myType> data_a= {11111,11111,11111,11111,11111,11111,11111,11111,11111};//,22953};
	funcGetShares(a, data_a);
	funcSoftmax(a, c, rows, columns);
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(c, "FLOAT", "c", c.size());
	funcSoftmax(a, c, rows, columns, true);
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(c, "FLOAT", "c", c.size());	

}


void debugSSBits()
{
	size_t rows = 4;
	size_t columns = 3;
	vector<smallType> a0 = {1,0,0,1,1,1,0,1,1,0,0,0};
	vector<smallType> a1 = {0,1,1,0,0,0,1,0,0,0,0,0,
							1,0,0,1,1,1,0,1,1,0,1,1,
							1,1,0,1,1,0,0,1,0,1,0,1};
	vector<smallType> rp = {0,0,1,1};
	RSSVectorSmallType x(rows*columns), y(rows*columns*columns), z(rows), answer(rows*columns);
	funcGetShares(x, a0);
	funcGetShares(y, a1);
	funcGetShares(z, rp);

#if (!LOG_DEBUG)
	funcReconstructBit(x, a1, x.size(), "x", true);
	funcReconstructBit(y, a1, y.size(), "y", true);
	funcReconstructBit(z, a1, z.size(), "z", true);
#endif	

	for (int i = 0; i < 3; ++i)
	{
		funcSelectBitShares(x, y, z, answer, rows, columns, i);
#if (!LOG_DEBUG)
		funcReconstructBit(answer, a1, answer.size(), "a", true);
#endif			
	}

}


void debugSS()
{

	vector<smallType> bits = {1,0,0,1,1,1,0,1,1,0};
	size_t size = bits.size();
	vector<myType> data = {1,29,10,2938,27,-1,-23,12,2,571}, reconst(size);
	assert(size == data.size() && "Size mismatch");
	RSSVectorMyType a(size), selection(size);
	RSSVectorSmallType b(size);

	funcGetShares(a, data);
	funcGetShares(b, bits);

	funcSelectShares(a, b, selection, size);

#if (!LOG_DEBUG)
	funcReconstruct(a, reconst, size, "a", true);
	funcReconstructBit(b, bits, size, "b", true);
	funcReconstruct(selection, reconst, size, "Sel'd", true);
#endif	
}




void debugMaxpool()
{

	size_t rows = 5;
	size_t columns = 3;
	size_t size = rows*columns;
	vector<myType> data = {1,2,3,
						   -3,-1,-2,
						   1,5,3,
						   5,1,6,
						   6,3,9}, reconst(size);
	RSSVectorMyType a(size), max(rows);
	RSSVectorSmallType maxPrime(rows*columns);
	vector<smallType> reconst_maxPrime(maxPrime.size());
	funcGetShares(a, data);
	funcMaxpool(a, max, maxPrime, rows, columns);
	

#if (!LOG_DEBUG)
	funcReconstruct(a, reconst, size, "a", true);
	funcReconstruct(max, reconst, rows, "val", true);
	funcReconstructBit(maxPrime, reconst_maxPrime, rows*columns, "maxP", true);
#endif	
}

void debugAvgpool()
{

	size_t rows = 5;
	size_t columns = 3;
	size_t size = rows*columns;
	vector<myType> data = {1,2,3,
						   3,1,2,
						   1,5,3,
						   5,1,6,
						   6,3,9}, reconst(size);
	for (int i =0; i< size;i++){
		data[i]=floatToMyType(data[i]);
	}
	RSSVectorMyType a(size), b(rows);
	funcGetShares(a, data);
	funcAvgpool(a,b,rows,columns);
	

#if (!LOG_DEBUG)
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(b, "FLOAT", "b", b.size());
#endif	
}

void debugLN()
{

	size_t rows = 5;
	size_t columns = 3;
	size_t size = rows*columns;
	vector<myType> data = {1,2,3,
						   3,1,2,
						   1,5,3,
						   5,1,6,
						   6,3,9}, reconst(size);
	for (int i =0; i< size;i++){
		data[i]=floatToMyType(data[i]);
	}
	RSSVectorMyType a(size), b(size);
	funcGetShares(a, data);
	funcLayerNorm(a,b,rows,columns);
	

#if (!LOG_DEBUG)
	print_vector(a, "FLOAT", "a", a.size());		
	print_vector(b, "FLOAT", "b", b.size());
#endif	
}
/******************************** Test ********************************/

void testMatMul(size_t rows, size_t common_dim, size_t columns, size_t iter)
{

/******************************** TODO ****************************************/	
	RSSVectorMyType a(rows*common_dim, make_pair(0,0));
	RSSVectorMyType b(common_dim*columns, make_pair(0,0));
	RSSVectorMyType c(rows*columns);

	for (int runs = 0; runs < iter; ++runs)
		funcMatMul(a, b, c, rows, common_dim, columns, 0, 0, FLOAT_PRECISION);
}


void testConvolution(size_t iw, size_t ih, size_t Din, size_t Dout, 
					size_t f, size_t S, size_t P, size_t B, size_t iter)
{
	// size_t ow 		= (((iw-f+2*P)/S)+1);
	// size_t oh		= (((ih-f+2*P)/S)+1);
	// size_t tempSize = ow*oh;

	// RSSVectorMyType a(iw*ih*Din*B, make_pair(0,0));
	// RSSVectorMyType b(f*f*Din*Dout, make_pair(0,0));
	// RSSVectorMyType ans(ow*oh*Dout*B, make_pair(0,0));
	// RSSVectorMyType c(Dout, make_pair(0,0));

	// for (int runs = 0; runs < iter; ++runs)
	// {
	// 	//Reshape activations
	// 	RSSVectorMyType temp1((iw+2*P)*(ih+2*P)*Din*B, make_pair(0,0));
	// 	zeroPad(a, temp1, iw, ih, P, Din, B);

	// 	//Reshape for convolution
	// 	RSSVectorMyType temp2((f*f*Din) * (ow * oh * B));
	// 	convToMult(temp1, temp2, (iw+2*P), (ih+2*P), f, Din, S, B);

	// 	//Perform the multiplication, transpose the actications.
	// 	RSSVectorMyType temp3(Dout * (ow*oh*B));
	// 	funcMatMul(b, temp2, temp3, Dout, (f*f*Din), (ow*oh*B), 0, 1, FLOAT_PRECISION);

	// 	//Add biases and meta-transpose
	// 	for (size_t i = 0; i < B; ++i)
	// 		for (size_t j = 0; j < Dout; ++j) 
	// 			for (size_t k = 0; k < tempSize; ++k)
	// 				ans[i*Dout*tempSize + j*tempSize + k] 
	// 					= temp3[j*B*tempSize + i*tempSize + k] + c[j];		
	// }
}


void testRelu(size_t r, size_t c, size_t iter)
{
	RSSVectorMyType a(r*c, make_pair(0,0));
	RSSVectorSmallType reluPrime(r*c);
	RSSVectorMyType b(r*c);

	for (int runs = 0; runs < iter; ++runs)
		funcRELU(a, reluPrime, b, r*c);
}


void testReluPrime(size_t r, size_t c, size_t iter)
{
	RSSVectorMyType a(r*c, make_pair(0,0));
	RSSVectorSmallType reluPrime(r*c);

	for (int runs = 0; runs < iter; ++runs)
		funcRELUPrime(a, reluPrime, r*c);
}


void testMaxpool(size_t ih, size_t iw, size_t Din, size_t f, size_t S, size_t B, size_t iter)
{
	size_t ow 		= (((iw-f)/S)+1);
	size_t oh		= (((ih-f)/S)+1);

	RSSVectorMyType a(iw*ih*Din*B);
	RSSVectorMyType b(ow*oh*Din*B);
	RSSVectorSmallType c(iw*ih*Din*B);
	RSSVectorMyType temp1(ow*oh*Din*B*f*f);
	size_t sizeBeta = iw;
	size_t sizeD 	= sizeBeta*ih;
	size_t sizeB 	= sizeD*Din;
	size_t counter 	= 0;

	for (int runs = 0; runs < iter; ++runs)
	{
		counter = 0;
		for (int b = 0; b < B; ++b)
			for (size_t r = 0; r < Din; ++r)
				for (size_t beta = 0; beta < ih-f+1; beta+=S) 
					for (size_t alpha = 0; alpha < iw-f+1; alpha+=S)
						for (int q = 0; q < f; ++q)
							for (int p = 0; p < f; ++p)
							{
								temp1[counter++] = 
								a[b*sizeB + r*sizeD + (beta + q)*sizeBeta + (alpha + p)];
							}
		//Pooling operation
		funcMaxpool(temp1, b, c, ow*oh*Din*B, f*f);
	}
}
