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

**jank is currently in alpha! Look [here](https://book.jank-lang.org/) for details.**

## Docs
Read the [jank book](https://book.jank-lang.org).

## Appetizer
```clojure
(ns my.app.filesystem
  (:include "boost/filesystem.hpp")
  (:refer-global :rename {boost.filesystem.file_size file-size}))

(defn file-info [file-path]
  (try
    (let [bytes (file-size (cpp/cast std.string file-path))]
      {:path file-path
       :size bytes})
    (catch boost.filesystem.filesystem_error e
      {:path file-path
       :error (.what e)})))

(file-info "/etc/passwd")
; {:path "/etc/passwd", :size 4025}

(file-info "/root/.bash_history")
; {:path "/root/.bash_history"
;  :error "boost::filesystem::file_size: Permission denied [system:13]: \"/root/.bash_history\""}
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
  <a href="https://nubank.com.br/">
    <img src="https://upload.wikimedia.org/wikipedia/commons/f/f7/Nubank_logo_2021.svg" height="100px">
  </a>
</p>

<!-- stijlist -->
<p align="center">
  <a href="http://www.somethingdoneright.net/about">
    Bert Muthalaly
  </a>
</p>

<!-- modulr-software -->
<p align="center">
  <a href="https://github.com/modulr-software">
    modulr-software
  </a>
</p>

<!-- multiplyco -->
<p align="center">
  <a href="https://multiply.co/">
    multiply.co
  </a>
</p>

<!-- keychera -->
<p align="center">
  <a href="https://keychera.github.io/">
    keychera
  </a>
</p>

<!-- fosskers -->
<p align="center">
  <a href="https://github.com/fosskers">
    Colin Woodbury
  </a>
</p>

<!-- pete23 -->
<p align="center">
  <a href="https://pete23.com">
    pete23
  </a>
</p>

<!-- ArikRahman -->
<p align="center">
  <a href="https://arik.zip/">
    Arik Rahman
  </a>
</p>

## In the news
<div align="center">

| [<img src="https://i0.wp.com/2023.clojure-conj.org/wp-content/uploads/2019/06/clojure.png?resize=150%2C150&ssl=1" height="100px"><br /><sub><b>Clojure Conj 2023</b></sub>](https://www.youtube.com/watch?v=Yw4IAY4Nx_o)<br />        | [<img src="https://user-images.githubusercontent.com/1057635/193151333-449385c2-9ddb-468e-b715-f149d173e310.svg" height="100px"><br /><sub><b>The REPL Interview</b></sub>](https://www.therepl.net/episodes/44/)<br /> |  [<img src="https://github.com/jank-lang/jank/assets/1057635/72ff097c-578c-46f8-a727-aae6dcf2a82f" width="100px"><br /><sub><b>Language Introduction</b></sub>](https://youtu.be/ncYlHfK25i0)<br />          | [<img src="https://github.com/jank-lang/jank/assets/1057635/9788a7c8-93da-47ea-8d1d-8a258a747942" width="100px"><br /><sub><b>Compiler Spotlight</b></sub>](https://compilerspotlight.substack.com/p/language-showcase-jank)<br /> |
| :-----------------------------------------------------------------------------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------------------------------------------------------------------------------------: | :-: | :-: |

</div>
