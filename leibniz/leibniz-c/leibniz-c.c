#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <omp.h>
#include <immintrin.h> // For AVX intrinsics

// pi = 3.14159265358979323846...

const uint64_t n8  =   100000000;    // 8 zeros
const uint64_t n9  =  1000000000;   // 9 zeros
const uint64_t n10 = 10000000000; // 10 zeros

/* Naive version */
int leibniz_1() {
  double tmp = 0.0;
  for (uint64_t i = 0; i <= n8; i++) {
    tmp = tmp + pow(-1, i) / (2.0 * i + 1.0);
  }
  tmp = 4 * tmp;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}

/* Change of flag */
int leibniz_2() {
  double tmp = 0.0;
  bool flag = true;
  for (uint64_t i = 0; i <= n9; i++) {
    if (flag) {
      tmp = tmp + 1.0 / (2.0 * i + 1.0);
    } else {
      tmp = tmp - 1.0 / (2.0 * i + 1.0);
    }
    flag = !flag;
  }
  tmp = 4 * tmp;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}

/* Basic version */
int leibniz_3() {
  double tmp = 0.0;
  double sign = 1.0;
  for (uint64_t i = 0; i <= n10; i++) {
    tmp = tmp + sign / (2.0 * i + 1.0);
    sign = -sign;
  }
  tmp = 4 * tmp;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}

/* 2-loop unrolling to avoid change of sign */
int leibniz_3b() {
  double tmp = 0.0;
  const uint64_t nmax = n10 - 1;
  for (uint64_t i = 0; i <= nmax; i += 2) {
    tmp += 1 / (2.0 * i + 1.0);
    tmp -= 1 / (2.0 * i + 3.0);
  }
  tmp = 4 * tmp;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}

