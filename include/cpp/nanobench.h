//  __   _ _______ __   _  _____  ______  _______ __   _ _______ _     _
//  | \  | |_____| | \  | |     | |_____] |______ | \  | |       |_____|
//  |  \_| |     | |  \_| |_____| |_____] |______ |  \_| |_____  |     |
//
// Microbenchmark framework for C++11/14/17/20
// https://github.com/martinus/nanobench
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2022 Martin Ankerl <martin.ankerl@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ANKERL_NANOBENCH_H_INCLUDED
#define ANKERL_NANOBENCH_H_INCLUDED

// see https://semver.org/
#define ANKERL_NANOBENCH_VERSION_MAJOR 4 // incompatible API changes
#define ANKERL_NANOBENCH_VERSION_MINOR 3 // backwards-compatible changes
#define ANKERL_NANOBENCH_VERSION_PATCH 7 // backwards-compatible bug fixes

///////////////////////////////////////////////////////////////////////////////////////////////////
// public facing api - as minimal as possible
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <chrono>  // high_resolution_clock
#include <cstring> // memcpy
#include <iosfwd>  // for std::ostream* custom output target in Config
#include <string>  // all names
#include <vector>  // holds all results

#define ANKERL_NANOBENCH(x) ANKERL_NANOBENCH_PRIVATE_##x()

#define ANKERL_NANOBENCH_PRIVATE_CXX() __cplusplus
#define ANKERL_NANOBENCH_PRIVATE_CXX98() 199711L
#define ANKERL_NANOBENCH_PRIVATE_CXX11() 201103L
#define ANKERL_NANOBENCH_PRIVATE_CXX14() 201402L
#define ANKERL_NANOBENCH_PRIVATE_CXX17() 201703L

#if ANKERL_NANOBENCH(CXX) >= ANKERL_NANOBENCH(CXX17)
#    define ANKERL_NANOBENCH_PRIVATE_NODISCARD() [[nodiscard]]
#else
#    define ANKERL_NANOBENCH_PRIVATE_NODISCARD()
#endif

#if defined(__clang__)
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_PADDED_PUSH() \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wpadded\"")
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_PADDED_POP() _Pragma("clang diagnostic pop")
#else
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_PADDED_PUSH()
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_PADDED_POP()
#endif

#if defined(__GNUC__)
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_EFFCPP_PUSH() _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Weffc++\"")
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_EFFCPP_POP() _Pragma("GCC diagnostic pop")
#else
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_EFFCPP_PUSH()
#    define ANKERL_NANOBENCH_PRIVATE_IGNORE_EFFCPP_POP()
#endif

#if defined(ANKERL_NANOBENCH_LOG_ENABLED)
#    include <iostream>
#    define ANKERL_NANOBENCH_LOG(x)                                                 \
        do {                                                                        \
            std::cout << __FUNCTION__ << "@" << __LINE__ << ": " << x << std::endl; \
        } while (0)
#else
#    define ANKERL_NANOBENCH_LOG(x) \
        do {                        \
        } while (0)
#endif

#define ANKERL_NANOBENCH_PRIVATE_PERF_COUNTERS() 0
#if defined(__linux__) && !defined(ANKERL_NANOBENCH_DISABLE_PERF_COUNTERS)
#    include <linux/version.h>
#    if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
// PERF_COUNT_HW_REF_CPU_CYCLES only available since kernel 3.3
// PERF_FLAG_FD_CLOEXEC since kernel 3.14
#        undef ANKERL_NANOBENCH_PRIVATE_PERF_COUNTERS
#        define ANKERL_NANOBENCH_PRIVATE_PERF_COUNTERS() 1
#    endif
#endif

#if defined(__clang__)
#    define ANKERL_NANOBENCH_NO_SANITIZE(...) __attribute__((no_sanitize(__VA_ARGS__)))
#else
#    define ANKERL_NANOBENCH_NO_SANITIZE(...)
#endif

#if defined(_MSC_VER)
#    define ANKERL_NANOBENCH_PRIVATE_NOINLINE() __declspec(noinline)
#else
#    define ANKERL_NANOBENCH_PRIVATE_NOINLINE() __attribute__((noinline))
#endif

// workaround missing "is_trivially_copyable" in g++ < 5.0
// See https://stackoverflow.com/a/31798726/48181
#if defined(__GNUC__) && __GNUC__ < 5
#    define ANKERL_NANOBENCH_IS_TRIVIALLY_COPYABLE(...) __has_trivial_copy(__VA_ARGS__)
#else
#    define ANKERL_NANOBENCH_IS_TRIVIALLY_COPYABLE(...) std::is_trivially_copyable<__VA_ARGS__>::value
#endif

// declarations ///////////////////////////////////////////////////////////////////////////////////

