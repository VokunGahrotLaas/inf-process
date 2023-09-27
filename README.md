# inf-process

[![inf-process make check](https://github.com/VokunGahrotLaas/inf-process/actions/workflows/check.yml/badge.svg?branch=master)](https://github.com/VokunGahrotLaas/inf-process/actions/workflows/check.yml?query=branch%3Amaster)

C++ wrapper for C io / POSIX / Windows functions.

## Features

* Wrapping a FILE* in a inf::stdio_stream.

* Creating a inf::stdio_stream from a POSIX fd / Windows HANDLE.

* Using dup() and dup2() on inf::stdio_stream with safe-ish versions.

* Wrapping a pipe() in a inf::pipe.

* Wrapping posix_spawn/waitpid/waitid on Unix \
    & _spawnvpe/_cwait on Windows with inf::spawn.

* Wrapping fork/waitpid/waitid on Unix with inf::fork.

* Errno wrapper for all function that set errno with inf::errno_guard, \
    throws inf::errno_error with a source_location, \
    the function name that set errno & the errno value.

* Wrapping memfd_create/ftruncate on Unix & CreateFileMappingA on Windows with \
    inf::shared_memory.

* Wrapping mmap on Unix & MapViewOfFile on Windows with inf::memory_map.

* Unit tests with `<inf/tests.hpp>`.

## TODOs

* write doc with doxygen

* write validating tests (WIP)

* add features

## Credits / Sources

* GNU's libstdc++: [stdio_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12263.html)
* GNU's libstdc++: [stdio_sync_filebuf](https://gcc.gnu.org/onlinedocs/gcc-13.1.0/libstdc++/api/a12267.html)
* LLVM's libc++: [std_stream](https://github.com/llvm/llvm-project/blob/main/libcxx/src/std_stream.h)
* Nicolai M. Josuttis: [fdstream](http://www.josuttis.com/cppcode/fdstream.html)
* GNU's libstdc++ source_location: [source_location](https://github.com/gcc-mirror/gcc/blob/1f973c295b699011acd7d5fcb04bd2bf887da16d/libstdc%2B%2B-v3/include/experimental/source_location)
