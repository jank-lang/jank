# Foreword
jank is a personal creation made public. It's a passion project which aspires to
push the boundaries of two languages very dear to me: C++ and Clojure. These two
languages could not be further apart, in their syntax, paradigms, culture,
adoption, and typical use cases. Still, I aim to bind them.

jank is currently alpha quality software. Most importantly, that means that you
would be crazy to ship it into production for anything that matters. More
specifically, it means that jank, and its related programs, will crash, leak
memory, provide incorrect results, and in general surprise, confound, and
frustrate until we can implement all remaining pieces and iron out all remaining
bugs. I need your help with this.

Before jank, achieving this level of seamless C++ interop, with JIT (just in
time) compiled C++, and full AOT (ahead of time) compilation support had never
been done, **from any dynamic language**. Swift has come closest, though it's
not dynamically typed and it lacks an official JIT compiler. Cppyy, for Python,
strives to compete, but it lacks AOT compilation support. Because of this trail
blazing, we have faced many bugs in Clang. Clang is the main challenge for both
compile-time performance and overall memory usage. This continues to be a
limiter for jank's success, due to the size and complexity of the Clang code
base and my limited time. To ensure jank's success, this challenge will need to
be tackled directly, most likely by finding and employing a part-time Clang
developer. If you are able to help with this, please reach out.

Moving on.

The performance of jank, during this alpha stage, will be quite bad. Depending
on the benchmark, you might find jank to be 2x or even 10x slower than Clojure
JVM. Maybe more.

**Do not be concerned by this. I am not concerned by this.**

I have not had the luxury to focus on performance much beyond some early design
decisions. Clojure JVM, aside from leaning on the JVM for much of its
performance, is doing many more optimizations than jank is currently doing. A
lot of functionality, such as the GC (BDWGC), sorted containers, and others are
currently placeholder. What's most important is that jank works and that it's
correct. Once we achieve that, I will endeaver to show that jank can be the
fastest Clojure dialect around. Right now, that is not a priority, no matter how
much you may want it to be.

Lastly, the Clojure community is empowered by backward compatibility. I respect
and appreciate this goal for both Clojure and jank. I will be codifying stable
APIs for embedding jank, ensuring the stability of jank's special forms,
developing an integrated build system to improve the longevity of jank libraries
which wrap native libraries, and pursuing binary compatibility as much as the
native world and all of its quirks allows. However, during the alpha release
stage, and until we have our first production release, anything goes.

Now, install jank. Build some software. Report all of your bugs on
[Slack](https://clojurians.slack.com/archives/C03SRH97FDK) or
[Github](https://github.com/jank-lang/jank)! Engage in the community. Work with
us as we stabilize and forge this language into what others in the future will
know it to be.

Jeaye