namespace ankerl {
namespace nanobench {

using Clock = std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock,
                               std::chrono::steady_clock>::type;
class Bench;
struct Config;
class Result;
class Rng;
class BigO;

/**
 * @brief Renders output from a mustache-like template and benchmark results.
 *
 * The templating facility here is heavily inspired by [mustache - logic-less templates](https://mustache.github.io/).
 * It adds a few more features that are necessary to get all of the captured data out of nanobench. Please read the
 * excellent [mustache manual](https://mustache.github.io/mustache.5.html) to see what this is all about.
 *
 * nanobench output has two nested layers, *result* and *measurement*.  Here is a hierarchy of the allowed tags:
 *
 * * `{{#result}}` Marks the begin of the result layer. Whatever comes after this will be instantiated as often as
 *   a benchmark result is available. Within it, you can use these tags:
 *
 *    * `{{title}}` See Bench::title().
 *
 *    * `{{name}}` Benchmark name, usually directly provided with Bench::run(), but can also be set with Bench::name().
 *
 *    * `{{unit}}` Unit, e.g. `byte`. Defaults to `op`, see Bench::title().
 *
 *    * `{{batch}}` Batch size, see Bench::batch().
 *
 *    * `{{complexityN}}` Value used for asymptotic complexity calculation. See Bench::complexityN().
 *
 *    * `{{epochs}}` Number of epochs, see Bench::epochs().
 *
 *    * `{{clockResolution}}` Accuracy of the clock, i.e. what's the smallest time possible to measure with the clock.
 *      For modern systems, this can be around 20 ns. This value is automatically determined by nanobench at the first
 *      benchmark that is run, and used as a static variable throughout the application's runtime.
 *
 *    * `{{clockResolutionMultiple}}` Configuration multiplier for `clockResolution`. See Bench::clockResolutionMultiple().
 *      This is the target runtime for each measurement (epoch). That means the more accurate your clock is, the faster
 *      will be the benchmark. Basing the measurement's runtime on the clock resolution is the main reason why nanobench is so fast.
 *
 *    * `{{maxEpochTime}}` Configuration for a maximum time each measurement (epoch) is allowed to take. Note that at least
 *      a single iteration will be performed, even when that takes longer than maxEpochTime. See Bench::maxEpochTime().
 *
 *    * `{{minEpochTime}}` Minimum epoch time, defaults to 1ms. See Bench::minEpochTime().
 *
 *    * `{{minEpochIterations}}` See Bench::minEpochIterations().
 *
 *    * `{{epochIterations}}` See Bench::epochIterations().
 *
 *    * `{{warmup}}` Number of iterations used before measuring starts. See Bench::warmup().
 *
 *    * `{{relative}}` True or false, depending on the setting you have used. See Bench::relative().
 *
 *    Apart from these tags, it is also possible to use some mathematical operations on the measurement data. The operations
 *    are of the form `{{command(name)}}`.  Currently `name` can be one of `elapsed`, `iterations`. If performance counters
 *    are available (currently only on current Linux systems), you also have `pagefaults`, `cpucycles`,
 *    `contextswitches`, `instructions`, `branchinstructions`, and `branchmisses`. All the measuers (except `iterations`) are
 *    provided for a single iteration (so `elapsed` is the time a single iteration took). The following tags are available:
 *
 *    * `{{median(<name>)}}` Calculate median of a measurement data set, e.g. `{{median(elapsed)}}`.
 *
 *    * `{{average(<name>)}}` Average (mean) calculation.
 *
 *    * `{{medianAbsolutePercentError(<name>)}}` Calculates MdAPE, the Median Absolute Percentage Error. The MdAPE is an excellent
 *      metric for the variation of measurements. It is more robust to outliers than the
 *      [Mean absolute percentage error (M-APE)](https://en.wikipedia.org/wiki/Mean_absolute_percentage_error).
 *      @f[
 *       \mathrm{MdAPE}(e) = \mathrm{med}\{| \frac{e_i - \mathrm{med}\{e\}}{e_i}| \}
 *      @f]
 *      E.g. for *elapsed*: First, @f$ \mathrm{med}\{e\} @f$ calculates the median by sorting and then taking the middle element
 *      of all *elapsed* measurements. This is used to calculate the absolute percentage
 *      error to this median for each measurement, as in  @f$ | \frac{e_i - \mathrm{med}\{e\}}{e_i}| @f$. All these results
 *      are sorted, and the middle value is chosen as the median absolute percent error.
 *
 *      This measurement is a bit hard to interpret, but it is very robust against outliers. E.g. a value of 5% means that half of the
 *      measurements deviate less than 5% from the median, and the other deviate more than 5% from the median.
 *
 *    * `{{sum(<name>)}}` Sums of all the measurements. E.g. `{{sum(iterations)}}` will give you the total number of iterations
*        measured in this benchmark.
 *
 *    * `{{minimum(<name>)}}` Minimum of all measurements.
 *
 *    * `{{maximum(<name>)}}` Maximum of all measurements.
 *
 *    * `{{sumProduct(<first>, <second>)}}` Calculates the sum of the products of corresponding measures:
 *      @f[
 *          \mathrm{sumProduct}(a,b) = \sum_{i=1}^{n}a_i\cdot b_i
 *      @f]
 *      E.g. to calculate total runtime of the benchmark, you multiply iterations with elapsed time for each measurement, and
 *      sum these results up:
 *      `{{sumProduct(iterations, elapsed)}}`.
 *
 *    * `{{#measurement}}` To access individual measurement results, open the begin tag for measurements.
 *
 *       * `{{elapsed}}` Average elapsed wall clock time per iteration, in seconds.
 *
 *       * `{{iterations}}` Number of iterations in the measurement. The number of iterations will fluctuate due
 *         to some applied randomness, to enhance accuracy.
 *
 *       * `{{pagefaults}}` Average number of pagefaults per iteration.
 *
 *       * `{{cpucycles}}` Average number of CPU cycles processed per iteration.
 *
 *       * `{{contextswitches}}` Average number of context switches per iteration.
 *
 *       * `{{instructions}}` Average number of retired instructions per iteration.
 *
 *       * `{{branchinstructions}}` Average number of branches executed per iteration.
 *
 *       * `{{branchmisses}}` Average number of branches that were missed per iteration.
 *
 *    * `{{/measurement}}` Ends the measurement tag.
 *
 * * `{{/result}}` Marks the end of the result layer. This is the end marker for the template part that will be instantiated
 *   for each benchmark result.
 *
 *
 *  For the layer tags *result* and *measurement* you additionally can use these special markers:
 *
 *  * ``{{#-first}}`` - Begin marker of a template that will be instantiated *only for the first* entry in the layer. Use is only
 *    allowed between the begin and end marker of the layer allowed. So between ``{{#result}}`` and ``{{/result}}``, or between
 *    ``{{#measurement}}`` and ``{{/measurement}}``. Finish the template with ``{{/-first}}``.
 *
 *  * ``{{^-first}}`` - Begin marker of a template that will be instantiated *for each except the first* entry in the layer. This,
 *    this is basically the inversion of ``{{#-first}}``. Use is only allowed between the begin and end marker of the layer allowed.
 *    So between ``{{#result}}`` and ``{{/result}}``, or between ``{{#measurement}}`` and ``{{/measurement}}``.
 *
 *  * ``{{/-first}}`` - End marker for either ``{{#-first}}`` or ``{{^-first}}``.
 *
 *  * ``{{#-last}}`` - Begin marker of a template that will be instantiated *only for the last* entry in the layer. Use is only
 *    allowed between the begin and end marker of the layer allowed. So between ``{{#result}}`` and ``{{/result}}``, or between
 *    ``{{#measurement}}`` and ``{{/measurement}}``. Finish the template with ``{{/-last}}``.
 *
 *  * ``{{^-last}}`` - Begin marker of a template that will be instantiated *for each except the last* entry in the layer. This,
 *    this is basically the inversion of ``{{#-last}}``. Use is only allowed between the begin and end marker of the layer allowed.
 *    So between ``{{#result}}`` and ``{{/result}}``, or between ``{{#measurement}}`` and ``{{/measurement}}``.
 *
 *  * ``{{/-last}}`` - End marker for either ``{{#-last}}`` or ``{{^-last}}``.
 *
   @verbatim embed:rst

   For an overview of all the possible data you can get out of nanobench, please see the tutorial at :ref:`tutorial-template-json`.

   The templates that ship with nanobench are:

   * :cpp:func:`templates::csv() <ankerl::nanobench::templates::csv()>`
   * :cpp:func:`templates::json() <ankerl::nanobench::templates::json()>`
   * :cpp:func:`templates::htmlBoxplot() <ankerl::nanobench::templates::htmlBoxplot()>`
   * :cpp:func:`templates::pyperf() <ankerl::nanobench::templates::pyperf()>`

   @endverbatim
 *
 * @param mustacheTemplate The template.
 * @param bench Benchmark, containing all the results.
 * @param out Output for the generated output.
 */
void render(char const* mustacheTemplate, Bench const& bench, std::ostream& out);
void render(std::string const& mustacheTemplate, Bench const& bench, std::ostream& out);

/**
 * Same as render(char const* mustacheTemplate, Bench const& bench, std::ostream& out), but for when
 * you only have results available.
 *
 * @param mustacheTemplate The template.
 * @param results All the results to be used for rendering.
 * @param out Output for the generated output.
 */
void render(char const* mustacheTemplate, std::vector<Result> const& results, std::ostream& out);
void render(std::string const& mustacheTemplate, std::vector<Result> const& results, std::ostream& out);

// Contains mustache-like templates
namespace templates {

/*!
  @brief CSV data for the benchmark results.

  Generates a comma-separated values dataset. First line is the header, each following line is a summary of each benchmark run.

  @verbatim embed:rst
  See the tutorial at :ref:`tutorial-template-csv` for an example.
  @endverbatim
 */
char const* csv() noexcept;

/*!
  @brief HTML output that uses plotly to generate an interactive boxplot chart. See the tutorial for an example output.

  The output uses only the elapsed wall clock time, and displays each epoch as a single dot.
  @verbatim embed:rst
  See the tutorial at :ref:`tutorial-template-html` for an example.
  @endverbatim

  @see ankerl::nanobench::render()
 */
char const* htmlBoxplot() noexcept;

/*!
 @brief Output in pyperf compatible JSON format, which can be used for more analyzation.
 @verbatim embed:rst
 See the tutorial at :ref:`tutorial-template-pyperf` for an example how to further analyze the output.
 @endverbatim
 */
char const* pyperf() noexcept;

/*!
  @brief Template to generate JSON data.

  The generated JSON data contains *all* data that has been generated. All times are as double values, in seconds. The output can get
  quite large.
  @verbatim embed:rst
  See the tutorial at :ref:`tutorial-template-json` for an example.
  @endverbatim
 */
char const* json() noexcept;

} // namespace templates

namespace detail {

template <typename T>
struct PerfCountSet;

class IterationLogic;
class PerformanceCounters;

#if ANKERL_NANOBENCH(PERF_COUNTERS)
class LinuxPerformanceCounters;
#endif

} // namespace detail
} // namespace nanobench
} // namespace ankerl

