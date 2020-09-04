#include "bench/bench.hpp"

#include <mpi.h>

void empty(bench::State &state) {
  for (auto _ : state) {
  }
}

BENCH(empty)->timing_root_rank()->no_iter_barrier();
BENCH_MAIN()