# Jank Project Instructions

## Project Overview

**Jank** is a native dialect of Clojure hosted on LLVM, implemented in C++20. It provides both Just-In-Time (JIT) and Ahead-Of-Time (AOT) compilation.

-   **Core Language**: Implemented in `src/jank/clojure/core.jank`.
-   **Compiler**: Implemented in C++ (`src/cpp/jank`), using Clang for C++ parsing and LLVM for code generation.
-   **Runtime**: A C++ runtime that supports dynamic typing, garbage collection (Boehm GC), and persistent data structures.


## Repository Structure & Working Directory

*   **Git Root**: The repository root is one level above `compiler+runtime`.
*   **Working Directory**: All build and test commands **MUST** be run from `compiler+runtime`.
*   **Git Commands**: When running git commands from `compiler+runtime`, paths may be returned relative to the git root (e.g., `compiler+runtime/src/...`).
    *   **Agent Tip**: Always verify file paths. If a path starts with `compiler+runtime/`, strip it to access the file relative to the current working directory.

## Build System

The project uses CMake with a two-phase build process to bootstrap the compiler.

### Standard Build Commands

*   **Configure**: `./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug -Djank_test=on -Djank_local_clang=on`
    *   **WARNING**: This script wipes `build/CMakeCache.txt` and `build/CMakeFiles` before running `cmake`.
    *   Use this to start fresh or change compilation flags.
    *   *Note*: If using the custom LLVM toolchain, ensure `CC` and `CXX` are set correctly before running this, or rely on `bin/build` to handle it if the cache allows.
*   **Build**: `./bin/build`
    *   Builds the project (generated artifacts go to `build/`).
    *   Automatically sets `CC` and `CXX` to the custom LLVM toolchain for the build step.
*   **Clean**: `./bin/clean`
    *   Runs `ninja clean` (removes compiled objects/executables).
    *   **Safe** to run; preserves `build/llvm-install`.
*   **Deep Clean**:
    *   Run `./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug -Djank_test=on -Djank_local_clang=on` to wipe the CMake cache and reconfigure.

### CMake Options (passed to `bin/configure`)

*   `jank_test=ON`: Enable tests (e.g., `./bin/configure -Djank_test=ON`).
*   `CMAKE_BUILD_TYPE`: `Debug` or `Release`.

### Custom LLVM Toolchain

The project relies on a custom build of LLVM/Clang located in `build/llvm-install`.
*   The `bin/build` script automatically sets `CC` and `CXX` to use this custom toolchain.
*   **Do not** delete `build/llvm-install` unless you intend to rebuild LLVM from scratch (which takes a long time).

### CMake Options

*   `jank_test=ON`: Enable the test suite (`ON` by default in `bin/build` script logic often, but explicit checks help).
*   `CMAKE_BUILD_TYPE`: `Debug`, `Release`, or `RelWithDebInfo`.

## Testing

Ensure all tests pass for **both** codegen backends (C++ and LLVM IR) before submitting changes.

### Running Tests

1.  **Run All Tests (LLVM IR)**:
    ```bash
    uv run bin/run_all_jank_tests.py --codegen llvm-ir
    ```
2.  **Run All Tests (C++)**:
    ```bash
    uv run bin/run_all_jank_tests.py --codegen cpp
    ```
3.  **Run Individual Test**:
    ```bash
    build/jank run --codegen llvm-ir path/to/file.jank
    # OR
    build/jank run --codegen cpp path/to/file.jank
    ```
4.  **Run Final Integration Tests**:
    ```bash
    PATH=$PWD/build:$PATH bb test/bash/error-reporting/bin/jank/test/error_reporting.clj test
    ```
    *Note: Requires `bb` (babashka) installed.*

### Troubleshooting
*   **Bin: OUTDATED**: If `bin/run_all_jank_tests.py` reports `Bin: OUTDATED`, it means the binary was compiled with older code. You must stop the test runner (Ctrl+C), run `./bin/build` to rebuild, and then restart the tests.

## Debugging

### Compiler Debugging
*   **Print IR**: Set `JANK_PRINT_IR=1` to dump the generated code to stdout.
    ```bash
    JANK_PRINT_IR=1 build/jank run --codegen llvm-ir path/to/test.jank
    ```
*   **Clean IR Output**: Pipe through `bin/parse_ir.py` to remove noise (module layout boilerplate, etc).
    ```bash
    JANK_PRINT_IR=1 build/jank run --codegen llvm-ir path/to/test.jank | uv run bin/parse_ir.py
    ```
*   **Check Health**: Verify environment and JIT capabilities.
    ```bash
    build/jank check-health
    ```

### Common Issues
*   **Segfaults**: Often due to issues in `jank_load_...` functions or unchecked usages of `nullptr` in codegen.
*   **Linker Errors**: Check `extern "C"` usage on module entry points.
*   **Missing Symbols**: Ensure standard headers are included in `include/cpp/jank/prelude.hpp` for the JIT PCH.
*   **LLVM IR Type Mismatches**: Segfaults (exit code -11) frequently occur if the analysis phase marks a type as a reference (e.g., `static const int` as `int const&`) while the LLVM backend expects a value. This causes `load` instructions on raw values. Always verify `src/cpp/jank/analyze/processor.cpp` treats static members as values to align with `MakeAotCallable`.