// definitions ////////////////////////////////////////////////////////////////////////////////////

namespace ankerl {
namespace nanobench {
namespace detail {

template <typename T>
struct PerfCountSet {
    T pageFaults{};
    T cpuCycles{};
    T contextSwitches{};
    T instructions{};
    T branchInstructions{};
    T branchMisses{};
};

} // namespace detail

ANKERL_NANOBENCH(IGNORE_PADDED_PUSH)
struct Config {
    // actual benchmark config
    std::string mBenchmarkTitle = "benchmark";
    std::string mBenchmarkName = "noname";
    std::string mUnit = "op";
    double mBatch = 1.0;
    double mComplexityN = -1.0;
    size_t mNumEpochs = 11;
    size_t mClockResolutionMultiple = static_cast<size_t>(1000);
    std::chrono::nanoseconds mMaxEpochTime = std::chrono::milliseconds(100);
    std::chrono::nanoseconds mMinEpochTime = std::chrono::milliseconds(1);
    uint64_t mMinEpochIterations{1};
    uint64_t mEpochIterations{0}; // If not 0, run *exactly* these number of iterations per epoch.
    uint64_t mWarmup = 0;
    std::ostream* mOut = nullptr;
    std::chrono::duration<double> mTimeUnit = std::chrono::nanoseconds{1};
    std::string mTimeUnitName = "ns";
    bool mShowPerformanceCounters = true;
    bool mIsRelative = false;

    Config();
    ~Config();
    Config& operator=(Config const&);
    Config& operator=(Config&&);
    Config(Config const&);
    Config(Config&&) noexcept;
};
ANKERL_NANOBENCH(IGNORE_PADDED_POP)

// Result returned after a benchmark has finished. Can be used as a baseline for relative().
ANKERL_NANOBENCH(IGNORE_PADDED_PUSH)
class Result {
public:
    enum class Measure : size_t {
        elapsed,
        iterations,
        pagefaults,
        cpucycles,
        contextswitches,
        instructions,
        branchinstructions,
        branchmisses,
        _size
    };

    explicit Result(Config const& benchmarkConfig);

    ~Result();
    Result& operator=(Result const&);
    Result& operator=(Result&&);
    Result(Result const&);
    Result(Result&&) noexcept;

    // adds new measurement results
    // all values are scaled by iters (except iters...)
    void add(Clock::duration totalElapsed, uint64_t iters, detail::PerformanceCounters const& pc);

    ANKERL_NANOBENCH(NODISCARD) Config const& config() const noexcept;

