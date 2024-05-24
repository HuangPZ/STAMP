#include "globals.h"
#if (USE_CUDA)
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>


#include <stdio.h> 
#include <iostream>
#include "Config.h"
#include "../util/TedKrovetzAesNiWrapperC.h"
#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <vector>
#include <time.h>
#include "secCompMultiParty.h"
#include "main_gf_funcs.h"
#include <string>
#include <openssl/sha.h>
#include <math.h>
#include <sstream>
#include "AESObject.h"
#include "connect.h"

#include "CPU.h"

void matrixMultRSS_Cuda(const RSSVectorMyType &a, const RSSVectorMyType &b, vector<myType> &temp3, 
					size_t rows, size_t common_dim, size_t columns,
				 	size_t transpose_a, size_t transpose_b);
void vectorMultRSS_Cuda(const RSSVectorMyType &a, vector<myType> &temp3, size_t rows, size_t columns);	
#endif