#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "utils/allocator.h"
#include "utils/arena.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>


static void BM_ParseJson(benchmark::State& state) {
    using namespace Utils;
    Utils::Arena arena{ 64_GB };

    for (auto _ : state) {

        state.PauseTiming();
        int fd = ::open( "graph.json", O_RDONLY );
        Utils::Allocator alloc{ arena };
        Utils::Job job{ Utils::OwnedFd( fd ), Utils::BorrowedFd( fd ) };
        state.ResumeTiming();

        Json::Lexer lexer{ job };
        Json::Parser parser{ lexer, alloc };
        auto doc = parser.parse();

        // Prevent the compiler from optimizing the result away.
        benchmark::DoNotOptimize(doc);
    }
}

BENCHMARK(BM_ParseJson);

BENCHMARK_MAIN();


