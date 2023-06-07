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
