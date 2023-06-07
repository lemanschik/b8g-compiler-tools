# Implementation of the opfs.zip stack
into Emscript for usage with B8G as also incremental adoption and compilation of emscripten it self
into wasm 

## Experiments
- x86_64 => contains linux-x86_64-archlinux
- wasmer => contains the wasmer code migration via porting it to wasm it self. 
- risc-v => contains the risc-v android, and linux builds. Including full ISA and up to 128bit Instructions.

## Project magnolia
Magnolia AIMS to stabilize the following flow ANY => WASM => ECMASCript

### Why?
We had perfect results with that method out of history as we did the first wasm linux iterations. 
We got code deduplication at scale and the execution and load time did optimize well as also 
We had no issues it worked out of the box.

e've been working on support for emitting standalone Wasm files from Emscripten, that do not depend on the Emscripten JS runtime! This post explains why that's interesting.

Using standalone mode in Emscripten
First, let's see what you can do with this new feature! Similar to this post let's start with a "hello world" type program that exports a single function that adds two numbers:

// add.c
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int add(int x, int y) {
  return x + y;
}
We'd normally build this with something like emcc -O3 add.c -o add.js which would emit add.js and add.wasm. Instead, let's ask emcc to only emit Wasm:

emcc -O3 add.c -o add.wasm
When emcc sees we only want Wasm then it makes it "standalone" - a Wasm file that can run by itself as much as possible, without any JavaScript runtime code from Emscripten.

Disassembling it, it's very minimal - just 87 bytes! It contains the obvious add function

(func $add (param $0 i32) (param $1 i32) (result i32)
 (i32.add
  (local.get $0)
  (local.get $1)
 )
)
and one more function, _start,

(func $_start
 (nop)
)
_start is part of the WASI spec, and Emscripten's standalone mode emits it so that we can run in WASI runtimes. (Normally _start would do global initialization, but here we just don't need any so it's empty.)

Write your own JavaScript loader
One nice thing about a standalone Wasm file like this is that you can write custom JavaScript to load and run it, which can be very minimal depending on your use case. For example, we can do this in Node.js:

// load-add.js
const binary = require('fs').readFileSync('add.wasm');

WebAssembly.instantiate(binary).then(({ instance }) => {
  console.log(instance.exports.add(40, 2));
});
Just 4 lines! Running that prints 42 as expected. Note that while this example is very simplistic, there are cases where you simply don't need much JavaScript, and may be able to do better than Emscripten's default JavaScript runtime (which supports a bunch of environments and options). A real-world example of that is in zeux's meshoptimizer - just 57 lines, including memory management, growth, etc.!

Running in Wasm runtimes
Another nice thing about standalone Wasm files is that you can run them in Wasm runtimes like wasmer, wasmtime, or WAVM. For example, consider this hello world:

// hello.cpp
#include <stdio.h>

int main() {
  printf("hello, world!\n");
  return 0;
}
We can build and run that in any of those runtimes:

$ emcc hello.cpp -O3 -o hello.wasm
$ wasmer run hello.wasm
hello, world!
$ wasmtime hello.wasm
hello, world!
$ wavm run hello.wasm
hello, world!
Emscripten uses WASI APIs as much as possible, so programs like this end up using 100% WASI and can run in WASI-supporting runtimes (see notes later on what programs require more than WASI).

Building Wasm plugins
Aside from the Web and the server, an exciting area for Wasm is plugins. For example, an image editor might have Wasm plugins that can perform filters and other operations on the image. For that type of use case you want a standalone Wasm binary, just like in the examples so far, but where it also has a proper API for the embedding application.

Plugins are sometimes related to dynamic libraries, as dynamic libraries are one way to implement them. Emscripten has support for dynamic libraries with the SIDE_MODULE option, and this has been a way to build Wasm plugins. The new standalone Wasm option described here is an improvement on that in several ways: First, a dynamic library has relocatable memory, which adds overhead if you don’t need it (and you don’t if you aren’t linking the Wasm with another Wasm after loading it). Second, standalone output is designed to run in Wasm runtimes as well, as mentioned earlier.

Okay, so far so good: Emscripten can either emit JavaScript + WebAssembly as it always did, and now it can also emit just WebAssembly by itself, which lets you run it in places that don't have JavaScript like Wasm runtimes, or you can write your own custom JavaScript loader code, etc. Now let's talk about the background and the technical details!

WebAssembly's two standard APIs
WebAssembly can only access the APIs it receives as imports - the core Wasm spec has no concrete API details. Given the current trajectory of Wasm, it looks like there will be 3 main categories of APIs that people import and use:

Web APIs: This is what Wasm programs use on the Web, which are the existing standardized APIs that JavaScript can use too. Currently these are called indirectly, through JS glue code, but in the future with interface types they will be called directly.
WASI APIs: WASI focuses on standardizing APIs for Wasm on the server.
Other APIs: Various custom embeddings will define their own application-specific APIs. For example, we gave the example earlier of an image editor with Wasm plugins that implement an API to do visual effects. Note that a plugin might also have access to “system” APIs, like a native dynamic library would, or it might be very sandboxed and have no imports at all (if the embedding just calls its methods).
