# Hello, world!
Now that you've installed jank, it's time to write your first jank program.
Following tradition, we'll write a trivial program which prints `Hello, world!`
to the screen.

## Project directory setup
jank doesn't require any particular directory structure. It can work with files
directly. However, in order to keep our systems clean, we'll create a new
project directory for this example. Run these commands in a terminal.

```bash
$ mkdir -p ~/projects/hello_world
$ cd ~/projects/hello_world
```

Next, use your text editor to create a file called `hello.jank` in the
`hello_world` directory. The contents should look like this.

```clojure
(println "Hello, world!")
```

Finally, we can run this file!

```bash
$ jank run hello.jank
Hello, world!
```
