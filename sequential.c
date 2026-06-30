/*
 * sequential.c
 * Baseline sekuensial untuk SnapGram:
 *   1. Perkalian matriks R = M x M (M berukuran N x N)
 *   2. Pencarian nilai maksimum per baris pada R, beserta posisi kolomnya
 *
 * Kompilasi : gcc -O2 -o sequential sequential.c
 * Jalankan  : ./sequential N [seed]
 *   N    = ukuran matriks (misal 1024, 2048, 4096)
 *   seed = seed random number generator (default 42), agar hasil bisa
 *          dibandingkan apple-to-apple dengan versi OpenMP/MPI
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s N [seed]\n", argv[0]);
        return 1;
    }
    int N    = atoi(argv[1]);
    int seed = (argc > 2) ? atoi(argv[2]) : 42;

    int       *M = malloc((size_t)N * N * sizeof(int));
    long long *R = malloc((size_t)N * N * sizeof(long long));
    long long *rowMax    = malloc((size_t)N * sizeof(long long));
    int       *rowMaxCol = malloc((size_t)N * sizeof(int));

    if (!M || !R || !rowMax || !rowMaxCol) {
        fprintf(stderr, "Gagal alokasi memori untuk N=%d\n", N);
        return 1;
    }

    /* Isi matriks M dengan integer acak 0..99 */
    srand(seed);
    for (long i = 0; i < (long)N * N; i++) {
        M[i] = rand() % 100;
    }

    clock_t t_start = clock();

    /* 1. Perkalian matriks R = M x M */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            long long sum = 0;
            for (int k = 0; k < N; k++) {
                sum += (long long)M[i * N + k] * M[k * N + j];
            }
            R[i * N + j] = sum;
        }
    }

    /* 2. Cari nilai maksimum per baris */
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

    clock_t t_end = clock();
    double elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;

    printf("[SEQUENTIAL] N=%d, waktu eksekusi=%.6f detik\n", N, elapsed);
    printf("Contoh: Row 0     -> max=%lld, kolom=%d\n", rowMax[0], rowMaxCol[0]);
    printf("Contoh: Row %-5d -> max=%lld, kolom=%d\n", N - 1, rowMax[N - 1], rowMaxCol[N - 1]);

    /* Catatan: cetak waktu di baris pertama dengan format konsisten supaya
       mudah diparsing skrip benchmarking milik Analis Kinerja. */

    free(M); free(R); free(rowMax); free(rowMaxCol);
    return 0;
}