    ANKERL_NANOBENCH(NODISCARD) double median(Measure m) const;
    ANKERL_NANOBENCH(NODISCARD) double medianAbsolutePercentError(Measure m) const;
    ANKERL_NANOBENCH(NODISCARD) double average(Measure m) const;
    ANKERL_NANOBENCH(NODISCARD) double sum(Measure m) const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double sumProduct(Measure m1, Measure m2) const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double minimum(Measure m) const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double maximum(Measure m) const noexcept;

    ANKERL_NANOBENCH(NODISCARD) bool has(Measure m) const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double get(size_t idx, Measure m) const;
    ANKERL_NANOBENCH(NODISCARD) bool empty() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) size_t size() const noexcept;

    // Finds string, if not found, returns _size.
    static Measure fromString(std::string const& str);

private:
    Config mConfig{};
    std::vector<std::vector<double>> mNameToMeasurements{};
};
ANKERL_NANOBENCH(IGNORE_PADDED_POP)

/**
 * An extremely fast random generator. Currently, this implements *RomuDuoJr*, developed by Mark Overton. Source:
 * http://www.romu-random.org/
 *
 * RomuDuoJr is extremely fast and provides reasonable good randomness. Not enough for large jobs, but definitely
 * good enough for a benchmarking framework.
 *
 *  * Estimated capacity: @f$ 2^{51} @f$ bytes
 *  * Register pressure: 4
 *  * State size: 128 bits
 *
 * This random generator is a drop-in replacement for the generators supplied by ``<random>``. It is not
 * cryptographically secure. It's intended purpose is to be very fast so that benchmarks that make use
 * of randomness are not distorted too much by the random generator.
 *
 * Rng also provides a few non-standard helpers, optimized for speed.
 */
class Rng final {
public:
    /**
     * @brief This RNG provides 64bit randomness.
     */
    using result_type = uint64_t;

    static constexpr uint64_t(min)();
    static constexpr uint64_t(max)();

    /**
     * As a safety precausion, we don't allow copying. Copying a PRNG would mean you would have two random generators that produce the
     * same sequence, which is generally not what one wants. Instead create a new rng with the default constructor Rng(), which is
     * automatically seeded from `std::random_device`. If you really need a copy, use copy().
     */
    Rng(Rng const&) = delete;

    /**
     * Same as Rng(Rng const&), we don't allow assignment. If you need a new Rng create one with the default constructor Rng().
     */
    Rng& operator=(Rng const&) = delete;

    // moving is ok
    Rng(Rng&&) noexcept = default;
    Rng& operator=(Rng&&) noexcept = default;
    ~Rng() noexcept = default;

    /**
     * @brief Creates a new Random generator with random seed.
     *
     * Instead of a default seed (as the random generators from the STD), this properly seeds the random generator from
     * `std::random_device`. It guarantees correct seeding. Note that seeding can be relatively slow, depending on the source of
     * randomness used. So it is best to create a Rng once and use it for all your randomness purposes.
     */
    Rng();

    /*!
      Creates a new Rng that is seeded with a specific seed. Each Rng created from the same seed will produce the same randomness
      sequence. This can be useful for deterministic behavior.

      @verbatim embed:rst
      .. note::

         The random algorithm might change between nanobench releases. Whenever a faster and/or better random
         generator becomes available, I will switch the implementation.
      @endverbatim

      As per the Romu paper, this seeds the Rng with splitMix64 algorithm and performs 10 initial rounds for further mixing up of the
      internal state.

      @param seed  The 64bit seed. All values are allowed, even 0.
     */
    explicit Rng(uint64_t seed) noexcept;
    Rng(uint64_t x, uint64_t y) noexcept;
    Rng(std::vector<uint64_t> const& data);

    /**
     * Creates a copy of the Rng, thus the copy provides exactly the same random sequence as the original.
     */
    ANKERL_NANOBENCH(NODISCARD) Rng copy() const noexcept;

    /**
     * @brief Produces a 64bit random value. This should be very fast, thus it is marked as inline. In my benchmark, this is ~46 times
     * faster than `std::default_random_engine` for producing 64bit random values. It seems that the fastest std contender is
     * `std::mt19937_64`. Still, this RNG is 2-3 times as fast.
     *
     * @return uint64_t The next 64 bit random value.
     */
    inline uint64_t operator()() noexcept;

    // This is slightly biased. See

    /**
     * Generates a random number between 0 and range (excluding range).
     *
     * The algorithm only produces 32bit numbers, and is slightly biased. The effect is quite small unless your range is close to the
     * maximum value of an integer. It is possible to correct the bias with rejection sampling (see
     * [here](https://lemire.me/blog/2016/06/30/fast-random-shuffling/), but this is most likely irrelevant in practices for the
     * purposes of this Rng.
     *
     * See Daniel Lemire's blog post [A fast alternative to the modulo
     * reduction](https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/)
     *
     * @param range Upper exclusive range. E.g a value of 3 will generate random numbers 0, 1, 2.
     * @return uint32_t Generated random values in range [0, range(.
     */
    inline uint32_t bounded(uint32_t range) noexcept;

    // random double in range [0, 1(
    // see http://prng.di.unimi.it/

    /**
     * Provides a random uniform double value between 0 and 1. This uses the method described in [Generating uniform doubles in the
     * unit interval](http://prng.di.unimi.it/), and is extremely fast.
     *
     * @return double Uniformly distributed double value in range [0,1(, excluding 1.
     */
    inline double uniform01() noexcept;

    /**
     * Shuffles all entries in the given container. Although this has a slight bias due to the implementation of bounded(), this is
     * preferable to `std::shuffle` because it is over 5 times faster. See Daniel Lemire's blog post [Fast random
     * shuffling](https://lemire.me/blog/2016/06/30/fast-random-shuffling/).
     *
     * @param container The whole container will be shuffled.
     */
    template <typename Container>
    void shuffle(Container& container) noexcept;

    /**
     * Extracts the full state of the generator, e.g. for serialization. For this RNG this is just 2 values, but to stay API compatible
     * with future implementations that potentially use more state, we use a vector.
     *
     * @return Vector containing the full state:
     */
    std::vector<uint64_t> state() const;

private:
    static constexpr uint64_t rotl(uint64_t x, unsigned k) noexcept;

