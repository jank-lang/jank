#pragma once

/* BDWGC defines the `GC` symbol by default, which conflicts with other C libs like libX11.
   We get around that by forcing this #define, which instead only exposes `UseGC`.
   https://github.com/jank-lang/jank/discussions/719 */
#define GC_NAME_CONFLICT 1

#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>
