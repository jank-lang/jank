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
use yet. Check back in a few months!**

## Docs
* [Installing jank](./compiler+runtime/doc/install.md)
* [Building jank](./compiler+runtime/doc/build.md)
* [Using jank with Leiningen](./lein-jank/README.md)
* [Using jank with Clojure CLI](./clojure-cli/README.md)

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

; Interop with C++ can happen *seamlessly*.
(defn sleep [ms]
  (let [duration (c++/std.chrono.milliseconds ms)]
    (c++/std.this_thread.sleep_for duration)))
```

## Sponsors
If you'd like your name, company, or logo here, you can
[sponsor this project](https://github.com/sponsors/jeaye) for at least $25/m.

<br/>

<p align="center">
  <a href="https://www.clojuriststogether.org/">
    <img src="https://www.clojuriststogether.org/header-logo.svg" height="100px">
  </a>
</p>

<p align="center">
  <a href="https://pitch.com/">
    Misha Karpenko
  </a>
</p>

<p align="center">
  <a href="http://www.somethingdoneright.net/about">
    Bert Muthalaly
  </a>
</p>

## In the news
<div align="center">

| [<img src="https://i0.wp.com/2023.clojure-conj.org/wp-content/uploads/2019/06/clojure.png?resize=150%2C150&ssl=1" height="100px"><br /><sub><b>Clojure Conj 2023</b></sub>](https://www.youtube.com/watch?v=Yw4IAY4Nx_o)<br />        | [<img src="https://user-images.githubusercontent.com/1057635/193151333-449385c2-9ddb-468e-b715-f149d173e310.svg" height="100px"><br /><sub><b>The REPL Interview</b></sub>](https://www.therepl.net/episodes/44/)<br /> |  [<img src="https://github.com/jank-lang/jank/assets/1057635/72ff097c-578c-46f8-a727-aae6dcf2a82f" width="100px"><br /><sub><b>Language Introduction</b></sub>](https://youtu.be/ncYlHfK25i0)<br />          | [<img src="https://github.com/jank-lang/jank/assets/1057635/9788a7c8-93da-47ea-8d1d-8a258a747942" width="100px"><br /><sub><b>Compiler Spotlight</b></sub>](https://compilerspotlight.substack.com/p/language-showcase-jank)<br /> |
| :-----------------------------------------------------------------------------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------------------------------------------------------------------------------------: | :-: | :-: |

</div>