### Advanced Debugging
*   **Single File Analysis**: Use `uv run bin/run_all_jank_tests.py --analyze path/to/file.jank` for rapid iteration.
*   **Verification**: Check `check-health` and run the `pass-static.jank` test to confirm LLVM backend stability.

## Architecture

### Directory Structure (`src/cpp/jank/`)
*   **`read/`**: Lexer (`lex.cpp`) and Parser (`parse.cpp`). Converts source to AST.
*   **`analyze/`**: Semantic analysis phase.
    *   `processor.cpp`: Main driver.
    *   `expr/`: AST node definitions (`def`, `let`, `if`, etc.).
    *   Converts parsed forms into typed expressions with unique names.
*   **`codegen/`**: Code generation.
    *   `processor.cpp`: **C++ Backend**. Generates C++ structs and source code.
    *   `llvm_processor.cpp`: **LLVM IR Backend**. Generates LLVM IR instructions directly.
*   **`runtime/`**: Runtime objects and functions.
    *   `obj/`: Usage of `jank_object_ref` (boxed values).
    *   `context.cpp`: Thread-local runtime context.

### Dual Backends
1.  **C++ Backend**:
    *   Generates C++ source.
    *   Each function is a `struct` inheriting from `jit_function`.
    *   Must handle "lifting" of variables and constants to member variables.
    *   Uses `jtl::string_builder`.
2.  **LLVM IR Backend**:
    *   Generates IR via `llvm::IRBuilder`.
    *   Maps locals to `llvm::Value*`.
    *   Must manually handle boxing/unboxing and type conversions (e.g., `i32` -> `double`).

## C++ Interop

Jank is designed for seamless C++ interop.

*   **Boxing/Unboxing**:
    *   `ctx->box_value(val)`: Wraps a C++ value in a `jank_object_ref`.
    *   `ctx->unbox_value(obj)`: Extracts the raw C++ value.
    *   **NOTE**: `operator&` in Jank (`cpp/&`) returns a raw pointer.
*   **Special Forms**:
    *   `cpp/raw`: Inject raw C++ strings.
    *   `try`/`catch`: Supports catching C++ exceptions by type (e.g., `catch cpp/std.runtime_error e`).
*   **Types**:
    *   `cpp/nullptr` maps to `std::nullptr_t`.
    *   `cpp/void` maps to `void`.

## Coding Conventions

*   **Style**: Follows `.clang-format`.
    *   **Comments**: STRICTLY use `/* ... */` block style for all comments in C++ files. Do NOT use `//` (single-line) style comments.
*   **C++ Standard**: C++20.
*   **Safety**:
    *   Avoid raw `new`/`delete`. Use `gc_new` (Boehm GC) where appropriate.
    *   Use `jank::error` for reporting compilation errors.

## Useful Scripts & Snippets

### Smart Formatting (Format Only Changed Files)
To format only the files you have changed (staged or unstaged) relative to the current directory:
```bash
git diff --name-only --relative --diff-filter=d HEAD -- '*.[hc]pp' '*.[hc]' '*.cc' '*.hh' '*.cxx' '*.hxx' 2>/dev/null | xargs clang-format -i
```
You can save this as `bin/format-dirty`.

## Workflow Tips
*   Always check the **Knowledge Items** (KIs) provided in the conversation for deep dives into specific subsystems like "Exceptions" or "Modules".
*   When fixing a bug, try to reproduce it with a minimal `.jank` file and run it with both codegen backends.

## Testing Conventions

### Test Suite Organization
*   **Location**:
    *   `test/jank/cpp/`: Tests focusing on C++ interop, types, and raw C++ behavior (e.g., catching specific C++ types, slicing, destructors).
    *   **Convention**: Use `jank::cpp::try_::<filename_sanitized>` for C++ `cpp/raw` blocks in these tests to avoid namespace collisions.
    *   `test/jank/form/`: Tests focusing on Jank spec forms and semantics (e.g., `try`, `if`, `loop`).
*   **Avoid Redundancy**:
    *   Do not create separate files for every variation of a test (e.g., one file per primitive type).
    *   Group related tests into a single file (e.g., `pass-catch-cpp-primitives.jank`) to reduce file clutter and build time overhead.
*   **Coverage**:
    *   **Exceptions**: When testing exception handling, ensure coverage for:
        *   Re-throwing exceptions (`throw;` in C++).
        *   Stack unwinding and destructor calls (RAII).
        *   Interaction between multiple catch clauses.

### Best Practices
*   **Avoid `println`**: Do not use `println` or other side-effect printing in tests. Use atoms or return values to verify state.
*   **Exception Messages**: Use `(cpp/.what e)` to assert exception messages.
*   **Pointer Testing**: When testing `void*` or opaque pointers:
    *   Define a global `void*` constant in a C++ block.
    *   Use `(cpp/== p global_ptr)` to verify the value.
    *   *Note*: Direct usage of caught `void*` variables in LLVM IR may currently segfault (see Known Issues).

## Known Issues

*   **LLVM IR `void*` Catch**: In `pass-catch-cpp-pointer.jank`, accessing a caught `void*` variable in the LLVM IR backend leads to a segfault, even though generic `void*` argument passing works.
*   **LLVM IR Type Mismatches**: Segfaults (exit code -11) frequently occur if the analysis phase marks a type as a reference (e.g., `static const int` as `int const&`) while the LLVM backend expects a value. This causes `load` instructions on raw values. Always verify `src/cpp/jank/analyze/processor.cpp` treats static members as values to align with `MakeAotCallable`.
