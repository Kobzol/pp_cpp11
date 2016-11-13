#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <xmmintrin.h>
#include <immintrin.h>
#include <malloc.h>

static void fill(float* p, int size)
{
	for (int i = 0; i < size; i++)
	{
		p[i] = rand() * (float) rand();
	}
}


#define VECTORIZE

void vectorize()
{
	srand((unsigned int) time(NULL));
	const size_t size = 1024 * 1024 * 64;
    const size_t malloc_size = size  * sizeof(float);
    const int iterations = 3;

    constexpr size_t align = 32;
#ifdef VECTORIZE
    constexpr int increment = align / 4;
#else
    constexpr int increment = 1;
#endif

	float* result, *mul, *add;
    assert(!posix_memalign((void**) &result, align, malloc_size));
    assert(!posix_memalign((void**) &mul, align, malloc_size));
    assert(!posix_memalign((void**) &add, align, malloc_size));

	fill(mul, size);
	fill(add, size);

    assert((long)mul % align == 0);
    assert((long)add % align == 0);
    assert((long)result % align == 0);

    auto start = std::chrono::steady_clock::now();
    for (int it = 0; it < iterations; it++)
    {
        for (int i = 0; i < size; i += increment)
        {
#ifdef VECTORIZE
            __m256 vecMul = _mm256_load_ps(mul + i);
            __m256 vecAdd = _mm256_load_ps(add + i);

            vecMul = _mm256_mul_ps(vecMul, vecMul);

            _mm256_store_ps(result + i, _mm256_fmadd_ps(vecAdd, vecAdd, vecMul));
#else
            result[i] = mul[i] * mul[i] + add[i] * add[i];
#endif
        }
    }

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
    ).count() / iterations << std::endl;

    free(mul);
    free(add);
    free(result);
}

/*
__m128 vecMul = _mm_load_ps(mul + i);
__m128 vecAdd = _mm_load_ps(add + i);

vecMul = _mm_mul_ps(vecMul, vecMul);

_mm_store_ps(result + i, _mm_fmadd_ps(vecAdd, vecAdd, vecMul));
 */