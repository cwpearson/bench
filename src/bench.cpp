#include "bench/bench.hpp"

#ifdef BENCH_USE_MPI
#include <mpi.h>
#endif

#include <iostream>
#include <vector>

namespace bench {

void init(int &argc, char **&argv) {
#ifdef BENCH_USE_MPI
  MPI_Init(&argc, &argv);
#endif
}

void finalize() {
#ifdef BENCH_USE_MPI
  MPI_Finalize();
#endif
}

int world_rank() {
#ifdef BENCH_USE_MPI
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return rank;
#endif
  return 0;
}

int world_size() {
#ifdef BENCH_USE_MPI
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return size;
#endif
  return 1;
}

void run_benchmarks() {

  for (Benchmark *benchmark : benchmarks) {
    if (0 == world_rank()) {
      std::cerr << "running " << benchmark->name() << "\n";
    }

    // estimate the time per iteration

    // decide how many iterations to run
    uint64_t iters = 10000;

    State state(iters, benchmark->timer(), benchmark->iter_barrier());

#ifdef BENCH_USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    benchmark->run(state);

    /*reporter
     */
    if (world_rank() == 0) {

      double iters = state.iterations();
      double nsElapsed = benchmark->timer()->get_elapsed() / iters;
      double sElapsed = nsElapsed / 1e9;
      double bytes = state.bytes_processed();


      std::cout << benchmark->name() << ": " << nsElapsed << "ns";
      if (state.bytes_processed()) {
        std::cout << " " << bytes / sElapsed << "B/s";
      }
      std::cout << "\n";
    }
  }
}

Benchmark *register_bench(const char *name, Function fn) {
  benchmarks.push_back(new FunctionBenchmark(name, fn));
  return benchmarks.back();
}

void FunctionBenchmark::run(State &state) { fn_(state); }

/*extern*/ std::vector<Benchmark *> benchmarks;

} // namespace bench