    uint64_t mX;
    uint64_t mY;
};

/**
 * @brief Main entry point to nanobench's benchmarking facility.
 *
 * It holds configuration and results from one or more benchmark runs. Usually it is used in a single line, where the object is
 * constructed, configured, and then a benchmark is run. E.g. like this:
 *
 *     ankerl::nanobench::Bench().unit("byte").batch(1000).run("random fluctuations", [&] {
 *         // here be the benchmark code
 *     });
 *
 * In that example Bench() constructs the benchmark, it is then configured with unit() and batch(), and after configuration a
 * benchmark is executed with run(). Once run() has finished, it prints the result to `std::cout`. It would also store the results
 * in the Bench instance, but in this case the object is immediately destroyed so it's not available any more.
 */
ANKERL_NANOBENCH(IGNORE_PADDED_PUSH)
class Bench {
public:
    /**
     * @brief Creates a new benchmark for configuration and running of benchmarks.
     */
    Bench();

    Bench(Bench&& other);
    Bench& operator=(Bench&& other);
    Bench(Bench const& other);
    Bench& operator=(Bench const& other);
    ~Bench() noexcept;

    /*!
      @brief Repeatedly calls `op()` based on the configuration, and performs measurements.

      This call is marked with `noinline` to prevent the compiler to optimize beyond different benchmarks. This can have quite a big
      effect on benchmark accuracy.

      @verbatim embed:rst
      .. note::

        Each call to your lambda must have a side effect that the compiler can't possibly optimize it away. E.g. add a result to an
        externally defined number (like `x` in the above example), and finally call `doNotOptimizeAway` on the variables the compiler
        must not remove. You can also use :cpp:func:`ankerl::nanobench::doNotOptimizeAway` directly in the lambda, but be aware that
        this has a small overhead.

      @endverbatim

      @tparam Op The code to benchmark.
     */
    template <typename Op>
    ANKERL_NANOBENCH(NOINLINE)
    Bench& run(char const* benchmarkName, Op&& op);

    template <typename Op>
    ANKERL_NANOBENCH(NOINLINE)
    Bench& run(std::string const& benchmarkName, Op&& op);

    /**
     * @brief Same as run(char const* benchmarkName, Op op), but instead uses the previously set name.
     * @tparam Op The code to benchmark.
     */
    template <typename Op>
    ANKERL_NANOBENCH(NOINLINE)
    Bench& run(Op&& op);

    /**
     * @brief Title of the benchmark, will be shown in the table header. Changing the title will start a new markdown table.
     *
     * @param benchmarkTitle The title of the benchmark.
     */
    Bench& title(char const* benchmarkTitle);
    Bench& title(std::string const& benchmarkTitle);
    ANKERL_NANOBENCH(NODISCARD) std::string const& title() const noexcept;

    /// Name of the benchmark, will be shown in the table row.
    Bench& name(char const* benchmarkName);
    Bench& name(std::string const& benchmarkName);
    ANKERL_NANOBENCH(NODISCARD) std::string const& name() const noexcept;

    /**
     * @brief Sets the batch size.
     *
     * E.g. number of processed byte, or some other metric for the size of the processed data in each iteration. If you benchmark
     * hashing of a 1000 byte long string and want byte/sec as a result, you can specify 1000 as the batch size.
     *
     * @tparam T Any input type is internally cast to `double`.
     * @param b batch size
     */
    template <typename T>
    Bench& batch(T b) noexcept;
    ANKERL_NANOBENCH(NODISCARD) double batch() const noexcept;

    /**
     * @brief Sets the operation unit.
     *
     * Defaults to "op". Could be e.g. "byte" for string processing. This is used for the table header, e.g. to show `ns/byte`. Use
     * singular (*byte*, not *bytes*). A change clears the currently collected results.
     *
     * @param unit The unit name.
     */
    Bench& unit(char const* unit);
    Bench& unit(std::string const& unit);
    ANKERL_NANOBENCH(NODISCARD) std::string const& unit() const noexcept;

    /**
     * @brief Sets the time unit to be used for the default output.
     *
     * Nanobench defaults to using ns (nanoseconds) as output in the markdown. For some benchmarks this is too coarse, so it is
     * possible to configure this. E.g. use `timeUnit(1ms, "ms")` to show `ms/op` instead of `ns/op`.
     *
     * @param tu Time unit to display the results in, default is 1ns.
     * @param tuName Name for the time unit, default is "ns"
     */
    Bench& timeUnit(std::chrono::duration<double> const& tu, std::string const& tuName);
    ANKERL_NANOBENCH(NODISCARD) std::string const& timeUnitName() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) std::chrono::duration<double> const& timeUnit() const noexcept;

    /**
     * @brief Set the output stream where the resulting markdown table will be printed to.
     *
     * The default is `&std::cout`. You can disable all output by setting `nullptr`.
     *
     * @param outstream Pointer to output stream, can be `nullptr`.
     */
    Bench& output(std::ostream* outstream) noexcept;
    ANKERL_NANOBENCH(NODISCARD) std::ostream* output() const noexcept;

    /**
     * Modern processors have a very accurate clock, being able to measure as low as 20 nanoseconds. This is the main trick nanobech to
     * be so fast: we find out how accurate the clock is, then run the benchmark only so often that the clock's accuracy is good enough
     * for accurate measurements.
     *
     * The default is to run one epoch for 1000 times the clock resolution. So for 20ns resolution and 11 epochs, this gives a total
     * runtime of
     *
     * @f[
     * 20ns * 1000 * 11 \approx 0.2ms
     * @f]
     *
     * To be precise, nanobench adds a 0-20% random noise to each evaluation. This is to prevent any aliasing effects, and further
     * improves accuracy.
     *
     * Total runtime will be higher though: Some initial time is needed to find out the target number of iterations for each epoch, and
     * there is some overhead involved to start & stop timers and calculate resulting statistics and writing the output.
     *
     * @param multiple Target number of times of clock resolution. Usually 1000 is a good compromise between runtime and accuracy.
     */
    Bench& clockResolutionMultiple(size_t multiple) noexcept;
    ANKERL_NANOBENCH(NODISCARD) size_t clockResolutionMultiple() const noexcept;

