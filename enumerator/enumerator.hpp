#ifndef ENUMERATOR_ENUMERATOR_H
#define ENUMERATOR_ENUMERATOR_H

#include <chrono>
#include <cstdio>
#include <memory>

#include "absl/memory/memory.h"
#include "enumerable/enumerable.hpp"

template <typename Node, typename Item>
class Enumerator {
 public:
  Enumerator() { start_time_ = std::chrono::high_resolution_clock::now(); }

  // To be called when reading input is done.
  void ReadDone() {
    read_done_time_ = std::chrono::high_resolution_clock::now();
    ReadDoneInternal();
  }

  // Call this to start the enumeration.
  void Run(Enumerable<Node, Item>* system) {
    run_start_time_ = std::chrono::high_resolution_clock::now();
    RunInternal(system);
    run_done_time_ = std::chrono::high_resolution_clock::now();
  }

  // Call this when the enumeration is done to print statistics.
  void PrintStats(FILE* out = stderr) {
    fprintf(out, "Reading time: %ld ms\n",
            std::chrono::duration_cast<std::chrono::microseconds>(
                read_done_time_ - start_time_)
                .count());
    fprintf(out, "Setup time: %ld ms\n",
            std::chrono::duration_cast<std::chrono::microseconds>(
                run_start_time_ - read_done_time_)
                .count());
    fprintf(out, "Run time: %ld ms\n",
            std::chrono::duration_cast<std::chrono::microseconds>(
                run_done_time_ - run_start_time_)
                .count());
    fprintf(out, "Solutions found: %lu\n", solutions_found_);
    fprintf(out, "Solutions per ms: %f\n",
            (float)solutions_found_ /
                std::chrono::duration_cast<std::chrono::microseconds>(
                    run_done_time_ - run_start_time_)
                    .count());
    PrintStatsInternal();
  }

  // Creates an enumerable system and sets it.
  template <class Enumerable, typename... Args>
  void MakeEnumerableSystemAndRun(Args&&... args) {
    Run(absl::make_unique<Enumerable>(args...).get());
  }

  // Sets a function to be called whenever a solution is found. If the function
  // returns false, enumeration is stopped.
  void SetItemFoundCallback(const std::function<bool(const Item&)>& cb) {
    cb_ = cb;
  }

 protected:
  virtual void RunInternal(Enumerable<Node, Item>* system) = 0;
  virtual void ReadDoneInternal() {}
  virtual void PrintStatsInternal() {}
  virtual bool ReportSolution(Enumerable<Node, Item>* system,
                              const Node& node) {
    solutions_found_++;
    if (cb_) {
      return cb_(system->NodeToItem(node));
    }
    return true;
  }

  std::function<bool(const Item&)> cb_{nullptr};
  std::chrono::high_resolution_clock::time_point start_time_;
  std::chrono::high_resolution_clock::time_point read_done_time_;
  std::chrono::high_resolution_clock::time_point run_start_time_;
  std::chrono::high_resolution_clock::time_point run_done_time_;
  std::size_t solutions_found_{0};
};

#endif  // ENUMERATOR_ENUMERATOR_H
