#include "bench/bench.hpp"

#include <mpi.h>

void pingpong(bench::State &state) {

  const int rank = bench::world_rank();
  const int size = bench::world_size();

  const size_t sz = 1;

  char *sbuf = new char[sz];
  char *rbuf = new char[sz];

  for (auto _ : state) {
    if (0 == rank) {
      MPI_Send(sbuf, sz, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(rbuf, sz, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (1 == rank) {
      MPI_Recv(rbuf, sz, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(sbuf, sz, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }
  }

  state.set_bytes_processed(sz);
  delete[] sbuf;
  delete[] rbuf;
}

BENCH(pingpong)->timing_root_rank()->no_iter_barrier();
BENCH_MAIN()