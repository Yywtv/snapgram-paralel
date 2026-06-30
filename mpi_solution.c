/*
 * mpi_solution.c
 * Solusi Paralel 2 (Distributed Memory) untuk SnapGram menggunakan MPI.
 *
 * Strategi paralelisasi:
 *   - Proses rank 0 membangkitkan matriks M, kemudian seluruh matriks M
 *     di-broadcast (MPI_Bcast) ke semua proses, karena setiap proses butuh
 *     akses penuh ke M untuk menghitung baris hasilnya sendiri.
 *   - Baris-baris matriks hasil R dibagi rata (dengan sisa pembagian
 *     didistribusikan ke proses pertama) ke setiap proses. Pembagian ini
 *     mirip dengan pendekatan row-wise decomposition yang umum dipakai
 *     pada perkalian matriks paralel berbasis MPI.
 *   - Setiap proses menghitung blok barisnya secara lokal, lalu langsung
 *     mencari nilai maksimum & posisi kolom untuk baris-baris tersebut
 *     (tidak perlu mengirim seluruh matriks R, cukup hasil maksimumnya
 *     saja -> menghemat komunikasi).
 *   - Hasil (nilai maks + posisi kolom per baris) dikumpulkan ke rank 0
 *     dengan MPI_Gatherv karena jumlah baris per proses bisa tidak rata.
 *
 * Kompilasi : mpicc -O2 -o mpi_solution mpi_solution.c
 * Jalankan  : mpirun -np <jumlah_proses> ./mpi_solution N [seed]
 *   N    = ukuran matriks
 *   seed = seed RNG (default 42, harus sama dengan versi sekuensial/OpenMP
 *          untuk verifikasi kebenaran hasil)
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Usage: mpirun -np P %s N [seed]\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    int N    = atoi(argv[1]);
    int seed = (argc > 2) ? atoi(argv[2]) : 42;

    int *M = malloc((size_t)N * N * sizeof(int));
    if (!M) {
        fprintf(stderr, "Rank %d gagal alokasi memori untuk N=%d\n", rank, N);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Hanya rank 0 yang membangkitkan data, lalu disebarkan ke semua proses */
    if (rank == 0) {
        srand(seed);
        for (long i = 0; i < (long)N * N; i++) {
            M[i] = rand() % 100;
        }
    }

    double t_start = MPI_Wtime();

    MPI_Bcast(M, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    /* Pembagian baris: rowsPerProc baris untuk semua proses, ditambah 1
       baris ekstra untuk proses dengan rank < remainder */
    int rowsPerProc = N / size;
    int remainder   = N % size;
    int numRows  = rowsPerProc + (rank < remainder ? 1 : 0);
    int startRow = rank * rowsPerProc + (rank < remainder ? rank : remainder);

    long long *localMax    = malloc((size_t)numRows * sizeof(long long));
    int       *localMaxCol = malloc((size_t)numRows * sizeof(int));

    for (int li = 0; li < numRows; li++) {
        int i = startRow + li;
        long long best = 0;
        int bestCol = 0;
        for (int j = 0; j < N; j++) {
            long long sum = 0;
            for (int k = 0; k < N; k++) {
                sum += (long long)M[i * N + k] * M[k * N + j];
            }
            if (j == 0 || sum > best) {
                best = sum;
                bestCol = j;
            }
        }
        localMax[li]    = best;
        localMaxCol[li] = bestCol;
    }

    /* Siapkan recvcounts & displs untuk MPI_Gatherv (hanya perlu di rank 0) */
    int *recvCounts = NULL, *displs = NULL;
    long long *globalMax    = NULL;
    int       *globalMaxCol = NULL;

    if (rank == 0) {
        recvCounts = malloc(size * sizeof(int));
        displs     = malloc(size * sizeof(int));
        globalMax    = malloc((size_t)N * sizeof(long long));
        globalMaxCol = malloc((size_t)N * sizeof(int));

        int offset = 0;
        for (int p = 0; p < size; p++) {
            int rp = rowsPerProc + (p < remainder ? 1 : 0);
            recvCounts[p] = rp;
            displs[p]     = offset;
            offset += rp;
        }
    }

    MPI_Gatherv(localMax, numRows, MPI_LONG_LONG,
                globalMax, recvCounts, displs, MPI_LONG_LONG,
                0, MPI_COMM_WORLD);
    MPI_Gatherv(localMaxCol, numRows, MPI_INT,
                globalMaxCol, recvCounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    double elapsed = MPI_Wtime() - t_start;
    double maxElapsed;
    MPI_Reduce(&elapsed, &maxElapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("[MPI] N=%d, processes=%d, waktu eksekusi=%.6f detik\n", N, size, maxElapsed);
        printf("Contoh: Row 0     -> max=%lld, kolom=%d\n", globalMax[0], globalMaxCol[0]);
        printf("Contoh: Row %-5d -> max=%lld, kolom=%d\n", N - 1, globalMax[N - 1], globalMaxCol[N - 1]);
        free(recvCounts); free(displs); free(globalMax); free(globalMaxCol);
    }

    free(M); free(localMax); free(localMaxCol);
    MPI_Finalize();
    return 0;
}
