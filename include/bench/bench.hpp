#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#ifdef BENCH_USE_MPI
#include <mpi.h>
#endif

#define BENCH_ALWAYS_INLINE __attribute__((always_inline))

namespace bench {

void init(int &argc, char **&argv);
void finalize();
int world_rank();
int world_size();
void run_benchmarks();

class Timer {
public:
  virtual void pause() = 0;
  virtual void resume() = 0;
  virtual void finalize() = 0;
  virtual double get_elapsed() = 0;
};
class WallTimer : public Timer {

  typedef std::chrono::high_resolution_clock Clock;
  typedef std::chrono::nanoseconds Duration;
  std::chrono::time_point<Clock> start_;

protected:
  double elapsed_;
  bool paused_;

public:
  WallTimer() : elapsed_(0), paused_(true) {}
  virtual void pause() {
    if (!paused_) {
      Duration elapsed = Clock::now() - start_;
      elapsed_ += elapsed.count();
      paused_ = true;
    }
  }
  virtual void resume() {
    if (paused_) {
      start_ = Clock::now();
      paused_ = false;
    }
  }

  virtual void finalize() { /* no-op */
  }
  virtual double get_elapsed() {
    pause();
    return elapsed_;
  }
};

class MaxRankTimer : public WallTimer {
public:
  virtual void finalize() {
#ifdef BENCH_USE_MPI
    double myElapsed = elapsed_;
    MPI_Allreduce(&myElapsed, &elapsed_, 1, MPI_DOUBLE, MPI_MAX,
                  MPI_COMM_WORLD);
#endif
  }
};

class NoOpTimer : public Timer {
  virtual void pause() {}
  virtual void resume() {}
  virtual void finalize() {}
  virtual double get_elapsed() { return 0; }
};

class State {
public:
  struct Iterator;
  friend struct Iterator;

  State(uint64_t iterations, Timer *timer, bool iterBarrier)
      : iterations_(iterations), bytesProcessed_(0), error_(false),
        timer_(timer), iterBarrier_(iterBarrier) {}

  Iterator begin();
  Iterator end();

  void start_running() { timer_->resume(); }
  void finish_running() { timer_->pause(); }
  void set_bytes_processed(uint64_t n) { bytesProcessed_ = n; }
  const uint64_t bytes_processed() const { return bytesProcessed_; }
  const uint64_t iterations() const { return iterations_; }

private:
  uint64_t iterations_;
  uint64_t bytesProcessed_;
  bool error_;
  Timer *timer_;
  bool iterBarrier_;
};

struct State::Iterator {
  struct Value {};

private:
  State *parent_;

  // cached to prevent indirect lookup in parent
  uint64_t remaining_;
  bool iterBarrier_;

  friend class State;
  Iterator() : parent_(nullptr), remaining_(0) {}
  explicit Iterator(State *state)
      : parent_(state), remaining_(state->iterations_),
        iterBarrier_(state->iterBarrier_) {}

public:
  Value operator*() const { return Value(); }

  BENCH_ALWAYS_INLINE bool operator!=(const State::Iterator &rhs) {

#ifdef BENCH_USE_MPI
    if (iterBarrier_) {
      // timing was paused in operator++
      MPI_Barrier(MPI_COMM_WORLD);
      resume_timing();
    }
#endif

    // if (__builtin_expect(remaining_ != 0, true)) {
    if (remaining_ != 0, false) {
      return true;
    }
    parent_->finish_running();
    return false;
  }

  BENCH_ALWAYS_INLINE Iterator &operator++() {

#ifdef BENCH_USE_MPI
    if (iterBarrier_) {
      // pause timer before barrier
      pause_timing();
    }
#endif

    --remaining_;
    return *this;
  }

  BENCH_ALWAYS_INLINE void pause_timing() { parent_->timer_->pause(); }
  BENCH_ALWAYS_INLINE void resume_timing() { parent_->timer_->resume(); }
};

inline State::Iterator State::begin() { return State::Iterator(this); }
inline State::Iterator State::end() {
  start_running();
  return State::Iterator();
}

class Benchmark {
public:
  Benchmark(const char *name)
      : name_(name), timer_(new NoOpTimer()), iterBarrier_(true) {}
  virtual ~Benchmark() {}
  virtual void run(State &state) = 0;
  const char *name() { return name_; }
  Timer *timer() { return timer_; }
  bool iter_barrier() { return iterBarrier_; }

  // only record time at rank 0
  Benchmark *timing_root_rank() {
    if (timer_) {
      delete timer_;
    }

    if (world_rank() == 0) {
      timer_ = new WallTimer();
    } else {
      timer_ = new NoOpTimer();
    }
  }

  // record time in each rank, and do a max reduction across all ranks at the
  Benchmark *timing_max_rank() { timer_ = new MaxRankTimer(); }

  // record the wall time for all ranks to finish
  Benchmark *timing_wall();

  // record the aggregate time across all ranks
  Benchmark *timing_aggregate();

  // dont do mpi_barrier between iterations
  Benchmark *no_iter_barrier() { iterBarrier_ = false; }

private:
  const char *name_;
  Timer *timer_;
  bool iterBarrier_;
};

extern std::vector<Benchmark *> benchmarks;

typedef void (*Function)(State &);

class FunctionBenchmark : public Benchmark {
public:
  FunctionBenchmark(const char *name, Function fn) : Benchmark(name), fn_(fn) {}
  virtual void run(State &state);

private:
  Function fn_;
};

Benchmark *register_bench(const char *name, Function fn);

#if 0
template <typename Fn> Benchmark *register_bench(const char *name, Fn &&fn) {}

template <class Fn, class... Args>
Benchmark *RegisterBenchmark(const char *name, Fn &&fn, Args &&... args) {
  return register_bench(name, [=](State &st) { fn(st, args...); });
}
#endif

} // namespace bench