    /**
     * @brief Controls number of epochs, the number of measurements to perform.
     *
     * The reported result will be the median of evaluation of each epoch. The higher you choose this, the more
     * deterministic the result be and outliers will be more easily removed. Also the `err%` will be more accurate the higher this
     * number is. Note that the `err%` will not necessarily decrease when number of epochs is increased. But it will be a more accurate
     * representation of the benchmarked code's runtime stability.
     *
     * Choose the value wisely. In practice, 11 has been shown to be a reasonable choice between runtime performance and accuracy.
     * This setting goes hand in hand with minEpochIterations() (or minEpochTime()). If you are more interested in *median* runtime,
     * you might want to increase epochs(). If you are more interested in *mean* runtime, you might want to increase
     * minEpochIterations() instead.
     *
     * @param numEpochs Number of epochs.
     */
    Bench& epochs(size_t numEpochs) noexcept;
    ANKERL_NANOBENCH(NODISCARD) size_t epochs() const noexcept;

    /**
     * @brief Upper limit for the runtime of each epoch.
     *
     * As a safety precausion if the clock is not very accurate, we can set an upper limit for the maximum evaluation time per
     * epoch. Default is 100ms. At least a single evaluation of the benchmark is performed.
     *
     * @see minEpochTime(), minEpochIterations()
     *
     * @param t Maximum target runtime for a single epoch.
     */
    Bench& maxEpochTime(std::chrono::nanoseconds t) noexcept;
    ANKERL_NANOBENCH(NODISCARD) std::chrono::nanoseconds maxEpochTime() const noexcept;

    /**
     * @brief Minimum time each epoch should take.
     *
     * Default is zero, so we are fully relying on clockResolutionMultiple(). In most cases this is exactly what you want. If you see
     * that the evaluation is unreliable with a high `err%`, you can increase either minEpochTime() or minEpochIterations().
     *
     * @see maxEpochTime(), minEpochIterations()
     *
     * @param t Minimum time each epoch should take.
     */
    Bench& minEpochTime(std::chrono::nanoseconds t) noexcept;
    ANKERL_NANOBENCH(NODISCARD) std::chrono::nanoseconds minEpochTime() const noexcept;

    /**
     * @brief Sets the minimum number of iterations each epoch should take.
     *
     * Default is 1, and we rely on clockResolutionMultiple(). If the `err%` is high and you want a more smooth result, you might want
     * to increase the minimum number or iterations, or increase the minEpochTime().
     *
     * @see minEpochTime(), maxEpochTime(), minEpochIterations()
     *
     * @param numIters Minimum number of iterations per epoch.
     */
    Bench& minEpochIterations(uint64_t numIters) noexcept;
    ANKERL_NANOBENCH(NODISCARD) uint64_t minEpochIterations() const noexcept;

    /**
     * Sets exactly the number of iterations for each epoch. Ignores all other epoch limits. This forces nanobench to use exactly
     * the given number of iterations for each epoch, not more and not less. Default is 0 (disabled).
     *
     * @param numIters Exact number of iterations to use. Set to 0 to disable.
     */
    Bench& epochIterations(uint64_t numIters) noexcept;
    ANKERL_NANOBENCH(NODISCARD) uint64_t epochIterations() const noexcept;

    /**
     * @brief Sets a number of iterations that are initially performed without any measurements.
     *
     * Some benchmarks need a few evaluations to warm up caches / database / whatever access. Normally this should not be needed, since
     * we show the median result so initial outliers will be filtered away automatically. If the warmup effect is large though, you
     * might want to set it. Default is 0.
     *
     * @param numWarmupIters Number of warmup iterations.
     */
    Bench& warmup(uint64_t numWarmupIters) noexcept;
    ANKERL_NANOBENCH(NODISCARD) uint64_t warmup() const noexcept;

    /**
     * @brief Marks the next run as the baseline.
     *
     * Call `relative(true)` to mark the run as the baseline. Successive runs will be compared to this run. It is calculated by
     *
     * @f[
     * 100\% * \frac{baseline}{runtime}
     * @f]
     *
     *  * 100% means it is exactly as fast as the baseline
     *  * >100% means it is faster than the baseline. E.g. 200% means the current run is twice as fast as the baseline.
     *  * <100% means it is slower than the baseline. E.g. 50% means it is twice as slow as the baseline.
     *
     * See the tutorial section "Comparing Results" for example usage.
     *
     * @param isRelativeEnabled True to enable processing
     */
    Bench& relative(bool isRelativeEnabled) noexcept;
    ANKERL_NANOBENCH(NODISCARD) bool relative() const noexcept;

    /**
     * @brief Enables/disables performance counters.
     *
     * On Linux nanobench has a powerful feature to use performance counters. This enables counting of retired instructions, count
     * number of branches, missed branches, etc. On default this is enabled, but you can disable it if you don't need that feature.
     *
     * @param showPerformanceCounters True to enable, false to disable.
     */
    Bench& performanceCounters(bool showPerformanceCounters) noexcept;
    ANKERL_NANOBENCH(NODISCARD) bool performanceCounters() const noexcept;

    /**
     * @brief Retrieves all benchmark results collected by the bench object so far.
     *
     * Each call to run() generates a Result that is stored within the Bench instance. This is mostly for advanced users who want to
     * see all the nitty gritty details.
     *
     * @return All results collected so far.
     */
    ANKERL_NANOBENCH(NODISCARD) std::vector<Result> const& results() const noexcept;

    /*!
      @verbatim embed:rst

      Convenience shortcut to :cpp:func:`ankerl::nanobench::doNotOptimizeAway`.

      @endverbatim
     */
    template <typename Arg>
    Bench& doNotOptimizeAway(Arg&& arg);

    /*!
      @verbatim embed:rst

      Sets N for asymptotic complexity calculation, so it becomes possible to calculate `Big O
      <https://en.wikipedia.org/wiki/Big_O_notation>`_ from multiple benchmark evaluations.

      Use :cpp:func:`ankerl::nanobench::Bench::complexityBigO` when the evaluation has finished. See the tutorial
      :ref:`asymptotic-complexity` for details.

      @endverbatim

      @tparam T Any type is cast to `double`.
      @param b Length of N for the next benchmark run, so it is possible to calculate `bigO`.
     */
    template <typename T>
    Bench& complexityN(T b) noexcept;
    ANKERL_NANOBENCH(NODISCARD) double complexityN() const noexcept;

