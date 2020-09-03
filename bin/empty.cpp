#include "bench/bench.hpp"

#include <mpi.h>

void empty(bench::State &state) {
  for (auto _ : state) {
  }
}

int main(int argc, char **argv) {

  bench::init(argc, argv);
  bench::register_bench("empty", empty)->timing_root_rank()->no_iter_barrier();
  bench::run_benchmarks();
  bench::finalize();
}