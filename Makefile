CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall

all: sequential openmp_solution mpi_solution

sequential: sequential.c
	$(CC) $(CFLAGS) -o sequential sequential.c

openmp_solution: openmp_solution.c
	$(CC) $(CFLAGS) -fopenmp -o openmp_solution openmp_solution.c

mpi_solution: mpi_solution.c
	$(MPICC) $(CFLAGS) -o mpi_solution mpi_solution.c

clean:
	rm -f sequential openmp_solution mpi_solution

.PHONY: all clean
