#include <cpptrace/cpptrace.hpp>
#include <cpptrace/from_current.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static cpptrace::stacktrace trace_anchor() {
    return cpptrace::generate_trace();
}

int main() {
    const auto trace = trace_anchor();
    if(trace.empty()) {
        std::cerr << "generate_trace returned no frames\n";
        return EXIT_FAILURE;
    }
    const bool symbolized = std::any_of(trace.begin(), trace.end(), [](const auto& frame) {
        return !frame.symbol.empty() || !frame.filename.empty();
    });
    if(!symbolized) {
        std::cerr << "trace frames had no symbols or source filenames\n";
        return EXIT_FAILURE;
    }

    CPPTRACE_TRY {
        throw std::runtime_error("FreeBSD cpptrace exception smoke");
    } CPPTRACE_CATCH(...) {
        const auto& exception_trace = cpptrace::from_current_exception();
        if(exception_trace.empty()) {
            std::cerr << "from_current_exception returned no frames\n";
            return EXIT_FAILURE;
        }
    }
    std::cout << "cpptrace unwinding, symbolization, and current-exception smoke passed\n";
}
