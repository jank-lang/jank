<a href="https://jank-lang.org">
  <img src="https://media.githubusercontent.com/media/jank-lang/jank/main/.github/img/banner.png" alt="jank banner" />
</a>

<div align="center">
  <a href="https://clojurians.slack.com/archives/C03SRH97FDK" target="_blank"><img src="https://img.shields.io/badge/slack-%23jank-e01563.svg?style=flat&logo=slack&logoColor=fd893f&colorA=363636&colorB=363636" /></a>
  <a href="https://github.com/sponsors/jeaye" target="_blank"><img src="https://img.shields.io/github/sponsors/jeaye?style=flat&logo=github&logoColor=fd893f&colorA=363636&colorB=363636" /></a>
  <a href="https://twitter.com/jeayewilkerson" target="_blank"><img src="https://img.shields.io/twitter/follow/jeayewilkerson?style=flat&logo=x&logoColor=fd893f&colorA=363636&colorB=363636" /></a>
  <br/>
  <a href="https://github.com/jank-lang/jank/actions" target="_blank"><img src="https://img.shields.io/github/actions/workflow/status/jank-lang/jank/build.yml?branch=main&style=flat&logo=github&logoColor=fd893f&colorA=363636&colorB=363636" alt="CI" /></a>
  <a href="https://codecov.io/gh/jank-lang/jank" target="_blank"><img src="https://img.shields.io/codecov/c/github/jank-lang/jank?style=flat&logo=codecov&logoColor=fd893f&colorA=363636&colorB=363636" /></a>
</div>

# What is jank?

Most simply, jank is a [Clojure](https://clojure.org/) dialect on LLVM with C++ interop.
Less simply, jank is a general-purpose programming language which embraces the interactive,
functional, value-oriented nature of Clojure and the desire for the native
runtime and performance of C++. jank aims to be strongly compatible with
Clojure. While Clojure's default host is the JVM and its interop is with Java,
jank's host is LLVM and its interop is with C++.

For the current progress of jank and its usability, see the tables here: https://jank-lang.org/progress/

The current tl;dr for jank's usability is: **still getting there, but not ready for
use yet**.

## Latest binaries
There are pre-compiled binaries for Ubuntu 22.04, which are built to follow the
`main` branch. You can download a tarball with everything you need here: https://github.com/jank-lang/jank/releases/tag/latest

## Appetizer
```clojure
; Comments begin with a ;
(println "meow") ; => nil

; All built-in data structures are persistent and immutable.
(def george {:name "George Clooney"}) ; => #'user/george

; Though all data is immutable by default, side effects are adhoc.
(defn say-hi [who]
  (println (str "Hi " (:name who) "!"))
  (assoc who :greeted? true))

; Doesn't change george.
(say-hi george) ; => {:name "George Clooney"
                ;     :greeted? true}

; Many core functions for working with immutable data.
(apply + (distinct [12 8 12 16 8 6])) ; => 42

; Interop with C++ can happen through inline C++.
(defn sleep [ms]
  (let [ms (int ms)]
    ; A special ~{ } syntax can be used from inline C++ to interpolate
    ; back into jank code.
    (native/raw "auto const duration(std::chrono::milliseconds(~{ ms }->data));
                 std::this_thread::sleep_for(duration);")))
```

## Docs
* [Building jank](./doc/build.md)

## Sponsors
If you'd like your name, company, or logo here, you can
[sponsor this project](https://github.com/sponsors/jeaye).

<br/>

<p align="center">
  <a href="https://www.clojuriststogether.org/">
    <img src="https://www.clojuriststogether.org/header-logo.svg" height="100px">
  </a>
</p>

<p align="center">
  <a href="https://devcarbon.com/">
    devcarbon.com
  </a>
</p>

## In the news
<table>
  <tr>
    <td>Clojure Conj 2023</td>
    <td>The REPL Interview</td>
    <td>devmio Interview</td>
    <td>Compiler Spotlight</td>
  </tr>
  <tr>
    <td>
      <a href="https://www.youtube.com/watch?v=Yw4IAY4Nx_o">
        <img src="https://i0.wp.com/2023.clojure-conj.org/wp-content/uploads/2019/06/clojure.png?resize=150%2C150&ssl=1" height="100px">
      </a>
    </td>
    <td>
      <a href="https://www.therepl.net/episodes/44/">
        <img src="https://user-images.githubusercontent.com/1057635/193151333-449385c2-9ddb-468e-b715-f149d173e310.svg" height="100px">
      </a>
    </td>
    <td>
      <a href="https://devm.io/programming/jank-programming-language">
        <img src="https://user-images.githubusercontent.com/1057635/193151345-7ad97eb4-f0f9-485a-acbb-fbe796bb7919.svg" width="300px">
      </a>
    </td>
    <td>
      <a href="https://compilerspotlight.substack.com/p/language-showcase-jank">
        <img src="https://user-images.githubusercontent.com/1057635/193154279-4b57dd8b-0985-4e35-85a2-d25b046232c5.png" width="350px">
      </a>
    </td>
  </tr>
 </table>
