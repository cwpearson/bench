# bench

Protoype C++11 MPI benchmark support library inspired by [google/benchmark](github.com/google/benchmark).

## Benchmark Loop

An example ping-pong benchmark (bin/pingpong.cpp)

```c++
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

int main(int argc, char **argv) {
  bench::init(argc, argv);
  bench::register_bench("pingpong", pingpong)->timing_root_rank()->no_iter_barrier();
  bench::run_benchmarks();
  bench::finalize();
}
```

The library will automatically determine the number of iterations to run.

Before the `pingpong` function is called, the library will call `MPI_Barrier(MPI_COMM_WORLD)`.
Then, `pingpong` will be called.
Setup code happens before the `auto _ : state` loop.
Each iteration of the loop contributes to the total time.
After each iteration, an `MPI_Barrier(MPI_COMM_WORLD)` is invoked, it's time does not contribute (see `Benchmark::no_iter_barrier()`.
After the loop, benchmark-specific teardown occurs.
`timing_root_rank()` says that the reported timing should be tracked just by elapsed time on the root rank.
`no_iter_barrier()` says that there should be no `MPI_Barrier()` between state iterations.

## Reporting

The reported time the average ns/iteration.
If `state.set_bytes_processed` is used, the provided value should be the number of bytes per iteration.
The reported number of bytes will be bytes / second.

## 

* `Benchmark::timing_max_rank()`: report the maximum time consumed across all ranks
* `Benchmark::timing_root_rank()`: only record time in rank 0
* `Benchmark::no_iter_barrier()`: Do not do an `MPI_Barrier()` between iterations.

## Roadmap

- [ ] Automatic Timing
  - [x] `timing_root_rank`
  - [x] `timing_max_rank`
  - [ ] `timing_wall`: the wall time from the first rank starts to the last rank ends
  - [ ] `timing_aggregate`: aggregate time consumed in each rank
- [ ] Manual timing
  - [x] state.pause_timing()
  - [x] state.resume_timing()
  - [ ] state.set_iteration_time()
- [ ] Iteration control
  - [ ] manual
  - [ ] automatic
- [ ] Support running a benchmark over multiple communicators
  - [ ] Benchmark must take a communicator
  - [ ] All pairs of ranks
  - [ ] Specific pairs of ranks
- [ ] CSV reporter
- [ ] Add arguments to a benchmark
- [ ] Add statistics for repeated runs
  - [ ] trimean
  - [ ] standard deviation
  - [ ] min
  - [ ] max
- [ ] JSON reporter
- [ ] Benchmark registration
  - [ ] static
    - [ ] Auto-generated main function
  - [x] function pointer
  - [ ] lambda function