/*
 * openmp_solution.c
 * Solusi Paralel 1 (Shared Memory) untuk SnapGram menggunakan OpenMP.
 *
 * Strategi paralelisasi:
 *   - Setiap baris i dari matriks hasil R bersifat independen terhadap
 *     baris lain (hanya butuh seluruh isi M, tidak menulis ke baris lain),
 *     sehingga loop terluar (baris) dibagi ke thread dengan
 *     "#pragma omp parallel for". schedule(dynamic) dipakai supaya beban
 *     kerja tetap seimbang walau ada variasi waktu antar baris.
 *   - Pencarian nilai maksimum per baris juga dibagi per baris dengan cara
 *     yang sama -> tidak ada race condition karena setiap thread menulis
 *     ke elemen array yang berbeda (rowMax[i], rowMaxCol[i]).
 *
 * Kompilasi : gcc -O2 -fopenmp -o openmp_solution openmp_solution.c
 * Jalankan  : ./openmp_solution N [seed] [num_threads]
 *   N            = ukuran matriks
 *   seed         = seed RNG (default 42, harus sama dengan versi lain
 *                  untuk verifikasi kebenaran hasil)
 *   num_threads  = jumlah thread (default: jumlah core yang tersedia)
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s N [seed] [num_threads]\n", argv[0]);
        return 1;
    }
    int N          = atoi(argv[1]);
    int seed       = (argc > 2) ? atoi(argv[2]) : 42;
    int numThreads = (argc > 3) ? atoi(argv[3]) : omp_get_max_threads();
    omp_set_num_threads(numThreads);

    int       *M = malloc((size_t)N * N * sizeof(int));
    long long *R = malloc((size_t)N * N * sizeof(long long));
    long long *rowMax    = malloc((size_t)N * sizeof(long long));
    int       *rowMaxCol = malloc((size_t)N * sizeof(int));

    if (!M || !R || !rowMax || !rowMaxCol) {
        fprintf(stderr, "Gagal alokasi memori untuk N=%d\n", N);
        return 1;
    }

    srand(seed);
    for (long i = 0; i < (long)N * N; i++) {
        M[i] = rand() % 100;
    }

    double t_start = omp_get_wtime();

    /* 1. Perkalian matriks R = M x M, dibagi per baris ke setiap thread */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            long long sum = 0;
            for (int k = 0; k < N; k++) {
                sum += (long long)M[i * N + k] * M[k * N + j];
            }
            R[i * N + j] = sum;
        }
    }

    /* 2. Cari nilai maksimum per baris, juga dibagi per baris */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; i++) {
        long long best = R[i * N + 0];
        int bestCol = 0;
        for (int j = 1; j < N; j++) {
            if (R[i * N + j] > best) {
                best = R[i * N + j];
                bestCol = j;
            }
        }
        rowMax[i]    = best;
        rowMaxCol[i] = bestCol;
    }

    double elapsed = omp_get_wtime() - t_start;

    printf("[OPENMP] N=%d, threads=%d, waktu eksekusi=%.6f detik\n", N, numThreads, elapsed);
    printf("Contoh: Row 0     -> max=%lld, kolom=%d\n", rowMax[0], rowMaxCol[0]);
    printf("Contoh: Row %-5d -> max=%lld, kolom=%d\n", N - 1, rowMax[N - 1], rowMaxCol[N - 1]);

    free(M); free(R); free(rowMax); free(rowMaxCol);
    return 0;
}