    /*!
      Calculates [Big O](https://en.wikipedia.org/wiki/Big_O_notation>) of the results with all preconfigured complexity functions.
      Currently these complexity functions are fitted into the benchmark results:

       @f$ \mathcal{O}(1) @f$,
       @f$ \mathcal{O}(n) @f$,
       @f$ \mathcal{O}(\log{}n) @f$,
       @f$ \mathcal{O}(n\log{}n) @f$,
       @f$ \mathcal{O}(n^2) @f$,
       @f$ \mathcal{O}(n^3) @f$.

      If we e.g. evaluate the complexity of `std::sort`, this is the result of `std::cout << bench.complexityBigO()`:

      ```
      |   coefficient |   err% | complexity
      |--------------:|-------:|------------
      |   5.08935e-09 |   2.6% | O(n log n)
      |   6.10608e-08 |   8.0% | O(n)
      |   1.29307e-11 |  47.2% | O(n^2)
      |   2.48677e-15 |  69.6% | O(n^3)
      |   9.88133e-06 | 132.3% | O(log n)
      |   5.98793e-05 | 162.5% | O(1)
      ```

      So in this case @f$ \mathcal{O}(n\log{}n) @f$ provides the best approximation.

      @verbatim embed:rst
      See the tutorial :ref:`asymptotic-complexity` for details.
      @endverbatim
      @return Evaluation results, which can be printed or otherwise inspected.
     */
    std::vector<BigO> complexityBigO() const;

    /**
     * @brief Calculates bigO for a custom function.
     *
     * E.g. to calculate the mean squared error for @f$ \mathcal{O}(\log{}\log{}n) @f$, which is not part of the default set of
     * complexityBigO(), you can do this:
     *
     * ```
     * auto logLogN = bench.complexityBigO("O(log log n)", [](double n) {
     *     return std::log2(std::log2(n));
     * });
     * ```
     *
     * The resulting mean squared error can be printed with `std::cout << logLogN`. E.g. it prints something like this:
     *
     * ```text
     * 2.46985e-05 * O(log log n), rms=1.48121
     * ```
     *
     * @tparam Op Type of mapping operation.
     * @param name Name for the function, e.g. "O(log log n)"
     * @param op Op's operator() maps a `double` with the desired complexity function, e.g. `log2(log2(n))`.
     * @return BigO Error calculation, which is streamable to std::cout.
     */
    template <typename Op>
    BigO complexityBigO(char const* name, Op op) const;

    template <typename Op>
    BigO complexityBigO(std::string const& name, Op op) const;

    /*!
      @verbatim embed:rst

      Convenience shortcut to :cpp:func:`ankerl::nanobench::render`.

      @endverbatim
     */
    Bench& render(char const* templateContent, std::ostream& os);
    Bench& render(std::string const& templateContent, std::ostream& os);

    Bench& config(Config const& benchmarkConfig);
    ANKERL_NANOBENCH(NODISCARD) Config const& config() const noexcept;

private:
    Config mConfig{};
    std::vector<Result> mResults{};
};
ANKERL_NANOBENCH(IGNORE_PADDED_POP)

/**
 * @brief Makes sure none of the given arguments are optimized away by the compiler.
 *
 * @tparam Arg Type of the argument that shouldn't be optimized away.
 * @param arg The input that we mark as being used, even though we don't do anything with it.
 */
template <typename Arg>
void doNotOptimizeAway(Arg&& arg);

namespace detail {

#if defined(_MSC_VER)
void doNotOptimizeAwaySink(void const*);

template <typename T>
void doNotOptimizeAway(T const& val);

#else

// These assembly magic is directly from what Google Benchmark is doing. I have previously used what facebook's folly was doing, but
// this seemed to have compilation problems in some cases. Google Benchmark seemed to be the most well tested anyways.
// see https://github.com/google/benchmark/blob/master/include/benchmark/benchmark.h#L307
template <typename T>
void doNotOptimizeAway(T const& val) {
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : : "r,m"(val) : "memory");
}

template <typename T>
void doNotOptimizeAway(T& val) {
#    if defined(__clang__)
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : "+r,m"(val) : : "memory");
#    else
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : "+m,r"(val) : : "memory");
#    endif
}
#endif

// internally used, but visible because run() is templated.
// Not movable/copy-able, so we simply use a pointer instead of unique_ptr. This saves us from
// having to include <memory>, and the template instantiation overhead of unique_ptr which is unfortunately quite significant.
ANKERL_NANOBENCH(IGNORE_EFFCPP_PUSH)
class IterationLogic {
public:
    explicit IterationLogic(Bench const& config) noexcept;
    ~IterationLogic();

    ANKERL_NANOBENCH(NODISCARD) uint64_t numIters() const noexcept;
    void add(std::chrono::nanoseconds elapsed, PerformanceCounters const& pc) noexcept;
    void moveResultTo(std::vector<Result>& results) noexcept;

private:
    struct Impl;
    Impl* mPimpl;
};
ANKERL_NANOBENCH(IGNORE_EFFCPP_POP)

ANKERL_NANOBENCH(IGNORE_PADDED_PUSH)
class PerformanceCounters {
public:
    PerformanceCounters(PerformanceCounters const&) = delete;
    PerformanceCounters& operator=(PerformanceCounters const&) = delete;

    PerformanceCounters();
    ~PerformanceCounters();

    void beginMeasure();
    void endMeasure();
    void updateResults(uint64_t numIters);

    ANKERL_NANOBENCH(NODISCARD) PerfCountSet<uint64_t> const& val() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) PerfCountSet<bool> const& has() const noexcept;

private:
#if ANKERL_NANOBENCH(PERF_COUNTERS)
    LinuxPerformanceCounters* mPc = nullptr;
#endif
    PerfCountSet<uint64_t> mVal{};
    PerfCountSet<bool> mHas{};
};
ANKERL_NANOBENCH(IGNORE_PADDED_POP)