/* 4-loop unrolling */
int leibniz_4() {
    double tmp = 0.0;
    const uint64_t nmax = n10 - 3;
    for (uint64_t i = 0; i <= nmax; i += 4) {
      tmp += 1 / (2.0 * i + 1.0);
      tmp -= 1 / (2.0 * (i + 1) + 1.0);
      tmp += 1 / (2.0 * (i + 2) + 1.0);
      tmp -= 1 / (2.0 * (i + 3) + 1.0);
    }
    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

/* basic pragma parallelization */
int leibniz_5() {
    double tmp = 0.0;
    uint64_t nmax = n10 - 1;

    #pragma omp parallel for reduction(+:tmp)
    for (uint64_t i = 0; i <= nmax; i += 2) {
      tmp += 1 / (2.0 * i + 1.0);
      tmp -= 1 / (2.0 * (i + 1) + 1.0);
     }
    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

/* basic pragma parallelization with 16-loop unrolling */
int leibniz_6() {
    double tmp = 0.0;
    uint64_t nmax = n10 - 15;

    #pragma omp parallel for reduction(+:tmp)
    for (uint64_t i = 0; i <= nmax; i += 16) {
      tmp += 1 / (2.0 * i + 1.0);
      tmp -= 1 / (2.0 * (i + 1) + 1.0);
      tmp += 1 / (2.0 * (i + 2) + 1.0);
      tmp -= 1 / (2.0 * (i + 3) + 1.0);
      tmp += 1 / (2.0 * (i + 4) + 1.0);
      tmp -= 1 / (2.0 * (i + 5) + 1.0);
      tmp += 1 / (2.0 * (i + 6) + 1.0);
      tmp -= 1 / (2.0 * (i + 7) + 1.0);
      tmp += 1 / (2.0 * (i + 8) + 1.0);
      tmp -= 1 / (2.0 * (i + 9) + 1.0);
      tmp += 1 / (2.0 * (i + 10) + 1.0);
      tmp -= 1 / (2.0 * (i + 11) + 1.0);
      tmp += 1 / (2.0 * (i + 12) + 1.0);
      tmp -= 1 / (2.0 * (i + 13) + 1.0);
      tmp += 1 / (2.0 * (i + 14) + 1.0);
      tmp -= 1 / (2.0 * (i + 15) + 1.0);
    }
    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

/* pragma parallelization with chunks */
int leibniz_6b() {
  double tmp = 0.0;
  uint64_t nmax = n10 - 1;
  int chunk_size = nmax / 1000;
  
#pragma omp parallel for schedule(static, chunk_size) reduction(+:tmp)
  for (uint64_t i = 0; i <= nmax; i += 2) {
    tmp += 1 / (2.0 * i + 1.0);
    tmp -= 1 / (2.0 * (i + 1) + 1.0);
  }
  tmp *= 4;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}


/* SIMD vectorization with 8-array for float precision, but no parallelization */
int leibniz_7() {
    float tmp = 0.0f;
    uint64_t nmax = n10 - 7;

    __m256 vec_tmp = _mm256_setzero_ps(); // Initialize vector to zero

    for (uint64_t i = 0; i <= nmax; i += 8) {
        __m256 vec_i = _mm256_set_ps(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
        __m256 vec_two = _mm256_set1_ps(2.0f);
        __m256 vec_one = _mm256_set1_ps(1.0f);
        __m256 vec_sign = _mm256_set_ps(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

        __m256 vec_denom = _mm256_fmadd_ps(vec_two, vec_i, vec_one); // 2.0 * i + 1.0
        __m256 vec_term = _mm256_div_ps(vec_sign, vec_denom); // sign / (2.0 * i + 1.0)

        vec_tmp = _mm256_add_ps(vec_tmp, vec_term); // Accumulate results
    }

    // Sum the vector elements
    float tmp_array[8];
    _mm256_storeu_ps(tmp_array, vec_tmp);
    for (int j = 0; j < 8; j++) {
        tmp += tmp_array[j];
    }

    tmp *= 4.0f;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

/* SIMD vectorization with 4-array for double precision, but no parallelization */
int leibniz_8() {
    double tmp = 0.0;
    uint64_t nmax = n10 - 3;

    __m256d vec_tmp = _mm256_setzero_pd(); // Initialize vector to zero

    for (uint64_t i = 0; i <= nmax; i += 4) {
        __m256d vec_i = _mm256_set_pd(i + 3, i + 2, i + 1, i);
        __m256d vec_two = _mm256_set1_pd(2.0);
        __m256d vec_one = _mm256_set1_pd(1.0);
        __m256d vec_sign = _mm256_set_pd(-1.0, 1.0, -1.0, 1.0);

        __m256d vec_denom = _mm256_fmadd_pd(vec_two, vec_i, vec_one); // 2.0 * i + 1.0
        __m256d vec_term = _mm256_div_pd(vec_sign, vec_denom); // sign / (2.0 * i + 1.0)

        vec_tmp = _mm256_add_pd(vec_tmp, vec_term); // Accumulate results
    }

    // Sum the vector elements
    double tmp_array[4];
    _mm256_storeu_pd(tmp_array, vec_tmp);
    tmp += tmp_array[0] + tmp_array[1] + tmp_array[2] + tmp_array[3];

    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

/* SIMD vectorization with 4-array for double precision, and basic pragma parallelization */
int leibniz_9() {
    double tmp = 0.0;
    uint64_t nmax = n10 - 4;

    #pragma omp parallel
    {
        __m256d vec_tmp = _mm256_setzero_pd(); // Initialize vector to zero

#pragma omp for // or for simd
        for (uint64_t i = 0; i <= nmax; i += 4) {
            __m256d vec_i = _mm256_set_pd(i + 3, i + 2, i + 1, i);
            __m256d vec_two = _mm256_set1_pd(2.0);
            __m256d vec_one = _mm256_set1_pd(1.0);
            __m256d vec_sign = _mm256_set_pd(-1.0, 1.0, -1.0, 1.0);

            __m256d vec_denom = _mm256_fmadd_pd(vec_two, vec_i, vec_one); // 2.0 * i + 1.0
            __m256d vec_term = _mm256_div_pd(vec_sign, vec_denom); // sign / (2.0 * i + 1.0)

            vec_tmp = _mm256_add_pd(vec_tmp, vec_term); // Accumulate results
        }

        // Sum the vector elements
        double tmp_array[4];
        _mm256_storeu_pd(tmp_array, vec_tmp);
        double local_tmp = tmp_array[0] + tmp_array[1] + tmp_array[2] + tmp_array[3];
                
        #pragma omp atomic
        tmp += local_tmp;
    }

    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}


/* SIMD vectorization with 4-array for double precision, and basic pragma parallelization */
int leibniz_9b() {
    double tmp = 0.0;
    const uint64_t nmax = n10 - 4;
    const int chunk_size = nmax / 1000;

    #pragma omp parallel
    {
        __m256d vec_tmp = _mm256_setzero_pd(); // Initialize vector to zero

#pragma omp for schedule(static, chunk_size)
        for (uint64_t i = 0; i <= nmax; i += 4) {
            __m256d vec_i = _mm256_set_pd(i + 3, i + 2, i + 1, i);
            __m256d vec_two = _mm256_set1_pd(2.0);
            __m256d vec_one = _mm256_set1_pd(1.0);
            __m256d vec_sign = _mm256_set_pd(-1.0, 1.0, -1.0, 1.0);

            __m256d vec_denom = _mm256_fmadd_pd(vec_two, vec_i, vec_one); // 2.0 * i + 1.0
            __m256d vec_term = _mm256_div_pd(vec_sign, vec_denom); // sign / (2.0 * i + 1.0)

            vec_tmp = _mm256_add_pd(vec_tmp, vec_term); // Accumulate results
        }

        // Sum the vector elements
        double tmp_array[4];
        _mm256_storeu_pd(tmp_array, vec_tmp);
        double local_tmp = tmp_array[0] + tmp_array[1] + tmp_array[2] + tmp_array[3];
                
        #pragma omp atomic
        tmp += local_tmp;
    }

    tmp *= 4;
    printf("Result: %.20f\n", tmp);
    return EXIT_SUCCESS;
}

int main(int argc, [[maybe_unused]] char* argv[argc+1]) {
  struct timespec start, end;
  double duration;
  clock_gettime(CLOCK_MONOTONIC, &start);
  
  leibniz_9b();
  
  clock_gettime(CLOCK_MONOTONIC, &end);
  duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Duration: %f seconds\n", duration);
  return EXIT_SUCCESS;
}

// end
