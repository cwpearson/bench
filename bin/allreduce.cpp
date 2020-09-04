#include "bench/bench.hpp"

#include <mpi.h>

void allreduce(bench::State &state) {

  const int rank = bench::world_rank();
  const int size = bench::world_size();

  const size_t sz = 100000;

  char *data = new char[sz];
  for (auto _ : state) {
    MPI_Allreduce(MPI_IN_PLACE, data, sz, MPI_BYTE, MPI_SUM, MPI_COMM_WORLD);
  }

  state.set_bytes_processed(sz);
  delete[] data;
}

BENCH(allreduce)->timing_max_rank();
BENCH_MAIN()