// Gets the singleton
PerformanceCounters& performanceCounters();

} // namespace detail

class BigO {
public:
    using RangeMeasure = std::vector<std::pair<double, double>>;

    template <typename Op>
    static RangeMeasure mapRangeMeasure(RangeMeasure data, Op op) {
        for (auto& rangeMeasure : data) {
            rangeMeasure.first = op(rangeMeasure.first);
        }
        return data;
    }

    static RangeMeasure collectRangeMeasure(std::vector<Result> const& results);

    template <typename Op>
    BigO(char const* bigOName, RangeMeasure const& rangeMeasure, Op rangeToN)
        : BigO(bigOName, mapRangeMeasure(rangeMeasure, rangeToN)) {}

    template <typename Op>
    BigO(std::string const& bigOName, RangeMeasure const& rangeMeasure, Op rangeToN)
        : BigO(bigOName, mapRangeMeasure(rangeMeasure, rangeToN)) {}

    BigO(char const* bigOName, RangeMeasure const& scaledRangeMeasure);
    BigO(std::string const& bigOName, RangeMeasure const& scaledRangeMeasure);
    ANKERL_NANOBENCH(NODISCARD) std::string const& name() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double constant() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) double normalizedRootMeanSquare() const noexcept;
    ANKERL_NANOBENCH(NODISCARD) bool operator<(BigO const& other) const noexcept;

private:
    std::string mName{};
    double mConstant{};
    double mNormalizedRootMeanSquare{};
};
std::ostream& operator<<(std::ostream& os, BigO const& bigO);
std::ostream& operator<<(std::ostream& os, std::vector<ankerl::nanobench::BigO> const& bigOs);

} // namespace nanobench
} // namespace ankerl

// implementation /////////////////////////////////////////////////////////////////////////////////

namespace ankerl {
namespace nanobench {

constexpr uint64_t(Rng::min)() {
    return 0;
}

constexpr uint64_t(Rng::max)() {
    return (std::numeric_limits<uint64_t>::max)();
}

ANKERL_NANOBENCH_NO_SANITIZE("integer", "undefined")
uint64_t Rng::operator()() noexcept {
    auto x = mX;

    mX = UINT64_C(15241094284759029579) * mY;
    mY = rotl(mY - x, 27);

    return x;
}

ANKERL_NANOBENCH_NO_SANITIZE("integer", "undefined")
uint32_t Rng::bounded(uint32_t range) noexcept {
    uint64_t r32 = static_cast<uint32_t>(operator()());
    auto multiresult = r32 * range;
    return static_cast<uint32_t>(multiresult >> 32U);
}

double Rng::uniform01() noexcept {
    auto i = (UINT64_C(0x3ff) << 52U) | (operator()() >> 12U);
    // can't use union in c++ here for type puning, it's undefined behavior.
    // std::memcpy is optimized anyways.
    double d;
    std::memcpy(&d, &i, sizeof(double));
    return d - 1.0;
}

template <typename Container>
void Rng::shuffle(Container& container) noexcept {
    auto size = static_cast<uint32_t>(container.size());
    for (auto i = size; i > 1U; --i) {
        using std::swap;
        auto p = bounded(i); // number in [0, i)
        swap(container[i - 1], container[p]);
    }
}

ANKERL_NANOBENCH_NO_SANITIZE("integer", "undefined")
constexpr uint64_t Rng::rotl(uint64_t x, unsigned k) noexcept {
    return (x << k) | (x >> (64U - k));
}

template <typename Op>
ANKERL_NANOBENCH_NO_SANITIZE("integer")
Bench& Bench::run(Op&& op) {
    // It is important that this method is kept short so the compiler can do better optimizations/ inlining of op()
    detail::IterationLogic iterationLogic(*this);
    auto& pc = detail::performanceCounters();

    while (auto n = iterationLogic.numIters()) {
        pc.beginMeasure();
        Clock::time_point before = Clock::now();
        while (n-- > 0) {
            op();
        }
        Clock::time_point after = Clock::now();
        pc.endMeasure();
        pc.updateResults(iterationLogic.numIters());
        iterationLogic.add(after - before, pc);
    }
    iterationLogic.moveResultTo(mResults);
    return *this;
}

// Performs all evaluations.
template <typename Op>
Bench& Bench::run(char const* benchmarkName, Op&& op) {
    name(benchmarkName);
    return run(std::forward<Op>(op));
}

template <typename Op>
Bench& Bench::run(std::string const& benchmarkName, Op&& op) {
    name(benchmarkName);
    return run(std::forward<Op>(op));
}

template <typename Op>
BigO Bench::complexityBigO(char const* benchmarkName, Op op) const {
    return BigO(benchmarkName, BigO::collectRangeMeasure(mResults), op);
}

template <typename Op>
BigO Bench::complexityBigO(std::string const& benchmarkName, Op op) const {
    return BigO(benchmarkName, BigO::collectRangeMeasure(mResults), op);
}

// Set the batch size, e.g. number of processed bytes, or some other metric for the size of the processed data in each iteration.
// Any argument is cast to double.
template <typename T>
Bench& Bench::batch(T b) noexcept {
    mConfig.mBatch = static_cast<double>(b);
    return *this;
}

// Sets the computation complexity of the next run. Any argument is cast to double.
template <typename T>
Bench& Bench::complexityN(T n) noexcept {
    mConfig.mComplexityN = static_cast<double>(n);
    return *this;
}

// Convenience: makes sure none of the given arguments are optimized away by the compiler.
template <typename Arg>
Bench& Bench::doNotOptimizeAway(Arg&& arg) {
    detail::doNotOptimizeAway(std::forward<Arg>(arg));
    return *this;
}

// Makes sure none of the given arguments are optimized away by the compiler.
template <typename Arg>
void doNotOptimizeAway(Arg&& arg) {
    detail::doNotOptimizeAway(std::forward<Arg>(arg));
}

namespace detail {

#if defined(_MSC_VER)
template <typename T>
void doNotOptimizeAway(T const& val) {
    doNotOptimizeAwaySink(&val);
}

#endif

} // namespace detail
} // namespace nanobench
} // namespace ankerl

#endif // ANKERL_NANOBENCH_H_INCLUDED
