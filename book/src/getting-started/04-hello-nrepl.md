# Hello, nREPL!
jank has nREPL support both via Leiningen projects and directly from the `jank`
command. This has been tested to work with both CIDER and Conjure.

## Within a Leiningen project
To start up an nREPL server for your project, you can just run the following.

```bash
$ lein repl
jank nREPL server is running on nrepl://127.0.0.1:55765
my-project.main=>
```

> [!NOTE]
> Generally, you don't need to copy the address or port yourself, since jank
> writes a `.nrepl-port` file which is found by most nREPL tooling.

## Using `jank` directly
If you're not within a Leiningen project, you can still quickly spin up an nREPL
server as part of any jank terminal nREPL client.

```bash
$ jank repl
jank nREPL server is running on nrepl://127.0.0.1:67563
user=>
```
