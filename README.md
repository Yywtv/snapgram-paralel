# SnapGram — Perkalian Matriks & Pencarian Maksimum per Baris

Perkalian matriks M x M
(matriks dikalikan dengan dirinya sendiri) dan pencarian nilai maksimum
beserta posisi kolomnya pada setiap baris hasil.

## Isi

- `sequential.c` — baseline sekuensial (tanpa paralelisasi).
- `openmp_solution.c` — Solusi Paralel 1: shared-memory, multi-core, dengan OpenMP.
- `mpi_solution.c` — Solusi Paralel 2: distributed-memory dengan MPI.
- `Makefile` — kompilasi ketiga program sekaligus.

## Instalasi prasyarat

- Compiler GCC dengan dukungan OpenMP (biasanya sudah ada di GCC modern).
- Implementasi MPI, misalnya OpenMPI (`sudo apt install libopenmpi-dev openmpi-bin`) atau MPICH.

atau dengan docker:

- Docker

```bash
git clone https://github.com/Yywtv/snapgram-paralel.git
```

## Setup

### Kompilasi

```bash
make            # build sequential, openmp_solution, mpi_solution
make clean      # bersihkan binary
```

Atau manual:

```bash
gcc -O2 -o sequential sequential.c
gcc -O2 -fopenmp -o openmp_solution openmp_solution.c
mpicc -O2 -o mpi_solution mpi_solution.c
```

Atau di Docker...

### Docker

```bash
docker build -t snapgram-project .    # pastiin working directory di snapgram_project
```

## Menjalankan program

```bash
# Sekuensial
./sequential 1024 42

# OpenMP (argumen ke-3 = jumlah thread, opsional)
./openmp_solution 1024 42 8

# MPI (jumlah proses ditentukan lewat mpirun -np)
mpirun -np 4 ./mpi_solution 1024 42
```

Atau di Docker...

```bash
# Sekuensial
docker run --rm snapgram-project ./sequential 1024 42

# OpenMP (argumen ke-3 = jumlah thread, opsional)
docker run --rm snapgram-project ./openmp_solution 1024 42 8

# MPI (jumlah proses ditentukan lewat mpirun -np)
docker run --rm snapgram-project mpirun --allow-run-as-root -np 4 ./mpi_solution 1024 42

Argumen:
1. `N` — ukuran matriks (disarankan 1024, 2048, atau 4096 sesuai soal).
2. `seed` — seed random number generator. **Gunakan seed yang sama** di
   ketiga program agar matriks input identik, sehingga hasil bisa
   diverifikasi sama persis (syarat kebenaran di Tahap 2 pengerjaan).
3. (khusus OpenMP) jumlah thread.

## Verifikasi kebenaran hasil

Jalankan ketiga program dengan `N` dan `seed` yang sama, lalu bandingkan
baris "Contoh: Row ..." pada output. Nilai max dan posisi kolom untuk
baris 0 dan baris N-1 harus identik di ketiga versi. Untuk pengujian lebih
ketat, Analis Kinerja dapat memodifikasi program agar menulis seluruh
array `rowMax`/`rowMaxCol` ke file dan membandingkannya dengan `diff`.

## Skenario benchmarking yang disarankan

- Variasikan `N`: 1024, 2048, 4096.
- Variasikan jumlah thread OpenMP: 1, 2, 4, 8, ... (dibandingkan dengan
  sequential sebagai baseline untuk speedup = T_sequential / T_parallel).
- Variasikan jumlah proses MPI: 1, 2, 4, 8, ...
- Catat waktu eksekusi dari output program (sudah dicetak otomatis), lalu
  hitung speedup dan efisiensi (efisiensi = speedup / jumlah_unit_proses).

## Catatan desain

- Setiap baris hasil perkalian matriks dihitung secara independen,
  sehingga dibagi per baris ke thread (OpenMP, `#pragma omp parallel for`)
  atau ke proses (MPI, row-wise decomposition). Pendekatan ini
  menghindari race condition tanpa perlu lock/critical section.
- Tipe `long long` dipakai untuk hasil perkalian agar aman dari overflow
  pada N besar (nilai elemen matriks 0–99).
- Pada versi MPI, hanya nilai maksimum & posisi kolom per baris yang
  dikumpulkan ke rank 0 (bukan seluruh matriks hasil), untuk mengurangi
  biaya komunikasi antar proses.
